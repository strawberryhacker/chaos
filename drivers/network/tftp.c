// TFTP network driver for the kernel soft reboot

#include <chaos/tftp.h>
#include <chaos/ipv4.h>
#include <chaos/mac.h>
#include <chaos/arp.h>
#include <chaos/mem.h>
#include <chaos/nic.h>
#include <chaos/kprint.h>
#include <chaos/panic.h>



struct __attribute__((packed)) udp_header {
    u16 source_port;
    u16 dest_port;
    u16 len;
    u16 crc;
};

struct __attribute__((packed)) ipv4_header {
    u8  version_ihl;
    u8  dscp_ecn;
    u16 len;
    u16 id;
    u16 frag_off;
    u8  ttl;
    u8  protocol;
    u16 crc;
    u32 source_ip;
    u32 dest_ip;
};

struct __attribute__((packed)) mac_header {
    u8 source_mac[6];
    u8 dest_mac[6];
    u16 type;
};

// Port number of our TFTP client
const u32 tftp_client_port = 313;
u32 tftp_server_port;

// Packet size
u32 packet_size = 512;

// Destination for the new kernel
u8* tftp_tmp_dest;

// MAC and IPv4 address of TFTP server
u8 tftp_server_mac[6];
u32 tftp_server_ip;
u32 tftp_server_port;

// MAC and  IPv4 of out client
u32 tftp_client_ip;

// Checks if the TFTP is done
u32 tftp_done;

static void ipv4_send(struct netbuf* buf) {
    // Append the IPv4 header
    buf->ptr -= sizeof(struct ipv4_header);
    buf->len += sizeof(struct ipv4_header);

    // Get a pointer to the start of the header
    struct ipv4_header* ipv4_header = (struct ipv4_header *)buf->ptr;

    // Fill in the fields
    ipv4_header->version_ihl = (4 << 4) | 5;
    ipv4_header->dscp_ecn = 0;
    store_be16(buf->len, &ipv4_header->len);
    store_be16(0, &ipv4_header->id);
    store_be16(0, &ipv4_header->frag_off);
    store_be16(0, &ipv4_header->crc);

    ipv4_header->protocol = 0x11; // UDP
    ipv4_header->ttl = 0xFF;

    store_be32(tftp_client_ip, &ipv4_header->source_ip);
    store_be32(tftp_server_ip, &ipv4_header->dest_ip);

    mac_send_to(buf, tftp_server_mac, MAC_TYPE_IPv4);
}

static void udp_send(struct netbuf* buf, u32 source_port, u32 dest_port) {
    // Append the UDP header
    buf->ptr -= sizeof(struct udp_header);
    buf->len += sizeof(struct udp_header);

    // Get a pointer to the start of the header
    struct udp_header* udp_header = (struct udp_header *)buf->ptr;

    // Fill in the fields
    store_be16(source_port, &udp_header->source_port);
    store_be16(dest_port, &udp_header->dest_port);
    store_be16(buf->len, &udp_header->len);
    store_be16(0, &udp_header->crc);

    ipv4_send(buf);
}

void handle_arp(struct netbuf* buf) {
    struct arp_header* arp_header = (struct arp_header *)buf->ptr;

    if (read_be16(&arp_header->operation) == 0x0001 && 
        read_be32(&arp_header->dest_ip) == tftp_client_ip) {
        // Send an ARP response
        struct netbuf* resp = alloc_netbuf();
        struct arp_header* arp_header_resp = (struct arp_header *)resp->ptr;

        // Fill in the ARP stuff
        arp_header_resp->mac_len = 6;
        arp_header_resp->ip_len = 4;
        store_be16(0x0001, &arp_header_resp->hardware_type);
        store_be16(0x0800, &arp_header_resp->protocol_type);
        store_be16(0x0002, &arp_header_resp->operation);

        // Fill in our MAC address
        get_mac_addr(arp_header_resp->source_mac);
        store_be32(tftp_client_ip, &arp_header_resp->source_ip);
        store_be32(read_be32(&arp_header->source_ip), &arp_header_resp->dest_ip);

        for (u32 i = 0; i < 6; i++) {
            arp_header_resp->dest_mac[i] = arp_header->source_mac[i];
        }

        resp->len = sizeof(struct arp_header);    
        mac_send_to(resp, arp_header->source_mac, MAC_TYPE_ARP);
    }
}

void tftp_ack(u32 block_num) {
    struct netbuf* buf = alloc_netbuf();
    
    store_be16(4, buf->ptr);
    store_be16(block_num, buf->ptr + 2);
    buf->len = 4;

    udp_send(buf, tftp_client_port, tftp_server_port);
}

