qemu-system-x86_64 -kernel ./linux-6.18/arch/x86/boot/bzImage -append console=ttyS0 -initrd rootfs.cpio.gz -enable-kvm -nographic -m 256 -fsdev local,id=hostshare,path=./share,security_model=none -device virtio-9p-pci,fsdev=hostshare,mount_tag=hostshare 

find . | cpio -o --format=newc | gzip > ../rootfs.cpio.gz
