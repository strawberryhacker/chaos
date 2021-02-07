# Makefile for the kernel drivers

src-y += drivers/kprint.c
src-y += drivers/panic.c
src-y += drivers/assert.c
src-y += drivers/boot_message.c
src-$(soft_reboot) += drivers/tftp.c
src-$(soft_reboot) += drivers/nic/netbuf.c

# SAMA5D27 files
src-$(sama5d2) += drivers/serial/sama5d2_serial.c
src-$(sama5d2) += drivers/gpio/sama5d2_gpio.c
src-$(sama5d2) += drivers/clk/sama5d2_clk.c
src-$(sama5d2) += drivers/timer/sama5d2_timer.c

ifeq ($(soft_reboot),y)
src-$(sama5d2) += drivers/nic/sama5d2_nic.c
endif

# Allwinner H3 files
src-$(h3) += drivers/serial/h3_serial.c
src-$(h3) += drivers/timer/h3_timer.c

ifeq ($(soft_reboot),y)
src-$(h3) += drivers/nic/h3_nic.c
endif
