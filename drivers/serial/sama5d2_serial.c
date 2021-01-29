// Serial kernel driver for SAMA5D2 chips

#include <chaos/kprint.h>
#include <sama5d2/regmap.h>

void kprint_from_buf(const char* buf, u32 size) {
    struct uart_reg* const hw = UART1;

    while (size--) {
        // Make sure we terminatie the line with CR-LF
        if (*buf == '\n') {
            while (!(hw->sr & (1 << 1)));
            hw->thr = '\r';
        }
        while (!(hw->sr & (1 << 1)));
        hw->thr = *buf++;
    }
}
