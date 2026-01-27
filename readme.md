# Homework 3

## Instruction how to run

0. make sure you are running on x86 platform
1. make sure qemu-system-x86_64 is installed
2. run in repo root directory:
```
qemu-system-x86_64 -kernel ./linux-6.18/arch/x86/boot/bzImage -append console=ttyS0 -initrd rootfs.cpio.gz -enable-kvm -nographic -m 256 -fsdev local,id=hostshare,path=./share,security_model=none -device virtio-9p-pci,fsdev=hostshare,mount_tag=hostshare
```
3. run ./mnt/runtest.sh in virtual environment
4. make sure "All tests passed" appears in vitual env console

## Compile kernel & module

1. run `yes "n" | make oldconfig` in linux-kernel directory
2. compile kernel by running make -j`nproc`
3. compile module by running make build in modules directory

### Troubleshoting

In case no output in terminal when running qemu try changing console=ttyS0 to ttyS1, 2, 3, so on
