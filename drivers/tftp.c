// TFTP network driver for kernel soft reboot

#include <chaos/tftp.h>
#include <chaos/mem.h>
#include <chaos/nic.h>
#include <chaos/kprint.h>
#include <chaos/panic.h>
#include <chaos/status.h>

// Settings for the TFTP interface. These settings can be overridden in the config file
#ifndef TFTP_CLIENT_IP
#define TFTP_CLIENT_IP "1.1.1.1"
#endif

#ifndef TFTP_SERVER_IP
#define TFTP_SERVER_IP "1.1.1.1"
#endif

#ifndef TFTP_CLIENT_MAC
#define TFTP_CLIENT_MAC "00:00:00:00:00:00"
#endif

#ifndef TFTP_DATA_SIZE
#define TFTP_DATA_SIZE "512"
#endif

#ifndef TFTP_FILE_NAME
#define TFTP_FILE_NAME "none.bin"
#endif

struct __attribute__((packed)) mac_header {
    u8  dest_mac[6];
    u8  source_mac[6];
    u16 type;
};

struct __attribute__((packed)) arp_header {
    u16 hardware_type;
    u16 protocol_type;
    u8  mac_len;
    u8  ip_len;
    u16 operation;
    u8  source_mac[6];
    u32 source_ip;
    u8  dest_mac[6];
    u32 dest_ip;
};

struct __attribute__((packed)) udp_header {
    u16 source_port;
    u16 dest_port;
    u16 len;
    u16 crc;
};

struct __attribute__((packed)) ip_header {
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

struct __attribute__((packed)) tftp_data_header {
    u16 opcode;
    u16 block_num;
};

struct __attribute__((packed)) tftp_ack {
    u16 opcode;
    u16 block_num;
};

// Network protocol defines
#define ARP_REQUEST  0x0001
#define ARP_RESPONSE 0x0002

#define ETHER_TYPE_IPv4 0x0800
#define ETHER_TYPE_ARP  0x0806

#define HTYPE_ETHER 0x0001

#define IP_PROTOCOL_UDP 0x11

#define TFTP_OPCODE_ACK  4
#define TFTP_OPCODE_OACK 6
#define TFTP_OPCODE_DATA 3
#define TFTP_OPCODE_READ 1

#define MAC_TYPE_IPv4   0x0800
#define MAC_TYPE_ARP    0x0806

static u8 mac_addr[6];
static u8 tftp_server_mac[6];
static u32 tftp_client_port = 313;
static u32 tftp_server_port;
static u32 packet_size;
static u8* tftp_tmp_dest;
static u32 tftp_server_ip;
static u32 tftp_server_port;
static u32 tftp_client_ip;
static u32 tftp_done;

// Hex lookup table for use in MAC address conversion
static const char hex_lookup[] = "0123456789ABCDEF";

// Converts a string into network IP representation. This returns 0 if success and
// -ERR_NET if failure
i32 string_to_ip(const char* str, u32* ip_addr) {
    u32 ip = 0;
    for (u32 i = 0; i < 4; i++) {
        // We use two variables for tracking if the segment has right format
        u32 num = 0;
        u32 prev = 0;

        while (*str >= '0' && *str <= '9') {
            prev = num;
            num = num * 10 + (*str++ - '0');

            if ((prev | num) == 0 || num > 0xFF) {
                return -ERR_NET;
            }
        }
        
        // Check the formatting
        if ((i == 3 && *str != '\0') || (i != 3 && *str != '.')) {
            return -ERR_NET;
        }

        // Skip the . charcater
        str++;

        // Update the network IP variable
        ip = (ip << 8) | (num & 0xFF);
    }

    *ip_addr = ip;
    return 0;
}

// Converts a network IP address into a string. The string must be at least 16 
// characters
void ip_to_string(char* str, u32 addr) {
    for (u32 i = 4; i --> 0;) {
        // Get a new byte-segment of the IP address
        u8 segment = (addr >> (i * 8)) & 0xFF;
        u8 base = 100;

        // Prevent any starting zeros
        while (base > segment) {
            base /= 10;
        }

        // The current segment is zero
        if (base == 0) {
            *str++ = '0';
        }

        // Print the number to the buffer
        while (base) {
            *str++ = (segment / base) + '0';
            segment = segment % base;
            base = base / 10;
        }

        // Add the delimiter
        if (i) {
            *str++ = '.';
        }
    }
    *str = 0;
}

// Check is a given character is a hexadecimal character
static inline u8 is_hex(char c) {
    if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
        return 1;
    } else {
        return 0;
    }
}

// Converts a hexadecimal character to a number in range 0..15
static inline u8 hex_to_num(char hex) {
    if ((hex >= '0' && hex <= '9')) {
        return hex - '0';
    }

    // Try to clear bit 5 to convert to uppercase
    hex &= ~(1 << 5);

    if ((hex >= 'A' && hex <= 'F')) {
        return hex - 'A' + 10;
    }

    return 0;
}

