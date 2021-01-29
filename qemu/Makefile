qemu:
	@qemu-system-arm -M orangepi-pc -nographic -kernel ./u-boot \
	-net nic,id=net0 -netdev user,id=hub0port0,tftp=/home/strawberry/tftp \
	-sd qemu_disk.img -s