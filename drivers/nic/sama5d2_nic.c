// NIC kernel driver for the SAMA5D2 chip

#include <chaos/nic.h>
#include <chaos/netbuf.h>
#include <chaos/kprint.h>
#include <chaos/cache.h>
#include <chaos/assert.h>
#include <chaos/panic.h>
#include <stdalign.h>

#include <sama5d2/sama5d2_clk.h>
#include <sama5d2/sama5d2_gpio.h>
#include <sama5d2/regmap.h>

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
            union {
                u32 type_id          : 2;
                u32 crc_status       : 2;
            };
            union {
                u32 type_id_match    : 1;
                u32 snap_status      : 1;
            };
            u32 addr_match_reg       : 2;
            u32 addr_match           : 1;
            u32 reserved             : 1;
            u32 unicast_hash_match   : 1;
            u32 multicast_hash_match : 1;
            u32 broadcast_detected   : 1;
        };
    };
};

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

#define NIC_NUM_RX_DESC 32
#define NIC_NUM_TX_DESC 32

#define NIC_NUM_UNUSED_TX_DESC 2
#define NIC_NUM_UNUSED_RX_DESC 2

#define NIC_QUEUES 3

// Setup static descriptors
static alignas(64) struct nic_rx_desc rx_desc[NIC_NUM_RX_DESC];
static alignas(64) struct nic_tx_desc tx_desc[NIC_NUM_TX_DESC];

// Setup queue 1 descriptors
static alignas(64) struct nic_rx_desc rx_desc_q1[NIC_NUM_RX_DESC];
static alignas(64) struct nic_tx_desc tx_desc_q1[NIC_NUM_TX_DESC];

// Setup queue 2 descriptors
static alignas(64) struct nic_rx_desc rx_desc_q2[NIC_NUM_RX_DESC];
static alignas(64) struct nic_tx_desc tx_desc_q2[NIC_NUM_TX_DESC];

struct nic_queue {
    struct nic_rx_desc* rx;
    struct nic_tx_desc* tx;
    u32 tx_count;
    u32 rx_count;
};

// Defines the queue descriptor lists
static const struct nic_queue queues[NIC_QUEUES] = {
    {
        .rx = rx_desc,
        .tx = tx_desc,
        .rx_count = NIC_NUM_RX_DESC,
        .tx_count = NIC_NUM_TX_DESC
    },
    {
        .rx = rx_desc_q1,
        .tx = tx_desc_q1,
        .rx_count = NIC_NUM_UNUSED_RX_DESC,
        .tx_count = NIC_NUM_UNUSED_TX_DESC,
    },
    {
        .rx = rx_desc_q2,
        .tx = tx_desc_q2,
        .rx_count = NIC_NUM_UNUSED_RX_DESC,
        .tx_count = NIC_NUM_UNUSED_TX_DESC,
    }
};

void nic_setup_dma_queues(void) {

    // Map in netbuffers and initialize the queues
    for (u32 i = 0; i < NIC_QUEUES; i++) {
        struct netbuf* buf;
        const struct nic_queue* queue = &queues[i];

        // Setup the TX queue
        for (u32 j = 0; j < queue->tx_count; j++) {
            buf = alloc_netbuf();
            struct nic_tx_desc* tx = &queue->tx[j];

            // Link the descriptor to the netbuffer
            // TODO: should use physical address and not virtual
            tx->status_word = 0;
            tx->used = 1;
            tx->addr = (u32)buf;
        }

        // Setup the RX queue
        for (u32 j = 0; j < queue->rx_count; j++) {
            buf = alloc_netbuf();
            struct nic_rx_desc* rx = &queue->rx[j];

            // Link the descriptor to the netbuffer
            // TODO: should use physical address and not virtual
            assert(((u32)buf & 0b11) == 0);

            rx->addr_word = 0;
            rx->status_word = 0;
            rx->addr = (u32)buf >> 2;
        }

        // Set the wrap bit on all queues
        queue->rx[queue->rx_count - 1].wrap = 1;
        queue->tx[queue->tx_count - 1].wrap = 1;
    }

    // Map in the queues in the NIC hardware
    struct gmac_reg* hw = GMAC_REG;

    hw->rbqb = (u32)&rx_desc[0];
    hw->tbqb = (u32)tx_desc;
    hw->rbqbapq[0] = (u32)rx_desc_q1;
    hw->tbqbapq[0] = (u32)tx_desc_q1;
    hw->rbqbapq[1] = (u32)rx_desc_q2;
    hw->tbqbapq[1] = (u32)tx_desc_q2;


}

u16 phy_read(u8 phy, u8 reg) {
    struct gmac_reg* const hw = GMAC_REG;

    assert(phy < 32);
    assert(reg < 32);

    hw->man = (1 << 30) | (1 << 29) | (1 << 17) | (phy << 23) | (reg << 18);
    while ((hw->nsr & (1 << 2)) == 0);

    // Return the result
    return (u16)hw->man;
}

