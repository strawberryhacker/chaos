# Top level makefile

top       = $(shell pwd)
build_dir = $(top)/build
obj_dir   = $(build_dir)/objects
tftp_dir  = /home/strawberry/tftp
com       = /dev/ttyUSB0

# Location of the configuration files
config_dir  = $(top)/config
config_file = $(config_dir)/$(target).cfg

# Generally we require a configuration file to be present when the build starts. In some
# cases it's no necessary. These targets are the exception 
target_list = help clean objclean qemu

# Check if the user has provided a configuration file
ifeq ($(filter $(MAKECMDGOALS), $(target_list)),)

# Test if the config file exist
ifneq ($(shell test -e $(config_file) && echo -n yes), yes)
$(error Please provide a valid config file)
endif

# Include the config file
include $(config_file)

# Include the required dependencies
include $(top)/arch/Makefile
include $(top)/kernel/Makefile
include $(top)/misc/Makefile
include $(top)/entry/Makefile
include $(top)/drivers/Makefile
include $(top)/include/Makefile
include $(top)/config/Makefile

# Check that the configuration file defines all the required variables
ifndef folder_name
$(error Error in configuration file)
endif

ifndef tftp_name
$(error Error in configuration file)
endif

ifndef target_name
$(error Error in configuration file)
endif

ifndef linker-script-y
$(error Linker script is not provided)
endif

ifndef link_location
$(error Link location is not specified)
endif

ifndef ddr_size
$(error DDR size is not given)
endif

ifndef ddr_start
$(error DDR size is not given)
endif

# In case of soft reboot we allow overriding the TFTP configuration
ifdef tftp_client_ip
cpflags += -DTFTP_CLIENT_IP=\"$(tftp_client_ip)\"
endif

ifdef tftp_server_ip
cpflags += -DTFTP_SERVER_IP=\"$(tftp_server_ip)\"
endif

ifdef tftp_client_mac
cpflags += -DTFTP_CLIENT_MAC=\"$(tftp_client_mac)\"
endif

ifdef tftp_data_size
cpflags += -DTFTP_DATA_SIZE=\"$(tftp_data_size)\"
endif

ifdef tftp_name
cpflags += -DTFTP_FILE_NAME=\"$(tftp_name)\"
endif

# Pass some information to the linker such as the link location and DDR info
ldflags += -T$(top)/$(linker-script-y)
ldflags += -Wl,--defsym=link_location=$(link_location)
ldflags += -Wl,--defsym=ddr_start_macro=$(ddr_start)
ldflags += -Wl,--defsym=ddr_size_macro=$(ddr_size)
endif

# ARM cross-compilers
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
obj      = $(patsubst %.c,%.o, $(src_path)) $(patsubst %.s,%.o, $(asm_path))
deps     = $(addprefix $(top)/, $(deps-y))

# Global path of the target to build excluding extension
global_target_name = $(build_dir)/$(folder_name)/$(target_name)

.PHONY: clean help elf lss bin all start objclean debug
.SECONDARY: $(obj)

# Main build rule
all: start elf lss bin debug

# This is the files we have to build
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
	@echo " " CC $(patsubst $(top)/%,%, $<)
	@$(cc) $(cpflags) $(cflags) -c $< -o $@

$(obj_dir)/%.o: $(top)/%.s $(deps)
	@mkdir -p $(dir $@)
	@echo " " AS $(patsubst $(top)/%,%, $<)
	@$(as) $(asflags) -c $< -o $@

# This is only used during developement to talk to the custom SAMA5 bootloader
install: all
	@echo Installing binary on SAMA5 board
	@python3 -B $(top)/scripts/kernel_load.py $(com) $(global_target_name).bin

# Debug support for Orange Pi QEMU
ifeq ($(target),orangepi_pc)
debug:
	@$(gdb) -f $(global_target_name).elf -x $(top)/scripts/orangepi_qemu.gdb
else
debug:
	@echo Debugging currently not supported
endif

# Deletes all object files, but leaves the binaries untouched 
objclean:
	@rm -r -f $(obj_dir)/

# Deletes all build objects
clean:
	@rm -r -f $(build_dir)

help:
	@echo "In order to build, you must provide some information about "
	@echo "the board. We have written some configuration files to help"
	@echo "you with that. These are placed in: "
	@echo
	@echo "   > project_dir/config"
	@echo
	@echo "You need to set the board variable equal the configuration file "
	@echo "you want to use. Note that the extension should be excluded. "
	@echo "This can be done in two ways: "
	@echo
	@echo "   > make board=some_configuration_file"
	@echo
	@echo or
	@echo 
	@echo "   > export board=some_configuration_file"
	@echo "   > make"
	@echo
	@echo "Using the last option you don't need to specify the board "
	@echo "each time, and you can enjoy the simplicity of make"
