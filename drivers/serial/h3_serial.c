// Serial driver for Allwinner H3 chips (kernel driver)

#include <chaos/kprint.h>
#include <h3/regmap.h>

void kprint_from_buf(const char* buf, u32 size) {
    struct uart_reg* const hw = UART0_REG;
    
    while (size--) {
        // Wait for empty FIFO
        while (!(hw->lsr & (1 << 5)));
        hw->thr = *buf++;
    }
}
