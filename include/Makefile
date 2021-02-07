# Makefile for the include files

cpflags += -I$(top)/include

deps-y += include/chaos/print_format.h
deps-y += include/chaos/types.h
deps-y += include/chaos/panic.h
deps-y += include/chaos/assert.h
deps-y += include/chaos/kprint.h
deps-y += include/chaos/nic.h
deps-y += include/chaos/list.h
deps-y += include/chaos/cache.h
deps-y += include/chaos/timer.h
deps-y += include/chaos/boot_message.h

deps-$(soft_reboot) += include/chaos/netbuf.h
deps-$(soft_reboot) += include/chaos/tftp.h

deps-$(sama5d2) += include/sama5d2/regmap.h
deps-$(sama5d2) += include/sama5d2/sama5d2_clk.h
deps-$(sama5d2) += include/sama5d2/sama5d2_gpio.h

deps-$(h3) += include/h3/regmap.h
