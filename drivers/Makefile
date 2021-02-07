# Makefile for the kernel drivers

src-y += drivers/kprint.c
src-y += drivers/panic.c
src-y += drivers/assert.c
src-y += drivers/boot.c
src-$(soft_reboot) += drivers/tftp.c
src-$(soft_reboot) += drivers/nic/netbuf.c

src-$(sama5d2) += drivers/serial/sama5d2_serial.c
src-$(sama5d2) += drivers/gpio/sama5d2_gpio.c
src-$(sama5d2) += drivers/clk/sama5d2_clk.c
src-$(sama5d2) += drivers/timer/sama5d2_timer.c

# The NIC driver should only be included if soft reboot is enabled. Otherwise some 
# dependencies are removed and the build will fail
ifeq ($(soft_reboot),y)
src-$(sama5d2) += drivers/nic/sama5d2_nic.c
endif

src-$(h3) += drivers/serial/h3_serial.c