// Converts a string into a network MAC address. This returns 0 if success and -ERR_NET if
// failure
i32 string_to_mac(const char* str, u8* mac) {
    for (u32 i = 0; i < 6; i++) {
        // We use two variables for tracking if the segment has right format
        u32 num = 0;
        u32 index = 2;

        while (is_hex(*str)) {
            num = num * 16 + hex_to_num(*str++);

            if (num > 0xFF || !index--) {
                return -ERR_NET;
            }
        }
        
        // Check the formatting
        if ((i == 5 && *str != '\0') || (i != 5 && *str != ':')) {
            return -ERR_NET;
        }

        // Skip the . charcater
        str++;

        // Update the network IP variable
        *mac++ = (u8)num;
    }
    return 0;
}

// Converts a network MAC address into a string. The string must be at least 18 
// characters
void mac_to_string(char* str, u8* mac, u8 lowercase) {
    u8 lower = (lowercase) ? (1 << 5) : 0;
    for (u32 i = 6; i --> 0;) {
        // Get a new byte-segment of the IP address
        u8 segment = *mac++;

        *str++ = hex_lookup[(segment >> 4) & 0xF] | lower;        
        *str++ = hex_lookup[(segment >> 0) & 0xF] | lower;        

        // Add the delimiter
        if (i) {
            *str++ = ':';
        }
    }
    *str = 0;
}

// Copies a MAC address from `source` to `dest` 
void copy_mac_addr(const u8* source, u8* dest) {
    for (u32 i = 0; i < 6; i++) {
        *dest++ = *source++;
    }
}

// Returns the MAC address of our computer. The buffer must contain at least 6 characters
void get_mac_addr(u8* mac) {
    copy_mac_addr(mac_addr, mac);
}

// Updates the MAC address of our computer
void update_mac_addr(const u8* new_mac) {
    copy_mac_addr(new_mac, mac_addr);
}

// Sends a raw MAC frame to the computer at `dest_mac`. It takes in the type field in the
// MAC header. 
static void mac_send(struct netbuf* buf, const u8* dest_mac, u16 type) {
    // Append the MAC header before the current pointer
    buf->len += sizeof(struct mac_header);
    buf->ptr -= sizeof(struct mac_header);

    // Get a pointer to the MAC header
    struct mac_header* header = (struct mac_header *)buf->ptr;

    store_be16(type, &header->type);
    get_mac_addr(header->source_mac);
    copy_mac_addr(dest_mac, header->dest_mac);

    // Send the raw ethernet frame
    nic_send(buf);
}

// Sends an IP packet to the TFTP server identified by the global address configuration
static void ip_send(struct netbuf* buf) {
    // Append the IPv4 header
    buf->ptr -= sizeof(struct ip_header);
    buf->len += sizeof(struct ip_header);

    // Get a pointer to the IP header
    struct ip_header* ipv4_header = (struct ip_header *)buf->ptr;

    // Fill in the fields
    ipv4_header->version_ihl = (4 << 4) | 5;
    ipv4_header->dscp_ecn = 0;
    store_be16(buf->len, &ipv4_header->len);
    store_be16(0, &ipv4_header->id);
    store_be16(0, &ipv4_header->frag_off);
    store_be16(0, &ipv4_header->crc);

    ipv4_header->protocol = IP_PROTOCOL_UDP;
    ipv4_header->ttl = 0xFF;

    store_be32(tftp_client_ip, &ipv4_header->source_ip);
    store_be32(tftp_server_ip, &ipv4_header->dest_ip);

    mac_send(buf, tftp_server_mac, MAC_TYPE_IPv4);
}

// Sends a UDP packet to the TFTP server identified by the global configuration
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

    ip_send(buf);
}

// Lookup table for fast processing of the ARP
static const u8 arp_fragment[8] = { 0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x02 };

