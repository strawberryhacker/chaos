// NIC driver for SAMA5D2 chips (kernel driver)

#include <chaos/netbuf.h>
#include <chaos/kprint.h>
#include <chaos/cache.h>
#include <chaos/assert.h>
#include <chaos/panic.h>
#include <chaos/nic.h>
#include <stdalign.h>

#include <sama5d2/sama5d2_clk.h>
#include <sama5d2/sama5d2_gpio.h>
#include <sama5d2/regmap.h>

// The NIC RX descriptor is read by the NIC DMA both during and in between transfers.
// After a transfer the DMA will update the status word according to the last transaction
struct nic_rx_desc {
    union {
        u32 addr_word;
        struct {
            u32 owner : 1;
            u32 wrap  : 1;
            u32 addr  : 30;
        };
    };
    union {
        u32 status_word;
        struct {
            u32 len                  : 13;
            u32 fcs_status           : 1;
            u32 sof                  : 1;
            u32 eof                  : 1;
            u32 cfi                  : 1;
            u32 vlan_pri             : 3;
            u32 pri_tag_detected     : 1;
            u32 vlan_tag_detected    : 1;
            u32 type_id              : 2;
            u32 type_id_match        : 1;
            u32 addr_match_reg       : 2;
            u32 addr_match           : 1;
            u32 reserved             : 1;
            u32 unicast_hash_match   : 1;
            u32 multicast_hash_match : 1;
            u32 broadcast_detected   : 1;
        };
    };
};

// The NIC TX descriptor is read by the NIC DMA when a packet should be transmitted. The
// status bits are both updated by the user as well as the DMA
struct nic_tx_desc {
    u32 addr;
    union {
        u32 status_word;
        struct {
            u32 len            : 14;
            u32 reserved0      : 1;
            u32 last           : 1;
            u32 ignore_crc     : 1;
            u32 reserved1      : 3;
            u32 crc_errors     : 3;
            u32 reserved2      : 3;
            u32 late_collision : 1;
            u32 ahb_corrupted  : 1;
            u32 reserved3      : 1;
            u32 retry_error    : 1;
            u32 wrap           : 1;
            u32 used           : 1;
        };
    };
};

enum nic_speed {
    NIC_100Mbps,
    NIC_10Mbps,
};

enum nic_duplex {
    NIC_DUPLEX_FULL,
    NIC_DUPLEX_HALF
};

// This structure hold the link configuration, including duplex setting and transfer speed
struct nic_link_setting {
    enum nic_speed speed;
    enum nic_duplex duplex;
};

// Note that even though we don't use the queues, we still have to configure them. 
// Otherwise the DMA will not work properly
#define NIC_NUM_RX_DESC 4
#define NIC_NUM_TX_DESC 4
#define NIC_NUM_UNUSED_TX_DESC 2
#define NIC_NUM_UNUSED_RX_DESC 2
#define NIC_QUEUES 4

// Setup static descriptors aligned on a cache line
static alignas(8) struct nic_rx_desc rx_descs[NIC_NUM_RX_DESC];
static alignas(8) struct nic_tx_desc tx_descs[NIC_NUM_TX_DESC];
static alignas(8) struct nic_rx_desc rx_descs_q1[NIC_NUM_UNUSED_RX_DESC];
static alignas(8) struct nic_tx_desc tx_descs_q1[NIC_NUM_UNUSED_TX_DESC];
static alignas(8) struct nic_rx_desc rx_descs_q2[NIC_NUM_UNUSED_RX_DESC];
static alignas(8) struct nic_tx_desc tx_descs_q2[NIC_NUM_UNUSED_TX_DESC];
static alignas(8) struct nic_rx_desc rx_descs_q3[NIC_NUM_UNUSED_RX_DESC];
static alignas(8) struct nic_tx_desc tx_descs_q3[NIC_NUM_UNUSED_TX_DESC];

// These keep track of the current active TX / RX descriptor
static u32 rx_index = 0;
static u32 tx_index = 0;

static struct netbuf* rx_desc_map[NIC_NUM_RX_DESC];
static struct netbuf* tx_desc_map[NIC_NUM_TX_DESC];

// Holds the address of our connected ethernet phy
static u8 phy_addr;

