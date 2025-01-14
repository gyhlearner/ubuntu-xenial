/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/*
 * Copyright (C) 2011 Lawrence Livermore National Security, LLC.
 * Copyright (C) 2015 Jörg Thalheim.
 */

#ifndef _ZFS_VFS_H
#define	_ZFS_VFS_H

#include <sys/taskq.h>
<<<<<<< HEAD
=======
#include <sys/cred.h>
>>>>>>> temp
#include <linux/backing-dev.h>

/*
 * 2.6.28 API change,
 * Added insert_inode_locked() helper function, prior to this most callers
 * used insert_inode_hash().  The older method doesn't check for collisions
 * in the inode_hashtable but it still acceptible for use.
 */
#ifndef HAVE_INSERT_INODE_LOCKED
static inline int
insert_inode_locked(struct inode *ip)
{
	insert_inode_hash(ip);
	return (0);
}
#endif /* HAVE_INSERT_INODE_LOCKED */

/*
 * 2.6.35 API change,
 * Add truncate_setsize() if it is not exported by the Linux kernel.
 *
 * Truncate the inode and pages associated with the inode. The pages are
 * unmapped and removed from cache.
 */
#ifndef HAVE_TRUNCATE_SETSIZE
static inline void
truncate_setsize(struct inode *ip, loff_t new)
{
	struct address_space *mapping = ip->i_mapping;

	i_size_write(ip, new);

	unmap_mapping_range(mapping, new + PAGE_SIZE - 1, 0, 1);
	truncate_inode_pages(mapping, new);
	unmap_mapping_range(mapping, new + PAGE_SIZE - 1, 0, 1);
}
#endif /* HAVE_TRUNCATE_SETSIZE */

/*
 * 2.6.32 - 2.6.33, bdi_setup_and_register() is not available.
 * 2.6.34 - 3.19, bdi_setup_and_register() takes 3 arguments.
<<<<<<< HEAD
 * 4.0 - x.y, bdi_setup_and_register() takes 2 arguments.
 */