// This will do a blocking ARP request. This returns 0 if success and -ERR_NET if the host
// does not respond to the ARP. 
i32 arp_get_mac_addr(u32 ip_addr, u8* mac_addr) {
    struct netbuf* buf = alloc_netbuf();
    struct arp_header* arp = (struct arp_header *)buf->ptr;

    // Fill in the ARP stuff
    arp->mac_len = 6;
    arp->ip_len = 4;
    store_be16(HTYPE_ETHER, &arp->hardware_type);
    store_be16(ETHER_TYPE_IPv4, &arp->protocol_type);
    store_be16(ARP_REQUEST, &arp->operation);

    // Fill in our MAC address
    get_mac_addr(arp->source_mac);
    store_be32(0, &arp->source_ip);
    store_be32(ip_addr, &arp->dest_ip);

    buf->len = sizeof(struct arp_header);

    // Send the ARP packet
    u8 broad_mac[6] = { [0 ... 5] = 0xFF };
    mac_send(buf, broad_mac, MAC_TYPE_ARP);

    // Poll for the reponse
    for (u32 i = 0; i < 1000; i++) {
        struct netbuf* buf = nic_receive();
        if (buf == NULL) {
            continue;
        }
        u32 valid = 1;
        
        // Check the ether type field
        if (read_be16(buf->ptr + 12) != ETHER_TYPE_ARP) {
            valid = 0;
        }

        // Skip the MAC header
        arp = (struct arp_header *)(buf->ptr + 14);

        // Compare the first framgent
        u8* arp_start = (u8 *)arp;
        for (u32 i = 0; i < sizeof(arp_fragment); i++) {
            if (arp_fragment[i] != arp_start[i]) {
                valid = 0;
            }
        }

        // Compare our MAC address
        get_mac_addr(broad_mac);
        for (u32 i = 0; i < 6; i++) {
            if (broad_mac[i] != arp->dest_mac[i]) {
                valid = 0;
            }
        }

        // Check the IP address
        if (read_be32(&arp->source_ip) != ip_addr) {
            valid = 0;
        }

        if (valid) {
            // Copy the MAC address to the user buffer
            copy_mac_addr(arp->source_mac, mac_addr);
            free_netbuf(buf);
            return 0;
        }
        free_netbuf(buf);
    }
    
    return -ERR_NET;
}

// Sends a gratuitous ARP. This basically broadcast our IP and MAC address on the network.
// This may prevent the server from sending us ARP requests
void send_gratuitous_arp(u32 ip_addr) {
    // Send an ARP request
    struct netbuf* buf = alloc_netbuf();
    struct arp_header* arp = (struct arp_header *)buf->ptr;

    // Fill in the ARP stuff
    arp->mac_len = 6;
    arp->ip_len = 4;
    store_be16(HTYPE_ETHER, &arp->hardware_type);
    store_be16(ETHER_TYPE_IPv4, &arp->protocol_type);
    store_be16(ARP_RESPONSE, &arp->operation);

    // Fill in our MAC address
    get_mac_addr(arp->source_mac);
    store_be32(ip_addr, &arp->source_ip);

    // Use our IP as the destination just for confirmation
    store_be32(ip_addr, &arp->dest_ip);

    const u8 broad_mac[6] = { [0 ... 5] = 0xFF };
    copy_mac_addr(broad_mac, arp->dest_mac);

    // Update the length of the ARP payload
    buf->len = sizeof(struct arp_header);

    // Broadcast the ARP packet
    mac_send(buf, broad_mac, MAC_TYPE_ARP);
}

// Handles and incoming ARP packet. If this is a request we send an ARP response
void handle_arp(struct netbuf* buf) {
    struct arp_header* arp_header = (struct arp_header *)buf->ptr;

    if (read_be16(&arp_header->operation) == ARP_REQUEST && 
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
        copy_mac_addr(arp_header->source_mac, arp_header_resp->dest_mac);
        store_be32(tftp_client_ip, &arp_header_resp->source_ip);
        store_be32(read_be32(&arp_header->source_ip), &arp_header_resp->dest_ip);

        resp->len = sizeof(struct arp_header);    
        mac_send(resp, arp_header->source_mac, MAC_TYPE_ARP);
    }
}

// Sends a TFTP ACK to the server
void tftp_ack(u32 block_num) {
    struct netbuf* buf = alloc_netbuf();
    buf->len = sizeof(struct tftp_ack);

    struct tftp_ack* ack = (struct tftp_ack *)buf->ptr;
    
    store_be16(TFTP_OPCODE_ACK, &ack->opcode);
    store_be16(block_num, &ack->block_num);

    udp_send(buf, tftp_client_port, tftp_server_port);
}

// Current sequence number
static u32 curr_sequence_num = 0;