// Contains a mapping between the RX descriptor, TX descriptor and both sizes for a given
// queue. This is to avoid a mess when configuring the hardware
struct nic_queue {
    struct nic_rx_desc* rx;
    struct nic_tx_desc* tx;
    u32 tx_count;
    u32 rx_count;
};

// The SAMA5D2 implements a GMAC with one base queue and two additional queues
static const struct nic_queue queues[NIC_QUEUES] = {
    {
        // Base queue
        .rx = rx_descs,
        .tx = tx_descs,
        .rx_count = NIC_NUM_RX_DESC,
        .tx_count = NIC_NUM_TX_DESC
    },
    {
        // Queue 1
        .rx = rx_descs_q1,
        .tx = tx_descs_q1,
        .rx_count = NIC_NUM_UNUSED_RX_DESC,
        .tx_count = NIC_NUM_UNUSED_TX_DESC,
    },
    {
        // Queue 2
        .rx = rx_descs_q2,
        .tx = tx_descs_q2,
        .rx_count = NIC_NUM_UNUSED_RX_DESC,
        .tx_count = NIC_NUM_UNUSED_TX_DESC,
    },
    {
        // Queue 3
        .rx = rx_descs_q3,
        .tx = tx_descs_q3,
        .rx_count = NIC_NUM_UNUSED_RX_DESC,
        .tx_count = NIC_NUM_UNUSED_TX_DESC,
    }
};

// Configures all the NIC queues (rings). This will allocate a netbuf for each DMA 
// descriptor and link the DMA descriptor to the netbuf->buf. This also configures the 
// hardware registers for each queue
void nic_setup_dma_queues() {
    for (u32 i = 0; i < NIC_QUEUES; i++) {
        
        struct netbuf* netbuf;
        const struct nic_queue* queue = &queues[i];

        // Configure the TX queue
        for (u32 j = 0; j < queue->tx_count; j++) {
            netbuf = alloc_netbuf();
            if (i == 0) {
                tx_desc_map[j] = netbuf;
            }
            struct nic_tx_desc* tx = &queue->tx[j];

            // Link the descriptor to the netbuf
            // TODO: when the MMU is on we must convert to physical address
            tx->addr = (u32)netbuf;
            tx->status_word = 0;

            // This will make sure the DMA can't use the buffer
            tx->used = 1;
        }

        // Configure the RX queue
        for (u32 j = 0; j < queue->rx_count; j++) {
            netbuf = alloc_netbuf();
            if (i == 0) {
                rx_desc_map[j] = netbuf;
            }
            struct nic_rx_desc* rx = &queue->rx[j];

            // Link the descriptor to the netbuf
            // TODO: when the MMU is on we must convert to physical address
            assert(((u32)netbuf & 0b11) == 0);

            rx->addr_word = 0;
            rx->status_word = 0;

            // The address is in bits 32..2
            rx->addr = (u32)netbuf >> 2;
        }

        // Mark the end descriptor with the wrap bit, causing the DMA to fetch the base
        // descriptor on the next read
        queue->rx[queue->rx_count - 1].wrap = 1;
        queue->tx[queue->tx_count - 1].wrap = 1;
    }

    // Map in the queues in the NIC hardware
    struct nic_reg* const nic_reg = NIC_REG;

    nic_reg->rbqb       = (u32)rx_descs;
    nic_reg->tbqb       = (u32)tx_descs;
    nic_reg->rbqbapq[0] = (u32)rx_descs_q1;
    nic_reg->tbqbapq[0] = (u32)tx_descs_q1;
    nic_reg->rbqbapq[1] = (u32)rx_descs_q2;
    nic_reg->tbqbapq[1] = (u32)tx_descs_q2;
    nic_reg->rbqbapq[2] = (u32)rx_descs_q3;
    nic_reg->tbqbapq[2] = (u32)tx_descs_q3;

    // Make sure we start reading from the base descriptor
    rx_index = 0;
    tx_index = 0;
}

// Reads a 16-bit register from the addressed ethernet PHY. Both the register address and 
// the etnernet PHY address should be in range 0..31
u16 ethernet_phy_read(u8 phy, u8 reg) {
    assert(phy < 32);
    assert(reg < 32);
    
    struct nic_reg* const nic_reg = NIC_REG;

    nic_reg->man = (1 << 30) | (1 << 29) | (1 << 17) | (phy << 23) | (reg << 18);
    while ((nic_reg->nsr & (1 << 2)) == 0);

    return (u16)nic_reg->man;
}

