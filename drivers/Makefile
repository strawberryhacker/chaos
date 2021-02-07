# Makefile for the kernel drivers

src-y += drivers/kprint.c
src-y += drivers/panic.c
src-y += drivers/assert.c
src-y += drivers/boot.c

src-y += drivers/network/netbuf.c
src-y += drivers/network/network.c
src-y += drivers/network/arp.c
src-y += drivers/network/ipv4.c
src-y += drivers/network/mac.c
src-y += drivers/network/tftp.c

src-$(sama5d2) += drivers/serial/sama5d2_serial.c
src-$(sama5d2) += drivers/gpio/sama5d2_gpio.c
src-$(sama5d2) += drivers/clk/sama5d2_clk.c
src-$(sama5d2) += drivers/nic/sama5d2_nic.c
src-$(sama5d2) += drivers/timer/sama5d2_timer.c

src-$(h3) += drivers/serial/h3_serial.c
