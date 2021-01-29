// Register map definitions for SAMA5D2 chips

#ifndef H3_REGMAP_H
#define H3_REGMAP_H

#include <chaos/types.h>

struct uart_reg {
    union {
        __r u32 rbr;
        __w u32 thr;
        _rw u32 dll;
    };
    union {
        _rw u32 dlh;
        _rw u32 ier;
    };
    union {
        _rw u32 iir;
        _rw u32 fcr;
    };
    _rw u32 lcr;
    _rw u32 mcr;
    _rw u32 lsr;
    _rw u32 msr;
    _rw u32 sch;
    __r u32 reserved0[23];
    _rw u32 usr;
    _rw u32 tfl;
    _rw u32 rfl;
    __r u32 reserved1[7];
    _rw u32 halt;
};

#define UART0_REG ((struct uart_reg *)0x01C28000)
#define UART1_REG ((struct uart_reg *)0x01C28400)
#define UART2_REG ((struct uart_reg *)0x01C28800)
#define UART3_REG ((struct uart_reg *)0x01C28C00)
#define UARTr_REG ((struct uart_reg *)0x01F02800)

#endif
