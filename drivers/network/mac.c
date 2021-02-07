// MAC network module for the kernel soft reboot

#include <chaos/mac.h>
#include <chaos/mem.h>
#include <chaos/nic.h>

static const u8 mac_addr[] = { 0xC0, 0xDE, 0xBA, 0xBE, 0xCA, 0xFE};

void copy_mac_addr(const u8* source, u8* dest) {
    for (u32 i = 0; i < 6; i++) {
        *dest++ = *source++;
    }
}

// Returns the MAC address of the computer. The buffer must contain at least 8 characters
void get_mac_addr(u8* mac) {
    copy_mac_addr(mac_addr, mac);
}

void mac_send_to(struct netbuf* buf, u8* dest_mac, u16 type) {
    // Append the MAC header before the current pointer
    buf->ptr -= 2;
    store_be16(type, buf->ptr);

    buf->ptr -= 6;
    get_mac_addr(buf->ptr);

    buf->ptr -= 6;
    copy_mac_addr(dest_mac, buf->ptr);

    buf->len += 14;

    // Send the raw ethernet frame
    nic_send(buf);
}

void mac_send_raw(struct netbuf* buf) {
    // Append the MAC header before the current pointer

}
