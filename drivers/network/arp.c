// ARP network module for the kernel soft reboot

#include <chaos/arp.h>
#include <chaos/kprint.h>
#include <chaos/list.h>
#include <chaos/status.h>
#include <chaos/netbuf.h>
#include <chaos/mem.h>
#include <chaos/mac.h>
#include <chaos/nic.h>
#include <stddef.h>

static const u8 arp_fragment[8] = { 0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x02 };

i32 arp_get_mac_addr(u32 ipv4_addr, u8* mac_addr) {
    // Send an ARP request
    struct netbuf* buf = alloc_netbuf();
    struct arp_header* arp = (struct arp_header *)buf->ptr;

    // Fill in the ARP stuff
    arp->mac_len = 6;
    arp->ip_len = 4;
    store_be16(0x0001, &arp->hardware_type);
    store_be16(0x0800, &arp->protocol_type);
    store_be16(0x0001, &arp->operation);

    // Fill in our MAC address
    get_mac_addr(arp->source_mac);
    store_be32(0x0000, &arp->source_ip);
    store_be32(ipv4_addr, &arp->dest_ip);

    buf->len = sizeof(struct arp_header);

    // Send the ARP packet
    u8 mac[6] = { [0 ... 5] = 0xFF };
    mac_send_to(buf, mac, MAC_TYPE_ARP);

    // Poll for the reponse
    while (1) {
        struct netbuf* buf = nic_receive();
        if (buf == NULL) {
            continue;
        }

        // Check the ether type field
        u32 valid = 1;
        if (read_be16(buf->ptr + 12) != MAC_TYPE_ARP) {
            goto error;
        }

        // Skip the MAC header
        arp = (struct arp_header *)(buf->ptr + 14);

        // Compare the first framgent
        u8* arp_start = (u8 *)arp;
        for (u32 i = 0; i < sizeof(arp_fragment); i++) {
            if (arp_fragment[i] != arp_start[i]) {
                goto error;
            }
        }

        // Compare our MAC address
        get_mac_addr(mac);
        for (u32 i = 0; i < 6; i++) {
            if (mac[i] != arp->dest_mac[i]) {
                goto error;
            }
        }

        // Check the IP address
        if (read_be32(&arp->source_ip) != ipv4_addr) {
            goto error;
        }

        // Copy the buffer pointer
        copy_mac_addr(arp->source_mac, mac_addr);

        free_netbuf(buf);
        return 0;
error:
        free_netbuf(buf);
    }
    
    return 0;
}

void send_garp(u32 ipv4_addr) {
    // Send an ARP request
    struct netbuf* buf = alloc_netbuf();
    struct arp_header* arp = (struct arp_header *)buf->ptr;

    // Fill in the ARP stuff
    arp->mac_len = 6;
    arp->ip_len = 4;
    store_be16(0x0001, &arp->hardware_type);
    store_be16(0x0800, &arp->protocol_type);
    store_be16(0x0002, &arp->operation);

    // Fill in our MAC address
    get_mac_addr(arp->source_mac);
    store_be32(ipv4_addr, &arp->source_ip);
    store_be32(ipv4_addr, &arp->dest_ip);

    // Send the ARP packet
    u8 mac[6] = { [0 ... 5] = 0xFF };

    for (u32 i = 0; i < 6; i++) {
        arp->dest_mac[i] = mac[i];
    }

    buf->len = sizeof(struct arp_header);    
    mac_send_to(buf, mac, MAC_TYPE_ARP);
}