// Writes a 16-bit register to the address ethernet PHY. Both the register address and 
// the etnernet PHY address should be in range 0..31
void ethernet_phy_write(u8 phy, u8 reg, u16 val) {
    assert(phy < 32);
    assert(reg < 32);

    struct nic_reg* const nic_reg = NIC_REG;

    nic_reg->man = (1 << 30) | (1 << 28) | (1 << 17) | (phy << 23) | (reg << 18) | val;
    while ((nic_reg->nsr & (1 << 2)) == 0);
}

// Performs a scan after available ethernet PHYs and return the address of the first 
// responding PHY
u8 ethernet_phy_scan() {
    for (u32 i = 0; i < 32; i++) {

        // Read the PHY ID reg
        if (ethernet_phy_read(i, 2) != 0xFFFF) {
            return i;
        }
    }
    panic("No phy\n");
    return 0;
}

// Configures the ethernet PHY for full-duplex, 100 Mbps opration and returns when the 
// auto-negotiation is complete and the link is up 
void phy_establish_link(u8 addr) {
    // Check if the link is already up
    if ((ethernet_phy_read(addr, 1) & (1 << 5)) == 0) {

        // Write our capabilites
        ethernet_phy_write(addr, 4, ethernet_phy_read(addr, 4) | (0b1111 << 5));

        // Restart the auto-negotiation
        ethernet_phy_write(addr, 0, ethernet_phy_read(addr, 0) | (1 << 9));
        while ((ethernet_phy_read(addr, 1) & (1 << 5)) == 0);
    }

    // Wait for the link-up status
    while ((ethernet_phy_read(addr, 1) & (1 << 2)) == 0);
}

// Get the speed and duplex setting from the link partner
void get_phy_settings(u8 addr, struct nic_link_setting* link_setting) {
    // Read the link partner status register
    u16 reg = ethernet_phy_read(addr, 5);

    if (reg & (0b11 << 7)) {
        link_setting->speed = NIC_100Mbps;
        link_setting->duplex = (reg & (1 << 8)) ? NIC_DUPLEX_FULL : NIC_DUPLEX_HALF;
    } else {
        link_setting->speed = NIC_10Mbps;
        link_setting->duplex = (reg & (1 << 6)) ? NIC_DUPLEX_FULL : NIC_DUPLEX_HALF;
    }
}

// Configure the NIC pins
void nic_pin_init() {
    sama5d2_gpio_set_func(GPIOD_REG,  9, GPIO_FUNC_D);
    sama5d2_gpio_set_func(GPIOD_REG, 10, GPIO_FUNC_D);
    sama5d2_gpio_set_func(GPIOD_REG, 11, GPIO_FUNC_D);
    sama5d2_gpio_set_func(GPIOD_REG, 12, GPIO_FUNC_D);
    sama5d2_gpio_set_func(GPIOD_REG, 13, GPIO_FUNC_D);
    sama5d2_gpio_set_func(GPIOD_REG, 14, GPIO_FUNC_D);
    sama5d2_gpio_set_func(GPIOD_REG, 15, GPIO_FUNC_D);
    sama5d2_gpio_set_func(GPIOD_REG, 16, GPIO_FUNC_D);
    sama5d2_gpio_set_func(GPIOD_REG, 17, GPIO_FUNC_D);
    sama5d2_gpio_set_func(GPIOD_REG, 18, GPIO_FUNC_D);
}

// Tries to receive a IEEE 802.3 ethernet packet from the NIC hardware. This will either
// return a netbuf with the packet data, or NULL. The netbuf will be completely unlinked 
// adfter this call, so the user must free the netbuf manually when done reading 
struct netbuf* nic_receive() {
    struct nic_rx_desc* rx_desc = &rx_descs[rx_index];

    // Read and clear the status flags
    u32 reg = NIC_REG->rsr;
    NIC_REG->rsr = reg;

