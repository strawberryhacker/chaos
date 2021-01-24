## Summary

The Chaos OS is a bare metal microkernel targeting embedded systems. It will include various bare metal example in several languages and of course several device server examples. Network stack and file systems are just implied. 

### Support

I plan on developing the kernel in parallel on these targets

- SAMA5D27 (single-core Cortex-A5)
- SAMA5D4  (single-core Cortex-A5)
- Orange Pi PC (quad-core Cortex-A7)
- Orange Pi (quad-core Cortex-A7 virtualized on QEMU)
- FireFly RK3288 (quad-core Cortex-A17)

### Structure 

The Chaos kernel will be loaded by u-boot into external memory. I do not intend to write any board spesific bootloader. The kernel will have a relocatable PIC stage for relocating itself to the beginning of DDR. This opens the possibilities for soft reboot, in which we can replace a running kernel. This will definitely outperform a u-boot reset. The kernel will have a TFTP component embedded into the kernel which will take care of the soft reboot.