void phy_write(u8 phy, u8 reg, u16 val) {
    struct gmac_reg* const hw = GMAC_REG;

    assert(phy < 32);
    assert(reg < 32);

    hw->man = (1 << 30) | (1 << 28) | (1 << 17) | (phy << 23) | (reg << 18) | val;
    while ((hw->nsr & (1 << 2)) == 0);
}

void phy_scan(u8* phy_addr) {
    for (u32 i = 0; i < 32; i++) {
        u16 val = phy_read(i, 2);
        if (val != 0xFFFF) {
            *phy_addr = i;
            return;
        }
    }
    panic("No phy\n");
}

void phy_establish_link(u8 addr) {
    // Check if the link is already up
    if ((phy_read(addr, 1) & (1 << 5)) == 0) {

        // Make sure we say we are 100Mbps capable
        phy_write(addr, 4, phy_read(addr, 4) | (0b1111 << 5));

        // Restart the auto-neg
        phy_write(addr, 0, phy_read(addr, 0) | (1 << 9));

        u32 count = 0;
        while ((phy_read(addr, 1) & (1 << 5)) == 0) {
            count++;
        }

        kprint("Auto-neg completed after {d} of retries\n", count);
    }

    // Wait for the link
    while ((phy_read(addr, 1) & (1 << 2)) == 0);
}

// Get the speed and duplex setting from the link partner
void get_phy_settings(u8 addr, enum nic_speed* speed, enum nic_duplex* dup) {
    u16 reg = phy_read(addr, 5);

    if (reg & (0b11 << 7)) {
        *speed = NIC_100Mbps;
        *dup = (reg & (1 << 8)) ? NIC_DUPLEX_FULL : NIC_DUPLEX_HALF;
    } else {
        *speed = NIC_10Mbps;
        *dup = (reg & (1 << 6)) ? NIC_DUPLEX_FULL : NIC_DUPLEX_HALF;
    }
}

// Configure the NIC pins
void nic_pin_init(void) {
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

static u8 phy_addr;
static u32 rx_index;
static u32 tx_index;

struct netbuf* nic_recv(void) {
    struct gmac_reg* const hw = GMAC_REG;

    // Clear any bits in the status register
    u32 reg = hw->rsr;
    kprint("NIC status => {r}\n", reg);
    hw->rsr = reg;

    // Check the descriptor number rx_index
    if (reg & (1 << 1)) {
        // We have a new packet

        struct nic_rx_desc* desc = &rx_desc[rx_index];

        // Convert the NIC descriptor into a netbuffer
        struct netbuf* buf = (struct netbuf *)(desc->addr << 2);

        // Increment the index pointer
        if (++rx_index >= NIC_NUM_RX_DESC) {
            rx_index = 0;
        }

        // We don't support packet linking
        assert(desc->sof && desc->eof);

        // Save the length
        buf->len = desc->len;
        buf->ptr = buf->buf;

        // Allocate a new netbuffer
        struct netbuf* new = alloc_netbuf();
        
        // Link in a new netbuffer
        desc->addr_word = 0;
        desc->status_word = 0;
        desc->addr = (u32)new->buf >> 2;

        // Invalidate the cache before returning !!!!!!!!!!
        return buf;
    }

    if (reg & (1 << 0)) {
        for (u32 i = 0; i < 32; i++) {
            kprint("Status {2:d} is {r}\n", i, rx_desc[i].addr_word);
        }
        while (1);
    }


    // Any error handling?
    return NULL;
}

void nic_send(struct netbuf* buf) {

}

// Configures the NIC hardware and enables the NIC interface
void nic_init(void) {

    kprint("Size of netbuffer => {d}\n", sizeof(struct netbuf));

    kprint("Setting up SAMA5D2 NIC\n");

    // Enable the NIC clock
    sama5d2_per_clk_en(5);

    // Enable the NIC pins
    nic_pin_init();

    //Setup the DMA queues
    nic_setup_dma_queues();

    // Get a pointer to the NIC hardware
    struct gmac_reg* const hw = GMAC_REG;
    
    // Setup the ethernet PHY
    hw->ncr |= (1 << 4);
    hw->ncfgr = (hw->ncfgr & ~(0x7 << 18)) | (5 << 18);
    phy_scan(&phy_addr);
    kprint("Found ethenet PHY on address {d}\n", phy_addr);

    // Wait for the link
    phy_establish_link(phy_addr);
    kprint("Link is up\n");

    enum nic_speed speed;
    enum nic_duplex dup;

    get_phy_settings(phy_addr, &speed, &dup);

    kprint("Print speed setting => {d}\n", speed);
    kprint("Print duplex setting => {d}\n", dup);

    rx_index = 0;
    tx_index = 0;

    // Setup the NIC - copy all frames, 100Mbps and full-duplex
    hw->ncfgr = (1 << 0) | (1 << 1) | (1 << 4);

    // Enable RMII
    hw->ur = (1 << 0);

    // Setup the DMA configuration
    hw->dcfgr = (4 << 0) | (3 << 8) | (1 << 10) | (0x18 << 16);

    // Disable all the interrupt
    hw->idr = 0xFFFFFFFF;

    hw->ncr |= (1 << 2) | (1 << 3);

    kprint("Done setup NIC\n");

    hw->ncr |= (1 << 9);
}

 