#if defined(HAVE_2ARGS_BDI_SETUP_AND_REGISTER)
static inline int
zpl_bdi_setup_and_register(struct backing_dev_info *bdi, char *name)
{
	return (bdi_setup_and_register(bdi, name));
}
#elif defined(HAVE_3ARGS_BDI_SETUP_AND_REGISTER)
static inline int
zpl_bdi_setup_and_register(struct backing_dev_info *bdi, char *name)
{
	return (bdi_setup_and_register(bdi, name, BDI_CAP_MAP_COPY));
=======
 * 4.0 - 4.11, bdi_setup_and_register() takes 2 arguments.
 * 4.12 - x.y, super_setup_bdi_name() new interface.
 */
#if defined(HAVE_SUPER_SETUP_BDI_NAME)
extern atomic_long_t zfs_bdi_seq;

static inline int
zpl_bdi_setup(struct super_block *sb, char *name)
{
	return super_setup_bdi_name(sb, "%.28s-%ld", name,
	    atomic_long_inc_return(&zfs_bdi_seq));
}
static inline void
zpl_bdi_destroy(struct super_block *sb)
{
}
#elif defined(HAVE_2ARGS_BDI_SETUP_AND_REGISTER)
static inline int
zpl_bdi_setup(struct super_block *sb, char *name)
{
	struct backing_dev_info *bdi;
	int error;

	bdi = kmem_zalloc(sizeof (struct backing_dev_info), KM_SLEEP);
	error = bdi_setup_and_register(bdi, name);
	if (error) {
		kmem_free(bdi, sizeof (struct backing_dev_info));
		return (error);
	}

	sb->s_bdi = bdi;

	return (0);
}
static inline void
zpl_bdi_destroy(struct super_block *sb)
{
	struct backing_dev_info *bdi = sb->s_bdi;

	bdi_destroy(bdi);
	kmem_free(bdi, sizeof (struct backing_dev_info));
	sb->s_bdi = NULL;
}
#elif defined(HAVE_3ARGS_BDI_SETUP_AND_REGISTER)
static inline int
zpl_bdi_setup(struct super_block *sb, char *name)
{
	struct backing_dev_info *bdi;
	int error;

	bdi = kmem_zalloc(sizeof (struct backing_dev_info), KM_SLEEP);
	error = bdi_setup_and_register(bdi, name, BDI_CAP_MAP_COPY);
	if (error) {
		kmem_free(sb->s_bdi, sizeof (struct backing_dev_info));
		return (error);
	}

	sb->s_bdi = bdi;

	return (0);
}
static inline void
zpl_bdi_destroy(struct super_block *sb)
{
	struct backing_dev_info *bdi = sb->s_bdi;

	bdi_destroy(bdi);
	kmem_free(bdi, sizeof (struct backing_dev_info));
	sb->s_bdi = NULL;
>>>>>>> temp
}
#else
extern atomic_long_t zfs_bdi_seq;

static inline int
<<<<<<< HEAD
zpl_bdi_setup_and_register(struct backing_dev_info *bdi, char *name)
{
	char tmp[32];
	int error;

=======
zpl_bdi_setup(struct super_block *sb, char *name)
{
	struct backing_dev_info *bdi;
	int error;

	bdi = kmem_zalloc(sizeof (struct backing_dev_info), KM_SLEEP);
>>>>>>> temp
	bdi->name = name;
	bdi->capabilities = BDI_CAP_MAP_COPY;

	error = bdi_init(bdi);
<<<<<<< HEAD
	if (error)
		return (error);

	sprintf(tmp, "%.28s%s", name, "-%d");
	error = bdi_register(bdi, NULL, tmp,
	    atomic_long_inc_return(&zfs_bdi_seq));
	if (error) {
		bdi_destroy(bdi);
		return (error);
	}

	return (error);
=======
	if (error) {
		kmem_free(bdi, sizeof (struct backing_dev_info));
		return (error);
	}

	error = bdi_register(bdi, NULL, "%.28s-%ld", name,
	    atomic_long_inc_return(&zfs_bdi_seq));
	if (error) {
		bdi_destroy(bdi);
		kmem_free(bdi, sizeof (struct backing_dev_info));
		return (error);
	}

	sb->s_bdi = bdi;

	return (0);
}
static inline void
zpl_bdi_destroy(struct super_block *sb)
{
	struct backing_dev_info *bdi = sb->s_bdi;

	bdi_destroy(bdi);
	kmem_free(bdi, sizeof (struct backing_dev_info));
	sb->s_bdi = NULL;
>>>>>>> temp
}
#endif

/*
 * 4.14 adds SB_* flag definitions, define them to MS_* equivalents
 * if not set.
 */
<<<<<<< HEAD
#ifndef		SB_RDONLY
#define		SB_RDONLY	MS_RDONLY
#endif

#ifndef		SB_SILENT
#define		SB_SILENT	MS_SILENT
#endif

#ifndef		SB_ACTIVE
#define		SB_ACTIVE	MS_ACTIVE
#endif

#ifndef		SB_POSIXACL
#define		SB_POSIXACL	MS_POSIXACL
#endif

#ifndef		SB_MANDLOCK
#define		SB_MANDLOCK	MS_MANDLOCK
=======
#ifndef	SB_RDONLY
#define	SB_RDONLY	MS_RDONLY
#endif

#ifndef	SB_SILENT
#define	SB_SILENT	MS_SILENT
#endif

#ifndef	SB_ACTIVE
#define	SB_ACTIVE	MS_ACTIVE
#endif

#ifndef	SB_POSIXACL
#define	SB_POSIXACL	MS_POSIXACL
#endif

#ifndef	SB_MANDLOCK
#define	SB_MANDLOCK	MS_MANDLOCK
>>>>>>> temp
#endif

/*
 * 2.6.38 API change,
 * LOOKUP_RCU flag introduced to distinguish rcu-walk from ref-walk cases.
 */
#ifndef LOOKUP_RCU
#define	LOOKUP_RCU	0x0
#endif /* LOOKUP_RCU */

/*
 * 3.2-rc1 API change,
 * Add set_nlink() if it is not exported by the Linux kernel.
 *
 * i_nlink is read-only in Linux 3.2, but it can be set directly in
 * earlier kernels.
 */
#ifndef HAVE_SET_NLINK
static inline void
set_nlink(struct inode *inode, unsigned int nlink)
{
	inode->i_nlink = nlink;
}
#endif /* HAVE_SET_NLINK */

/*
 * 3.3 API change,
 * The VFS .create, .mkdir and .mknod callbacks were updated to take a
 * umode_t type rather than an int.  To cleanly handle both definitions
 * the zpl_umode_t type is introduced and set accordingly.
 */
#ifdef HAVE_MKDIR_UMODE_T
typedef	umode_t		zpl_umode_t;
#else
typedef	int		zpl_umode_t;
#endif

/*
 * 3.5 API change,
 * The clear_inode() function replaces end_writeback() and introduces an
 * ordering change regarding when the inode_sync_wait() occurs.  See the
 * configure check in config/kernel-clear-inode.m4 for full details.
 */
#if defined(HAVE_EVICT_INODE) && !defined(HAVE_CLEAR_INODE)
#define	clear_inode(ip)		end_writeback(ip)
#endif /* HAVE_EVICT_INODE && !HAVE_CLEAR_INODE */

/*
 * 3.6 API change,
 * The sget() helper function now takes the mount flags as an argument.
 */
#ifdef HAVE_5ARG_SGET
#define	zpl_sget(type, cmp, set, fl, mtd)	sget(type, cmp, set, fl, mtd)
#else
#define	zpl_sget(type, cmp, set, fl, mtd)	sget(type, cmp, set, mtd)
#endif /* HAVE_5ARG_SGET */

#if defined(SEEK_HOLE) && defined(SEEK_DATA) && !defined(HAVE_LSEEK_EXECUTE)
static inline loff_t
lseek_execute(
	struct file *filp,
	struct inode *inode,
	loff_t offset,
	loff_t maxsize)
{
	if (offset < 0 && !(filp->f_mode & FMODE_UNSIGNED_OFFSET))
		return (-EINVAL);

	if (offset > maxsize)
		return (-EINVAL);

	if (offset != filp->f_pos) {
		spin_lock(&filp->f_lock);
		filp->f_pos = offset;
		filp->f_version = 0;
		spin_unlock(&filp->f_lock);
	}

	return (offset);
}
#endif /* SEEK_HOLE && SEEK_DATA && !HAVE_LSEEK_EXECUTE */

#if defined(CONFIG_FS_POSIX_ACL)
/*
 * These functions safely approximates the behavior of posix_acl_release()
 * which cannot be used because it calls the GPL-only symbol kfree_rcu().
 * The in-kernel version, which can access the RCU, frees the ACLs after
 * the grace period expires.  Because we're unsure how long that grace
 * period may be this implementation conservatively delays for 60 seconds.
 * This is several orders of magnitude larger than expected grace period.
 * At 60 seconds the kernel will also begin issuing RCU stall warnings.
 */
#include <linux/posix_acl.h>
<<<<<<< HEAD
#ifndef HAVE_POSIX_ACL_CACHING
#define	ACL_NOT_CACHED ((void *)(-1))
#endif /* HAVE_POSIX_ACL_CACHING */

#if defined(HAVE_POSIX_ACL_RELEASE) && !defined(HAVE_POSIX_ACL_RELEASE_GPL_ONLY)

#define	zpl_posix_acl_release(arg)		posix_acl_release(arg)
#define	zpl_set_cached_acl(ip, ty, n)		set_cached_acl(ip, ty, n)
#define	zpl_forget_cached_acl(ip, ty)		forget_cached_acl(ip, ty)

#else

static inline void
zpl_posix_acl_free(void *arg) {
	kfree(arg);
}
=======

#if defined(HAVE_POSIX_ACL_RELEASE) && !defined(HAVE_POSIX_ACL_RELEASE_GPL_ONLY)
#define	zpl_posix_acl_release(arg)		posix_acl_release(arg)
#else
void zpl_posix_acl_release_impl(struct posix_acl *);
>>>>>>> temp

static inline void
zpl_posix_acl_release(struct posix_acl *acl)
{
	if ((acl == NULL) || (acl == ACL_NOT_CACHED))
		return;

<<<<<<< HEAD
	if (atomic_dec_and_test(&acl->a_refcount)) {
		taskq_dispatch_delay(system_taskq, zpl_posix_acl_free, acl,
		    TQ_SLEEP, ddi_get_lbolt() + 60*HZ);
	}
}

static inline void
zpl_set_cached_acl(struct inode *ip, int type, struct posix_acl *newer) {
#ifdef HAVE_POSIX_ACL_CACHING
=======
	if (atomic_dec_and_test(&acl->a_refcount))
		zpl_posix_acl_release_impl(acl);
}
#endif /* HAVE_POSIX_ACL_RELEASE */

#ifdef HAVE_SET_CACHED_ACL_USABLE
#define	zpl_set_cached_acl(ip, ty, n)		set_cached_acl(ip, ty, n)
#define	zpl_forget_cached_acl(ip, ty)		forget_cached_acl(ip, ty)
#else
static inline void
zpl_set_cached_acl(struct inode *ip, int type, struct posix_acl *newer)
{
>>>>>>> temp
	struct posix_acl *older = NULL;

	spin_lock(&ip->i_lock);

	if ((newer != ACL_NOT_CACHED) && (newer != NULL))
		posix_acl_dup(newer);

	switch (type) {
	case ACL_TYPE_ACCESS:
		older = ip->i_acl;
		rcu_assign_pointer(ip->i_acl, newer);
		break;
	case ACL_TYPE_DEFAULT:
		older = ip->i_default_acl;
		rcu_assign_pointer(ip->i_default_acl, newer);
		break;
	}

	spin_unlock(&ip->i_lock);

	zpl_posix_acl_release(older);
<<<<<<< HEAD
#endif /* HAVE_POSIX_ACL_CACHING */
}

static inline void
zpl_forget_cached_acl(struct inode *ip, int type) {
	zpl_set_cached_acl(ip, type, (struct posix_acl *)ACL_NOT_CACHED);
}
#endif /* HAVE_POSIX_ACL_RELEASE */
=======
}

static inline void
zpl_forget_cached_acl(struct inode *ip, int type)
{
	zpl_set_cached_acl(ip, type, (struct posix_acl *)ACL_NOT_CACHED);
}
#endif /* HAVE_SET_CACHED_ACL_USABLE */
>>>>>>> temp

#ifndef HAVE___POSIX_ACL_CHMOD
#ifdef HAVE_POSIX_ACL_CHMOD
#define	__posix_acl_chmod(acl, gfp, mode)	posix_acl_chmod(acl, gfp, mode)
#define	__posix_acl_create(acl, gfp, mode)	posix_acl_create(acl, gfp, mode)
#else
static inline int
<<<<<<< HEAD
__posix_acl_chmod(struct posix_acl **acl, int flags, umode_t umode) {
=======
__posix_acl_chmod(struct posix_acl **acl, int flags, umode_t umode)
{
>>>>>>> temp
	struct posix_acl *oldacl = *acl;
	mode_t mode = umode;
	int error;

	*acl = posix_acl_clone(*acl, flags);
	zpl_posix_acl_release(oldacl);

	if (!(*acl))
		return (-ENOMEM);

	error = posix_acl_chmod_masq(*acl, mode);
	if (error) {
		zpl_posix_acl_release(*acl);
		*acl = NULL;
	}

	return (error);
}

static inline int
<<<<<<< HEAD
__posix_acl_create(struct posix_acl **acl, int flags, umode_t *umodep) {
=======
__posix_acl_create(struct posix_acl **acl, int flags, umode_t *umodep)
{
>>>>>>> temp
	struct posix_acl *oldacl = *acl;
	mode_t mode = *umodep;
	int error;

	*acl = posix_acl_clone(*acl, flags);
	zpl_posix_acl_release(oldacl);

	if (!(*acl))
		return (-ENOMEM);

	error = posix_acl_create_masq(*acl, &mode);
	*umodep = mode;

	if (error < 0) {
		zpl_posix_acl_release(*acl);
		*acl = NULL;
	}

	return (error);
}
#endif /* HAVE_POSIX_ACL_CHMOD */
#endif /* HAVE___POSIX_ACL_CHMOD */

#ifdef HAVE_POSIX_ACL_EQUIV_MODE_UMODE_T
typedef umode_t zpl_equivmode_t;
#else
typedef mode_t zpl_equivmode_t;
#endif /* HAVE_POSIX_ACL_EQUIV_MODE_UMODE_T */

/*
 * 4.8 API change,
 * posix_acl_valid() now must be passed a namespace, the namespace from
 * from super block associated with the given inode is used for this purpose.
 */
#ifdef HAVE_POSIX_ACL_VALID_WITH_NS
#define	zpl_posix_acl_valid(ip, acl)  posix_acl_valid(ip->i_sb->s_user_ns, acl)
#else
#define	zpl_posix_acl_valid(ip, acl)  posix_acl_valid(acl)
#endif

#endif /* CONFIG_FS_POSIX_ACL */

<<<<<<< HEAD
#ifndef HAVE_CURRENT_UMASK
static inline int
current_umask(void)
{
	return (current->fs->umask);
}
#endif /* HAVE_CURRENT_UMASK */

=======
>>>>>>> temp
/*
 * 2.6.38 API change,
 * The is_owner_or_cap() function was renamed to inode_owner_or_capable().
 */
#ifdef HAVE_INODE_OWNER_OR_CAPABLE
#define	zpl_inode_owner_or_capable(ip)		inode_owner_or_capable(ip)
#else
#define	zpl_inode_owner_or_capable(ip)		is_owner_or_cap(ip)
#endif /* HAVE_INODE_OWNER_OR_CAPABLE */

/*
 * 3.19 API change
 * struct access f->f_dentry->d_inode was replaced by accessor function
 * file_inode(f)
 */
#ifndef HAVE_FILE_INODE
static inline struct inode *file_inode(const struct file *f)
{
	return (f->f_dentry->d_inode);
}
#endif /* HAVE_FILE_INODE */

/*
<<<<<<< HEAD
=======
 * 4.1 API change
 * struct access file->f_path.dentry was replaced by accessor function
 * file_dentry(f)
 */
#ifndef HAVE_FILE_DENTRY
static inline struct dentry *file_dentry(const struct file *f)
{
	return (f->f_path.dentry);
}
#endif /* HAVE_FILE_DENTRY */

#ifdef HAVE_KUID_HELPERS
static inline uid_t zfs_uid_read_impl(struct inode *ip)
{
#ifdef HAVE_SUPER_USER_NS
	return (from_kuid(ip->i_sb->s_user_ns, ip->i_uid));
#else
	return (from_kuid(kcred->user_ns, ip->i_uid));
#endif
}

static inline uid_t zfs_uid_read(struct inode *ip)
{
	return (zfs_uid_read_impl(ip));
}

static inline gid_t zfs_gid_read_impl(struct inode *ip)
{
#ifdef HAVE_SUPER_USER_NS
	return (from_kgid(ip->i_sb->s_user_ns, ip->i_gid));
#else
	return (from_kgid(kcred->user_ns, ip->i_gid));
#endif
}

static inline gid_t zfs_gid_read(struct inode *ip)
{
	return (zfs_gid_read_impl(ip));
}

static inline void zfs_uid_write(struct inode *ip, uid_t uid)
{
#ifdef HAVE_SUPER_USER_NS
	ip->i_uid = make_kuid(ip->i_sb->s_user_ns, uid);
#else
	ip->i_uid = make_kuid(kcred->user_ns, uid);
#endif
}

static inline void zfs_gid_write(struct inode *ip, gid_t gid)
{
#ifdef HAVE_SUPER_USER_NS
	ip->i_gid = make_kgid(ip->i_sb->s_user_ns, gid);
#else
	ip->i_gid = make_kgid(kcred->user_ns, gid);
#endif
}

#else
static inline uid_t zfs_uid_read(struct inode *ip)
{
	return (ip->i_uid);
}

static inline gid_t zfs_gid_read(struct inode *ip)
{
	return (ip->i_gid);
}

static inline void zfs_uid_write(struct inode *ip, uid_t uid)
{
	ip->i_uid = uid;
}

static inline void zfs_gid_write(struct inode *ip, gid_t gid)
{
	ip->i_gid = gid;
}
#endif

/*
>>>>>>> temp
 * 2.6.38 API change
 */
#ifdef HAVE_FOLLOW_DOWN_ONE
#define	zpl_follow_down_one(path)		follow_down_one(path)
#define	zpl_follow_up(path)			follow_up(path)
#else
#define	zpl_follow_down_one(path)		follow_down(path)
#define	zpl_follow_up(path)			follow_up(path)
#endif

<<<<<<< HEAD
=======
/*
 * 4.9 API change
 */
#ifndef HAVE_SETATTR_PREPARE
static inline int
setattr_prepare(struct dentry *dentry, struct iattr *ia)
{
	return (inode_change_ok(dentry->d_inode, ia));
}
#endif

/*
 * 4.11 API change
 * These macros are defined by kernel 4.11.  We define them so that the same
 * code builds under kernels < 4.11 and >= 4.11.  The macros are set to 0 so
 * that it will create obvious failures if they are accidentally used when built
 * against a kernel >= 4.11.
 */

#ifndef STATX_BASIC_STATS
#define	STATX_BASIC_STATS	0
#endif

#ifndef AT_STATX_SYNC_AS_STAT
#define	AT_STATX_SYNC_AS_STAT	0
#endif

/*
 * 4.11 API change
 * 4.11 takes struct path *, < 4.11 takes vfsmount *
 */

#ifdef HAVE_VFSMOUNT_IOPS_GETATTR
#define	ZPL_GETATTR_WRAPPER(func)					\
static int								\
func(struct vfsmount *mnt, struct dentry *dentry, struct kstat *stat)	\
{									\
	struct path path = { .mnt = mnt, .dentry = dentry };		\
	return func##_impl(&path, stat, STATX_BASIC_STATS,		\
	    AT_STATX_SYNC_AS_STAT);					\
}
#elif defined(HAVE_PATH_IOPS_GETATTR)
#define	ZPL_GETATTR_WRAPPER(func)					\
static int								\
func(const struct path *path, struct kstat *stat, u32 request_mask,	\
    unsigned int query_flags)						\
{									\
	return (func##_impl(path, stat, request_mask, query_flags));	\
}
#else
#error
#endif

/*
 * 4.9 API change
 * Preferred interface to get the current FS time.
 */
#if !defined(HAVE_CURRENT_TIME)
static inline struct timespec
current_time(struct inode *ip)
{
	return (timespec_trunc(current_kernel_time(), ip->i_sb->s_time_gran));
}
#endif

>>>>>>> temp
#endif /* _ZFS_VFS_H */