// Handles an incoming UDP packet. If this is a TFTP to our port it will check the 
// sequence number and conditianally write the data to memory. This will also ACK any 
// TFTP data packets
void handle_udp(struct netbuf* buf) {
    struct udp_header* header = (struct udp_header *)buf->ptr;

    if (read_be16(&header->dest_port) == tftp_client_port) {
        // Update the TFTP server port
        if (tftp_server_port == 0) {
            tftp_server_port = read_be16(&header->source_port);
        }

        // Get the lenght
        u16 len = read_be16(&header->len) - sizeof(struct udp_header) - 
            sizeof(struct tftp_data_header);

        // Advance the pointer
        buf->ptr += sizeof(struct udp_header);
        struct tftp_data_header* tftp_header = (struct tftp_data_header *)buf->ptr;

        // Check if the TFTP is a data packet
        if (read_be16(&tftp_header->opcode) == TFTP_OPCODE_DATA) {

            // Get the sequence number
            u16 sequence_num = read_be16(&tftp_header->block_num);
            if (sequence_num == (curr_sequence_num + 1)) {
                // Get the data pointer
                u8* src = buf->ptr + sizeof(struct tftp_data_header);

                // Copy the file fragment to memory
                for (u32 i = 0; i < len; i++) {
                    *tftp_tmp_dest++ = *src++;
                }

                tftp_ack(sequence_num);
                curr_sequence_num++;

                // ZLP or short packes is interpreted as the EOF marker
                if (len != packet_size) {
                    tftp_done = 1;
                }
            }
        } else if (read_be16(&tftp_header->opcode) == TFTP_OPCODE_OACK) {

            // Check if the OACK contains "blksize" with the wrong size
            buf->ptr += 2;
            u32 success = 1;
            const char* block_size_str = "blksize";
            while (*block_size_str) {
                if (*buf->ptr++ != *block_size_str++) {
                    success = 0;
                }
            }
            // Check the terminating zero
            if (*buf->ptr++) {
                success = 0;
            }
            const char* size_str = TFTP_DATA_SIZE;
            while (*size_str) {
                if (*buf->ptr++ != *size_str++) {
                    success = 0;
                }
            }
            // Check the terminating zero
            if (*buf->ptr++) {
                success = 0;
            }

            if (!success) {
                panic("TFTP block size error");
            }

            // Send the ACK
            tftp_ack(0);
        }
    }
}

// Handles an incoming packet on the TFTP link
void handle_tftp(struct netbuf* buf) {
    struct ip_header* header = (struct ip_header *)buf->ptr;

    // Check the IP, len and protocol
    if (header->protocol == 0x11 && read_be32(&header->dest_ip) == tftp_client_ip) {
        // Skip the IP header
        buf->ptr += sizeof(struct ip_header);
        handle_udp(buf);
    }
}

// Performs a TFTP read request
void tftp_request(const char* file_name, const char* block_size) {
    struct netbuf* buf = alloc_netbuf();
    
    u8* ptr = buf->ptr;

    store_be16(TFTP_OPCODE_READ, ptr);
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

// Tries to read the given file from TFTP server. This will write the file to `dest`
i32 tftp_read_file(void* dest) {

    // Clear the server TFTP port
    tftp_server_port = 0;
    curr_sequence_num = 0;

    // Get the TFTP size
    packet_size = 0;
    const char* size_ptr = TFTP_DATA_SIZE;
    const char* tmp = size_ptr;
    while (*tmp) {
        packet_size = packet_size * 10 + (*tmp++ - '0');
    }
    curr_sequence_num = 0;

    // Get the network IP address
    string_to_ip(TFTP_SERVER_IP, &tftp_server_ip);
    string_to_ip(TFTP_CLIENT_IP, &tftp_client_ip);

    boot_start_timer();

    // Get the MAC address of the host computer
    while (arp_get_mac_addr(tftp_server_ip, tftp_server_mac) != 0);

    // Send a gratuitous ARP advertising our MAC address
    send_gratuitous_arp(tftp_client_ip);

    // This is where we copy the file
    tftp_tmp_dest = dest;
    tftp_request(TFTP_FILE_NAME, TFTP_DATA_SIZE);

    // This loop will read the file
    tftp_done = 0;
    while (tftp_done == 0) {
        // Try to receive a raw packet
        struct netbuf* buf = nic_receive();
        if (buf == NULL) {
            continue;
        }

        // We have a new packet
        struct mac_header* mac_header = (struct mac_header *)buf->ptr;

        // Skip MAC header
        buf->ptr += sizeof(struct mac_header);

        if (read_be16(&mac_header->type) == MAC_TYPE_ARP) {
            handle_arp(buf);
        } else if (read_be16(&mac_header->type) == MAC_TYPE_IPv4) {
            handle_tftp(buf);
        }

        // Free the netbuffer after use
        free_netbuf(buf);
    }

    boot_message("Starting new kernel at {p}\n", dest);

    // We have a new image in memory - execute it
    void (*new_kernel)() = (void *)((u32)dest);
    new_kernel();

    return 0;
}

// Start the networking
void tftp_init() {
    kprint("Starting TFTP/IP soft reboot stack\n");
    netbuf_init();

    // Call the device specific NIC initialization routine
    nic_init();

    // Update our MAC address
    u8 mac[6];
    if (string_to_mac(TFTP_CLIENT_MAC, mac) == 0) {
        update_mac_addr(mac);
    }

    kprint("TFTP stack ready\n");

    // TODO: Some packets are droppen unless this delay is present 
    for (u32 i = 0; i < 500000; i++) {
        asm ("nop");
    }    
}