// Current sequence number
static u32 curr_sequence_num = 0;

void handle_udp(struct netbuf* buf, u32 size) {
    struct udp_header* header = (struct udp_header *)buf->ptr;

    if (read_be16(&header->dest_port) == tftp_client_port) {

        // Check and update the source port
        if (tftp_server_port == 0) {
            tftp_server_port = read_be16(&header->source_port);
        }

        // Get the lenght
        u16 len = read_be16(buf->ptr + 4) - 4 - sizeof(struct udp_header);

        // Advance the pointer
        buf->ptr += sizeof(struct udp_header);

        // Check if the TFTP is a data packet

        if (read_be16(buf->ptr) == 3) {

            // Get the sequence number
            u16 sequence_num = read_be16(buf->ptr + 2);
            if (sequence_num == (curr_sequence_num + 1)) {

                // Write the buffer to memory
                u8* src = buf->ptr + 4;
                for (u32 i = 0; i < len; i++) {
                    *tftp_tmp_dest++ = *src++;
                }

                tftp_ack(sequence_num);
                curr_sequence_num++;
            
                if (len != packet_size) {
                    tftp_done = 1;
                }
            } else {
                
            }
        }
    }
}

void handle_tftp(struct netbuf* buf) {
    struct ipv4_header* header = (struct ipv4_header *)buf->ptr;

    u32 size = 0;

    // Check the IP, len and protocol
    if (header->protocol == 0x11 && read_be32(&header->dest_ip) == tftp_client_ip) {
        // UDP packet for us
        size = read_be16(&header->len);
        size -= (4 * (header->version_ihl & 0xF));

        buf->ptr += sizeof(struct ipv4_header);

        handle_udp(buf, size);
    }
}

void tftp_request(const char* file_name, const char* block_size) {
    struct netbuf* buf = alloc_netbuf();
    
    u8* ptr = buf->ptr;

    store_be16(0x0001, ptr);
    ptr += 2;

    // Write the filename
    do {
        *ptr++ = *file_name;
    } while (*file_name++);

    // Write the mode
    const char* mode_field = "octet";
    do {
        *ptr++ = *mode_field;
    } while (*mode_field++);

    if (block_size) {
        // Insert the block size option
        const char* block_size_field = "blksize";
        do {
            *ptr++ = *block_size_field;
        } while (*block_size_field++);

        // Write the block size
        do {
            *ptr++ = *block_size;
        } while (*block_size++);
    }

    // Update the length
    buf->len = ptr - buf->ptr;

    // Send the new packet
    udp_send(buf, tftp_client_port, 69);
}

i32 tftp_read_file(void* dest, const char* file_name, const char* dest_ip,
    const char* source_ip) {

    // Clear the server TFTP port
    tftp_server_port = 0;
    curr_sequence_num = 0;

    // Get the network IP address
    string_to_ipv4(dest_ip, &tftp_server_ip);
    string_to_ipv4(source_ip, &tftp_client_ip);

    kprint("Doing TFTP from {s} to {s}\n", source_ip, dest_ip);

    // Get the MAC address of the host computer
    arp_get_mac_addr(tftp_server_ip, tftp_server_mac);

    // Send a GARP
    send_garp(tftp_client_ip);

    // We have the MAC address of the HOST
    kprint("MAC address of host is ");
    for (u32 i = 0; i < 6; i++) {
        kprint("{0:2:h}:", tftp_server_mac[i]);
    }
    kprint("\n");
    boot_message("Loading the kernel to {p}\n", dest);

    tftp_tmp_dest = dest;

    tftp_request(file_name, NULL);

    tftp_done = 0;
    while (tftp_done == 0) {
        struct netbuf* buf = nic_receive();
        if (buf == NULL) {
            continue;
        }

        // We have a new packet {data, OACK, ARP req}
        struct mac_header* mac_header = (struct mac_header *)buf->ptr;

        if (read_be16(&mac_header->type) == MAC_TYPE_ARP) {
            buf->ptr += sizeof(struct mac_header);
            handle_arp(buf);
        } else if (read_be16(&mac_header->type) == MAC_TYPE_IPv4) {
            buf->ptr += sizeof(struct mac_header);
            handle_tftp(buf);

        }
        free_netbuf(buf);
    }

    boot_message("Starting new kernel at {p}\n", dest);

    // We have a new image in memory - execute it
    void (*new_kernel)() = (void *)((u32)dest);
    new_kernel();

    return 0;
}


// Start the networking
void network_start() {
    boot_message("Starting networking\n");
    
    netbuf_init();
    nic_init();

    boot_message("Networking ready\n");

    for (u32 i = 0; i < 500000; i++) {
        asm ("nop");
    }    
}
