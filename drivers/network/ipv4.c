// IPv4 networking module for the kernel soft reboot

#include <chaos/ipv4.h>
#include <chaos/kprint.h>
#include <chaos/status.h>

i32 string_to_ipv4(const char* str, u32* addr) {
    u32 ip = 0;
    for (u32 i = 0; i < 4; i++) {

        // Segment number
        u32 num = 0;
        u32 index = 0;
        while (*str >= '0' && *str <= '9') {
            num = num * 10 + (*str++ - '0');
            index++;
        }

        // We are only allowing 1..3 characters per segment
        if (index == 0 || index > 3) {
            return -ERR_NET;
        }

        // Check the range of the numbers so that we reject e.g. 999
        if (num > 0xFF) {
            return -ERR_NET;
        }
        
        // Check the formatting
        if ((i == 3 && *str != 0) || (i != 3 && *str != '.')) {
            kprint("HERE\n");
            return -ERR_NET;
        }

        // Skip the . charcater
        str++;

        // Update the network IP variable
        ip = (ip << 8) | (num & 0xFF);
    }

    *addr = ip;
    return 0;
}

// Converts a network IPv4 address into a string. The string must be at least 16 
// characters
void ipv4_to_string(char* str, u32 addr) {
    for (u32 i = 4; i --> 0;) {
        // Get a new byte-segment of the IP address
        u8 seg = (addr >> (i * 8)) & 0xFF;
        u8 base = 100;

        // Prevent any starting zeros
        while (base > seg) {
            base /= 10;
        }

        if (base == 0) {
            *str++ = '0';
        }

        // Print the number to the buffer
        while (base) {
            *str++ = (seg / base) + '0';
            seg = seg % base;
            base = base / 10;
        }

        // Add the .
        if (i) *str++ = '.';
    }
    *str = 0;
}
