dnl #
<<<<<<< HEAD
dnl # 2.6.27 API change
dnl # lookup_bdev() was exported.
dnl #
AC_DEFUN([ZFS_AC_KERNEL_LOOKUP_BDEV],
	[AC_MSG_CHECKING([whether lookup_bdev() is available])
=======
dnl # 2.6.27, lookup_bdev() was exported.
dnl # 4.4.0-6.21 - x.y on Ubuntu, lookup_bdev() takes 2 arguments.
dnl #
AC_DEFUN([ZFS_AC_KERNEL_LOOKUP_BDEV],
	[AC_MSG_CHECKING([whether lookup_bdev() wants 1 arg])
>>>>>>> temp
	ZFS_LINUX_TRY_COMPILE_SYMBOL([
		#include <linux/fs.h>
	], [
		lookup_bdev(NULL);
	], [lookup_bdev], [fs/block_dev.c], [
		AC_MSG_RESULT(yes)
<<<<<<< HEAD
		AC_DEFINE(HAVE_LOOKUP_BDEV, 1, [lookup_bdev() is available])
	], [
		AC_MSG_RESULT(no)
		AC_MSG_CHECKING([whether lookup_bdev() is available and wants 2 args])
		ZFS_LINUX_TRY_COMPILE_SYMBOL([
			#include <linux/fs.h>
		], [
			lookup_bdev(NULL, 0);
		], [lookup_bdev], [fs/block_dev.c], [
			AC_MSG_RESULT(yes)
			AC_DEFINE(HAVE_LOOKUP_BDEV_2ARGS, 1, [lookup_bdev() with 2 args is available])
=======
		AC_DEFINE(HAVE_1ARG_LOOKUP_BDEV, 1, [lookup_bdev() wants 1 arg])
	], [
		AC_MSG_RESULT(no)
		AC_MSG_CHECKING([whether lookup_bdev() wants 2 args])
		ZFS_LINUX_TRY_COMPILE_SYMBOL([
			#include <linux/fs.h>
		], [
			lookup_bdev(NULL, FMODE_READ);
		], [lookup_bdev], [fs/block_dev.c], [
			AC_MSG_RESULT(yes)
			AC_DEFINE(HAVE_2ARGS_LOOKUP_BDEV, 1,
			    [lookup_bdev() wants 2 args])
>>>>>>> temp
		], [
			AC_MSG_RESULT(no)
		])
	])
<<<<<<< HEAD
])
=======
])
>>>>>>> temp
