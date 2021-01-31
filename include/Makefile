# Makefile for the include directory

cpflags += -I$(top)/include

deps-y += include/chaos/print_format.h
deps-y += include/chaos/types.h
deps-y += include/chaos/panic.h
deps-y += include/chaos/assert.h
deps-y += include/chaos/kprint.h
deps-y += include/chaos/nic.h
deps-y += include/chaos/network.h
deps-y += include/chaos/netbuf.h
deps-y += include/chaos/list.h
deps-y += include/chaos/cache.h

deps-$(sama5d2) += include/sama5d2/regmap.h
deps-$(sama5d2) += include/sama5d2/sama5d2_clk.h
deps-$(sama5d2) += include/sama5d2/sama5d2_gpio.h

deps-$(h3) += include/h3/regmap.h
