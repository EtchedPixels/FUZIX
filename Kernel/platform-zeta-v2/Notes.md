# README for Zeta SBC V2

## TODO

- ROMWBW Support
- ParPortProp full SD/VDU/Serial support
- Swap
- Move to tinydisk ?

## UNA BIOS

The file "Kernel/platform-zeta-v2/diskboot.bin" is an UNA BIOS compatible
boot sector which will allow the system to be booted directly from a PPIDE 
drive. The boot sector will load the kernel image starting from the third
sector of the drive (ie counting from 0, sector number 2).

The kernel image (fuzix.bin) should be written to the disk drive from the third
sector onwards, in the space before the first partition starts. Be careful to
ensure there is sufficient space before the first partition to hold the kernel
or you risk overwriting the filesystem in the first partition! Modern disk
partitioning programs generally start the first partition at 1MB (sector 2048)
and thus allow plenty of space.

To check the space before the first partition, use "fdisk -l /dev/sdX" and
check the value in the "Start" column for the first partition. If this number
is 128 or larger, there is sufficient space.

To write the boot sector to a disk drive, without overwriting the partition
table, you can use dd as follows;
    dd if=diskboot.bin bs=446 count=1 of=/dev/sdX

To write the kernel image to the disk drive, use dd as follows;
    dd if=fuzix.bin bs=512 seek=2 of=/dev/sdX

Load the disk into the machine and boot. At the "Boot UNA unit number or ROM?"
prompt, type the number of the disk unit to boot from.

## Flash ROM

Currently the system can be booted from the Flash ROM. 

Memory allocation (16 KiB pages):
- ROM pages:	0 - 31
  - Kernel image:	0 - 3 - copied by bootrom.s to pages 32 - 35
  - ROM disk image:	4 - 31
- RAM pages:	32 - 63
  - Kernel pages:	32 - 34
  - Common page:	35
  - User space pages:	36 - 63

Use the following steps to build the Flash ROM image:

1. Build the kernel (see above)
2. Build the userspace utils (needs to be documented)
3. [In FUZIX/Kernel dir] Create a 256 KiB filesystem image:
	../Standalone/mkfs zeta_v2_rootfs.img 16 512
4. Populate the filesystem image
	../Standalone/ucp zeta_v2_rootfs.img

run the following ucp commands:

mkdir bin
chmod 0755 bin
mkdir etc
chmod 0755 etc
mkdir dev
chmod 0755 dev
mkdir tmp
chmod 01777 tmp
mkdir usr
chmod 0755 usr
mkdir usr/lib
chmod 0755 usr/lib
mkdir root
chmod 0700 root
cd /dev
mknod tty1  20660 513
mknod tty2  20660 514
mknod tty3  20660 515
mknod tty4  20660 516
mknod fd0   60660 0
mknod fd1   60660 1
mknod rd0   60660 256
mknod rd1   60660 257
mknod null  20666 1024
mknod mem   20660 1025
mknod zero  20444 1026
mknod proc  20660 1027
cd /
bget ../Applications/util/init
chmod 0755 init
cd /etc
bget passwd
chmod 0644 passwd
cd /bin
bget ../Applications/util/cat
chmod 0755 cat
bget ../Applications/util/cp
chmod 0755 cp
bget ../Applications/util/date
chmod 0755 date
bget ../Applications/util/df
chmod 0755 df
bget ../Applications/util/echo
chmod 0755 echo
bget ../Applications/util/fdisk
chmod 0755 fdisk
bget ../Applications/util/fsck
chmod 0755 fsck
bget ../Applications/util/ln
chmod 0755 ln
bget ../Applications/util/ls
chmod 0755 ls
bget ../Applications/util/mkdir
chmod 0755 mkdir
bget ../Applications/util/mkfs
chmod 0755 mkfs
bget ../Applications/util/mount
chmod 0755 mount
bget ../Applications/util/mv
chmod 0755 mv
bget ../Applications/util/passwd
chmod 0755 passwd
bget ../Applications/util/rm
chmod 0755 rm
bget ../Applications/util/rmdir
chmod 0755 rmdir
bget ../Applications/util/ssh
chmod 0755 ssh
bget ../Applications/util/umount
chmod 0755 umount
exit

5. Create ROM image:
dd if=fuzix.rom of=fuzix_64k.rom bs=64k count=1 conv=sync
cat fuzix_64k.rom zeta_v2_rootfs.img > fuzix_zeta_v2_bootfs.rom

6. Program the image (fuzix_zeta_v2_bootfs.rom) to the Flash ROM using an
EPROM programmer

7. Power on the the system. At bootdev: prompt enter either rd0 or device number 256 to boot from the RAM disk.
