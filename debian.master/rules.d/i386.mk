human_arch	= 32 bit x86
<<<<<<< HEAD
build_arch	= i386
header_arch	= x86_64
=======
build_arch	= x86
header_arch	= $(build_arch)
>>>>>>> temp
defconfig	= defconfig
flavours        = generic lowlatency
build_image	= bzImage
kernel_file	= arch/$(build_arch)/boot/bzImage
install_file	= vmlinuz
loader		= grub
vdso		= vdso_install
no_dumpfile	= true
do_tools_usbip  = true
do_tools_cpupower = true
do_tools_perf   = true
do_tools_x86	= true
do_tools_hyperv = true
do_extras_package = true
