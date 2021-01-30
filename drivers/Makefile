# Makefile for the kernel drivers

src-y += drivers/kprint.c
src-y += drivers/panic.c
src-y += drivers/assert.c
src-y += drivers/net/netbuf.c
src-y += drivers/net/network.c

src-$(sama5d2) += drivers/serial/sama5d2_serial.c
src-$(sama5d2) += drivers/gpio/sama5d2_gpio.c
src-$(sama5d2) += drivers/clk/sama5d2_clk.c
src-$(sama5d2) += drivers/nic/sama5d2_nic.c

src-$(h3) += drivers/serial/h3_serial.c