    if (rx_desc->owner) {
        // Convert the DMA descriptor address pointer 32..2 into a netbuf
        struct netbuf* netbuf = rx_desc_map[rx_index];

        // We don't support packet linking
        assert(rx_desc->sof && rx_desc->eof);

        // Save the length and reset the netbuf pointers
        netbuf->len = rx_desc->len;
        netbuf->ptr = netbuf->buf;

        // Since the current netbuf should be returned, we must allocate a new one and 
        // replace the old one
        struct netbuf* new = alloc_netbuf();
        rx_desc_map[rx_index] = new;
        rx_desc->addr = (u32)new->buf >> 2;
        rx_desc->owner = 0;

        if (++rx_index >= NIC_NUM_RX_DESC) {
            rx_index = 0;
        }

        // TODO: invalidate the cache before returning !!!!!!!!!!
        return netbuf;
    }

    // Any error handling?
    return NULL;
}

// Sends a IEEE 802.3 network packet from the NIC. This should be called with an allocated
// netbuf. This function will take care of freeing the netbuffers after they have been
// transmitted
void nic_send(struct netbuf* buf) {
    struct nic_tx_desc* tx_desc = &tx_descs[tx_index];
    struct nic_reg* const nic_reg = NIC_REG;

    // Check the transmit status
    if (nic_reg->tsr & ((1 << 4) | (1 << 8) | 0x110)) {
        panic("Warning: NIC TX error");
    }

    nic_reg->tsr = nic_reg->tsr;

    // This buffer should be owned by us, if not, we have saturated the network card. In 
    // this case we wait for the packet to be transmitted
    if (tx_desc->used == 0) {
        panic("Warning: network card saturated\n");
        while (tx_desc->used == 0);
    }

    // The NIC transmit ring should always contain a linked netbuf on each node
    struct netbuf* netbuf = tx_desc_map[tx_index];
    free_netbuf(netbuf);

    // Map in the new descriptor
    tx_desc_map[tx_index] = buf;

    tx_desc->addr = (u32)buf->ptr;
    tx_desc->len = buf->len;
    tx_desc->ignore_crc = 0;
    tx_desc->last = 1;

    // Clean any cached regions here !!!!!!
    tx_desc->used = 0;

    // If the NIC is idle we start a new transfer
    nic_reg->ncr |= (1 << 9);

    // Get the next entry in the ring
    if (++tx_index >= NIC_NUM_TX_DESC) {
        tx_index = 0;
    }
}

// Configures the NIC hardware and enables the NIC interface. This will setup the NIC in
// a non-interrupt driven mode. Polling is the only way of sending / receiving packets
void nic_init() {
    boot_message("Starting kernel NIC driver for SAMA5D2\n");

    // Enable clock and pins
    sama5d2_per_clk_en(5);
    nic_pin_init();

    // Reset the interface
    struct nic_reg* const nic_reg = NIC_REG;
    nic_reg->ncr = 0;
    nic_reg->ncfgr = 0;

    // Disable interrupts
    nic_reg->idr = ~0;
    nic_reg->idrpq[0] = ~0;
    nic_reg->idrpq[1] = ~0;

    // Clear interrupts
    (void)nic_reg->isr;
    (void)nic_reg->isrpq[0];
    (void)nic_reg->isrpq[1];

    //Setup the DMA queues
    nic_setup_dma_queues();
    
    // Enable the PHY management interface and set the bus speed
    nic_reg->ncfgr = (nic_reg->ncfgr & ~(0x7 << 18)) | (5 << 18);
    nic_reg->ncr |= (1 << 4);
    
    // Wait for the link-up status
    phy_addr = ethernet_phy_scan();
    phy_establish_link(phy_addr);

    // Get the highest link setting
    struct nic_link_setting link_setting;
    get_phy_settings(phy_addr, &link_setting);

    // Copy all frames, 100Mbps and full-duplex configuration
    nic_reg->ncfgr = (1 << 0) | (1 << 1) | (1 << 4);

    // Configure for RMII mode
    nic_reg->ur = (1 << 0);

    // Enable TCP/UDP/IP CRC engine
    nic_reg->ncfgr |= (1 << 24);

    // DMA configuration
    nic_reg->dcfgr = (4 << 0) | (3 << 8) | (1 << 10) | (1 << 11) | (0x18 << 16);

    // Ignore interrupts
    nic_reg->idr = 0xFFFFFFFF;

    // Enable receiver and transmitter
    nic_reg->ncr |= (1 << 2) | (1 << 3);
}
