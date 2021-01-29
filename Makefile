# Top level makefile

top = $(shell pwd)
build_dir = $(top)/build
obj_dir = $(build_dir)/objects
tftp_dir = /home/strawberry/tftp
com = /dev/ttyUSB0

# Location of the configuration files
config_dir = $(top)/config
config_file = $(config_dir)/$(board).cfg

# List over targets that doesn't require the configuration file to be present
target_list = help clean objclean qemu

# Check if the user has provided a configuration file
ifeq ($(filter $(MAKECMDGOALS), $(target_list)),)
ifneq ($(shell test -e $(config_file) && echo -n yes), yes)
$(error Please provide a valid config file)
endif

# Include the config file
include $(config_file)

# Include the required dependencies
include $(top)/arch/Makefile
include $(top)/kernel/Makefile
include $(top)/lib/Makefile
include $(top)/entry/Makefile
include $(top)/drivers/Makefile
include $(top)/include/Makefile

# Check that the name variables in the config file are correct
ifeq ($(folder_name),)
$(error Error in configuration file)
endif

# Check for a valid TFTP file name
ifeq ($(tftp_name),)
$(error Error in configuration file)
endif

# Check for a valid target name
ifeq ($(target_name),)
$(error Error in configuration file)
endif

# Verify that we have a valid linker script
ifeq ($(linker-script-y),)
$(error Linker script is not provided)
endif

# Check that we have a valid link location
ifeq ($(link_location),)
$(error Link location is not specified)
endif

# Check that we have a valid load address
ifeq ($(load_location),)
$(error Load location is not specified)
endif

# Check that we have a valid DDR size
ifeq ($(ddr_size),)
$(error DDR size is not given)
endif

# Check that we have a valid DDR start address
ifeq ($(ddr_start),)
$(error DDR size is not given)
endif

# Update the linker flags
ldflags += -T$(top)/$(linker-script-y)
ldflags += -Wl,--defsym=link_location=$(link_location)
endif

# Compilers
cc      = arm-none-eabi-gcc
objdump = arm-none-eabi-objdump
objcopy = arm-none-eabi-objcopy
as      = arm-none-eabi-as
gdb     = gdb-multiarch

# Set the compiler flags
cflags += -x c -O1 -g3 -fdata-sections -mlong-calls -Wall
cflags += -std=gnu99 -c -ffunction-sections -march=armv7-a
cflags += -Wno-unused-function -Wno-unused-variable
cflags += -ffreestanding

ldflags += -Wl,--start-group -Wl,--end-group
ldflags += -Wl,--gc-sections -march=armv7-a

asflags += -march=armv7-a -g3

# Process the build dependencies
src_path = $(addprefix $(obj_dir)/, $(src-y))
asm_path = $(addprefix $(obj_dir)/, $(asm-y))
obj = $(patsubst %.c,%.o, $(src_path)) $(patsubst %.s,%.o, $(asm_path))

# If a dependency change the entire project will be rebuilt
deps = $(addprefix $(top)/, $(headers-y))

# Global path of the target to build excluding extension
global_target_name = $(build_dir)/$(folder_name)/$(target_name)

.PHONY: clean help elf lss bin all start objclean debug
.SECONDARY: $(obj)

all: start elf lss bin debug

elf: $(global_target_name).elf
lss: $(global_target_name).lss
bin: $(global_target_name).bin

# Kill QEMU if it is running
start:
	@pkill qemu || true

%.elf: $(obj)
	@mkdir -p $(dir $@)
	@$(cc) $(ldflags) $^ -o $@

%.lss: %.elf
	@mkdir -p $(dir $@)
	@$(objdump) -h -S $< > $@

# Builds the binary and copies it to the TFTP directory
%.bin: %.elf
	@mkdir -p $(dir $@)
	@$(objcopy) -O binary $< $@
	@echo
	@cp $@ $(tftp_dir)/$(tftp_name)
	@echo Binary is copied to TFTP directory

$(obj_dir)/%.o: $(top)/%.c $(deps)
	@mkdir -p $(dir $@)
	@echo " " CC $(patsubst $(dir $(top))%,%, $<)
	@$(cc) $(cpflags) $(cflags) -c $< -o $@

$(obj_dir)/%.o: $(top)/%.s $(deps)
	@mkdir -p $(dir $@)
	@echo " " AS $(patsubst $(dir $(top))%,%, $<)
	@$(as) $(asflags) -c $< -o $@

# This is only used during developement to talk to the custom SAMA5 bootloader
install: all
	@echo Installing binary on SAMA5 board
	@python3 -B $(top)/scripts/kernel_load.py $(com) $(global_target_name).bin

# Debug support for Orange Pi QEMU
ifeq ($(board),orangepi_pc)
debug:
	@$(gdb) -f $(global_target_name).elf -x $(top)/scripts/orangepi_qemu.gdb
else
debug:
	@echo Debugging currently not supported
endif

objclean:
	@rm -r -f $(obj_dir)/

clean:
	@rm -r -f $(build_dir)

help:
	@echo "Just provide a configuraton file and run makefile"
	@echo "- either make board = <your board>"
	@echo " - or exte"
