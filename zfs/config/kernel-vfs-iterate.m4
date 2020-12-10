<<<<<<< HEAD
dnl #
dnl # 3.11 API change
dnl #
AC_DEFUN([ZFS_AC_KERNEL_VFS_ITERATE], [
	AC_MSG_CHECKING([whether fops->iterate() is available])
=======
AC_DEFUN([ZFS_AC_KERNEL_VFS_ITERATE], [
	dnl #
	dnl # 4.7 API change
	dnl #
	AC_MSG_CHECKING([whether fops->iterate_shared() is available])
>>>>>>> temp
	ZFS_LINUX_TRY_COMPILE([
		#include <linux/fs.h>
		int iterate(struct file *filp, struct dir_context * context)
		    { return 0; }

		static const struct file_operations fops
		    __attribute__ ((unused)) = {
<<<<<<< HEAD
			.iterate	 = iterate,
=======
			.iterate_shared	 = iterate,
>>>>>>> temp
		};
	],[
	],[
		AC_MSG_RESULT(yes)
<<<<<<< HEAD
		AC_DEFINE(HAVE_VFS_ITERATE, 1,
		          [fops->iterate() is available])
	],[
		AC_MSG_RESULT(no)

		AC_MSG_CHECKING([whether fops->readdir() is available])
		ZFS_LINUX_TRY_COMPILE([
			#include <linux/fs.h>
			int readdir(struct file *filp, void *entry, filldir_t func)
=======
		AC_DEFINE(HAVE_VFS_ITERATE_SHARED, 1,
		          [fops->iterate_shared() is available])
	],[
		AC_MSG_RESULT(no)

		dnl #
		dnl # 3.11 API change
		dnl #
		AC_MSG_CHECKING([whether fops->iterate() is available])
		ZFS_LINUX_TRY_COMPILE([
			#include <linux/fs.h>
			int iterate(struct file *filp, struct dir_context * context)
>>>>>>> temp
			    { return 0; }

			static const struct file_operations fops
			    __attribute__ ((unused)) = {
<<<<<<< HEAD
				.readdir = readdir,
=======
				.iterate	 = iterate,
>>>>>>> temp
			};
		],[
		],[
			AC_MSG_RESULT(yes)
<<<<<<< HEAD
			AC_DEFINE(HAVE_VFS_READDIR, 1,
				  [fops->readdir() is available])
		],[
			AC_MSG_ERROR(no; file a bug report with ZFSOnLinux)
		])

=======
			AC_DEFINE(HAVE_VFS_ITERATE, 1,
				  [fops->iterate() is available])
		],[
			AC_MSG_RESULT(no)

			AC_MSG_CHECKING([whether fops->readdir() is available])
			ZFS_LINUX_TRY_COMPILE([
				#include <linux/fs.h>
				int readdir(struct file *filp, void *entry, filldir_t func)
				    { return 0; }

				static const struct file_operations fops
				    __attribute__ ((unused)) = {
					.readdir = readdir,
				};
			],[
			],[
				AC_MSG_RESULT(yes)
				AC_DEFINE(HAVE_VFS_READDIR, 1,
					  [fops->readdir() is available])
			],[
				AC_MSG_ERROR(no; file a bug report with ZFSOnLinux)
			])
		])
>>>>>>> temp
	])
])
