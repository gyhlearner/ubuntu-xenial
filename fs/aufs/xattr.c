/*
<<<<<<< HEAD
 * Copyright (C) 2014-2015 Junjiro R. Okajima
=======
 * Copyright (C) 2014-2017 Junjiro R. Okajima
>>>>>>> temp
 *
 * This program, aufs is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * handling xattr functions
 */

<<<<<<< HEAD
=======
#include <linux/fs.h>
#include <linux/posix_acl_xattr.h>
>>>>>>> temp
#include <linux/xattr.h>
#include "aufs.h"

static int au_xattr_ignore(int err, char *name, unsigned int ignore_flags)
{
	if (!ignore_flags)
		goto out;
	switch (err) {
	case -ENOMEM:
	case -EDQUOT:
		goto out;
	}

	if ((ignore_flags & AuBrAttr_ICEX) == AuBrAttr_ICEX) {
		err = 0;
		goto out;
	}

#define cmp(brattr, prefix) do {					\
		if (!strncmp(name, XATTR_##prefix##_PREFIX,		\
			     XATTR_##prefix##_PREFIX_LEN)) {		\
			if (ignore_flags & AuBrAttr_ICEX_##brattr)	\
				err = 0;				\
			goto out;					\
		}							\
	} while (0)

	cmp(SEC, SECURITY);
	cmp(SYS, SYSTEM);
	cmp(TR, TRUSTED);
	cmp(USR, USER);
#undef cmp

	if (ignore_flags & AuBrAttr_ICEX_OTH)
		err = 0;

out:
	return err;
}

static const int au_xattr_out_of_list = AuBrAttr_ICEX_OTH << 1;

static int au_do_cpup_xattr(struct dentry *h_dst, struct dentry *h_src,
			    char *name, char **buf, unsigned int ignore_flags,
			    unsigned int verbose)
{
	int err;
	ssize_t ssz;
	struct inode *h_idst;

	ssz = vfs_getxattr_alloc(h_src, name, buf, 0, GFP_NOFS);
	err = ssz;
	if (unlikely(err <= 0)) {
		if (err == -ENODATA
		    || (err == -EOPNOTSUPP
			&& ((ignore_flags & au_xattr_out_of_list)
			    || (au_test_nfs_noacl(d_inode(h_src))
				&& (!strcmp(name, XATTR_NAME_POSIX_ACL_ACCESS)
				    || !strcmp(name,
					       XATTR_NAME_POSIX_ACL_DEFAULT))))
			    ))
			err = 0;
		if (err && (verbose || au_debug_test()))
			pr_err("%s, err %d\n", name, err);
		goto out;
	}

	/* unlock it temporary */
	h_idst = d_inode(h_dst);
<<<<<<< HEAD
	mutex_unlock(&h_idst->i_mutex);
	err = vfsub_setxattr(h_dst, name, *buf, ssz, /*flags*/0);
	mutex_lock_nested(&h_idst->i_mutex, AuLsc_I_CHILD2);
=======
	inode_unlock(h_idst);
	err = vfsub_setxattr(h_dst, name, *buf, ssz, /*flags*/0);
	inode_lock_nested(h_idst, AuLsc_I_CHILD2);
>>>>>>> temp
	if (unlikely(err)) {
		if (verbose || au_debug_test())
			pr_err("%s, err %d\n", name, err);
		err = au_xattr_ignore(err, name, ignore_flags);
	}

out:
	return err;
}

int au_cpup_xattr(struct dentry *h_dst, struct dentry *h_src, int ignore_flags,
		  unsigned int verbose)
{
	int err, unlocked, acl_access, acl_default;
	ssize_t ssz;
	struct inode *h_isrc, *h_idst;
	char *value, *p, *o, *e;

	/* try stopping to update the source inode while we are referencing */
	/* there should not be the parent-child relationship between them */
	h_isrc = d_inode(h_src);
	h_idst = d_inode(h_dst);
<<<<<<< HEAD
	mutex_unlock(&h_idst->i_mutex);
	mutex_lock_nested(&h_isrc->i_mutex, AuLsc_I_CHILD);
	mutex_lock_nested(&h_idst->i_mutex, AuLsc_I_CHILD2);
=======
	inode_unlock(h_idst);
	vfsub_inode_lock_shared_nested(h_isrc, AuLsc_I_CHILD);
	inode_lock_nested(h_idst, AuLsc_I_CHILD2);
>>>>>>> temp
	unlocked = 0;

	/* some filesystems don't list POSIX ACL, for example tmpfs */
	ssz = vfs_listxattr(h_src, NULL, 0);
	err = ssz;
	if (unlikely(err < 0)) {
		AuTraceErr(err);
		if (err == -ENODATA
		    || err == -EOPNOTSUPP)
			err = 0;	/* ignore */
		goto out;
	}

	err = 0;
	p = NULL;
	o = NULL;
	if (ssz) {
		err = -ENOMEM;
		p = kmalloc(ssz, GFP_NOFS);
		o = p;
		if (unlikely(!p))
			goto out;
		err = vfs_listxattr(h_src, p, ssz);
	}
<<<<<<< HEAD
	mutex_unlock(&h_isrc->i_mutex);
=======
	inode_unlock_shared(h_isrc);
>>>>>>> temp
	unlocked = 1;
	AuDbg("err %d, ssz %zd\n", err, ssz);
	if (unlikely(err < 0))
		goto out_free;

	err = 0;
	e = p + ssz;
	value = NULL;
	acl_access = 0;
	acl_default = 0;
	while (!err && p < e) {
		acl_access |= !strncmp(p, XATTR_NAME_POSIX_ACL_ACCESS,
				       sizeof(XATTR_NAME_POSIX_ACL_ACCESS) - 1);
		acl_default |= !strncmp(p, XATTR_NAME_POSIX_ACL_DEFAULT,
					sizeof(XATTR_NAME_POSIX_ACL_DEFAULT)
					- 1);
		err = au_do_cpup_xattr(h_dst, h_src, p, &value, ignore_flags,
				       verbose);
		p += strlen(p) + 1;
	}
	AuTraceErr(err);
	ignore_flags |= au_xattr_out_of_list;
	if (!err && !acl_access) {
		err = au_do_cpup_xattr(h_dst, h_src,
				       XATTR_NAME_POSIX_ACL_ACCESS, &value,
				       ignore_flags, verbose);
		AuTraceErr(err);
	}
	if (!err && !acl_default) {
		err = au_do_cpup_xattr(h_dst, h_src,
				       XATTR_NAME_POSIX_ACL_DEFAULT, &value,
				       ignore_flags, verbose);
		AuTraceErr(err);
	}

	kfree(value);

out_free:
	kfree(o);
out:
	if (!unlocked)
<<<<<<< HEAD
		mutex_unlock(&h_isrc->i_mutex);
=======
		inode_unlock_shared(h_isrc);
>>>>>>> temp
	AuTraceErr(err);
	return err;
}

/* ---------------------------------------------------------------------- */

<<<<<<< HEAD
=======
static int au_smack_reentering(struct super_block *sb)
{
#if IS_ENABLED(CONFIG_SECURITY_SMACK)
	/*
	 * as a part of lookup, smack_d_instantiate() is called, and it calls
	 * i_op->getxattr(). ouch.
	 */
	return si_pid_test(sb);
#else
	return 0;
#endif
}

>>>>>>> temp
enum {
	AU_XATTR_LIST,
	AU_XATTR_GET
};

struct au_lgxattr {
	int type;
	union {
		struct {
			char	*list;
			size_t	size;
		} list;
		struct {
			const char	*name;
			void		*value;
			size_t		size;
		} get;
	} u;
};

static ssize_t au_lgxattr(struct dentry *dentry, struct au_lgxattr *arg)
{
	ssize_t err;
<<<<<<< HEAD
=======
	int reenter;
>>>>>>> temp
	struct path h_path;
	struct super_block *sb;

	sb = dentry->d_sb;
<<<<<<< HEAD
	err = si_read_lock(sb, AuLock_FLUSH | AuLock_NOPLM);
	if (unlikely(err))
		goto out;
	err = au_h_path_getattr(dentry, /*force*/1, &h_path);
=======
	reenter = au_smack_reentering(sb);
	if (!reenter) {
		err = si_read_lock(sb, AuLock_FLUSH | AuLock_NOPLM);
		if (unlikely(err))
			goto out;
	}
	err = au_h_path_getattr(dentry, /*force*/1, &h_path, reenter);
>>>>>>> temp
	if (unlikely(err))
		goto out_si;
	if (unlikely(!h_path.dentry))
		/* illegally overlapped or something */
		goto out_di; /* pretending success */

	/* always topmost entry only */
	switch (arg->type) {
	case AU_XATTR_LIST:
		err = vfs_listxattr(h_path.dentry,
				    arg->u.list.list, arg->u.list.size);
		break;
	case AU_XATTR_GET:
<<<<<<< HEAD
=======
		AuDebugOn(d_is_negative(h_path.dentry));
>>>>>>> temp
		err = vfs_getxattr(h_path.dentry,
				   arg->u.get.name, arg->u.get.value,
				   arg->u.get.size);
		break;
	}

out_di:
<<<<<<< HEAD
	di_read_unlock(dentry, AuLock_IR);
out_si:
	si_read_unlock(sb);
=======
	if (!reenter)
		di_read_unlock(dentry, AuLock_IR);
out_si:
	if (!reenter)
		si_read_unlock(sb);
>>>>>>> temp
out:
	AuTraceErr(err);
	return err;
}

ssize_t aufs_listxattr(struct dentry *dentry, char *list, size_t size)
{
	struct au_lgxattr arg = {
		.type = AU_XATTR_LIST,
		.u.list = {
			.list	= list,
			.size	= size
		},
	};

	return au_lgxattr(dentry, &arg);
}

<<<<<<< HEAD
ssize_t aufs_getxattr(struct dentry *dentry, const char *name, void *value,
		      size_t size)
=======
static ssize_t au_getxattr(struct dentry *dentry,
			   struct inode *inode __maybe_unused,
			   const char *name, void *value, size_t size)
>>>>>>> temp
{
	struct au_lgxattr arg = {
		.type = AU_XATTR_GET,
		.u.get = {
			.name	= name,
			.value	= value,
			.size	= size
		},
	};

	return au_lgxattr(dentry, &arg);
}

<<<<<<< HEAD
int aufs_setxattr(struct dentry *dentry, const char *name, const void *value,
		  size_t size, int flags)
{
	struct au_srxattr arg = {
=======
static int au_setxattr(struct dentry *dentry, struct inode *inode,
		       const char *name, const void *value, size_t size,
		       int flags)
{
	struct au_sxattr arg = {
>>>>>>> temp
		.type = AU_XATTR_SET,
		.u.set = {
			.name	= name,
			.value	= value,
			.size	= size,
			.flags	= flags
		},
	};

<<<<<<< HEAD
	return au_srxattr(dentry, &arg);
}

int aufs_removexattr(struct dentry *dentry, const char *name)
{
	struct au_srxattr arg = {
		.type = AU_XATTR_REMOVE,
		.u.remove = {
			.name	= name
		},
	};

	return au_srxattr(dentry, &arg);
=======
	return au_sxattr(dentry, inode, &arg);
>>>>>>> temp
}

/* ---------------------------------------------------------------------- */

<<<<<<< HEAD
#if 0
static size_t au_xattr_list(struct dentry *dentry, char *list, size_t list_size,
			    const char *name, size_t name_len, int type)
{
	return aufs_listxattr(dentry, list, list_size);
}

static int au_xattr_get(struct dentry *dentry, const char *name, void *buffer,
			size_t size, int type)
{
	return aufs_getxattr(dentry, name, buffer, size);
}

static int au_xattr_set(struct dentry *dentry, const char *name,
			const void *value, size_t size, int flags, int type)
{
	return aufs_setxattr(dentry, name, value, size, flags);
}

static const struct xattr_handler au_xattr_handler = {
	/* no prefix, no flags */
	.list	= au_xattr_list,
	.get	= au_xattr_get,
	.set	= au_xattr_set
	/* why no remove? */
};

static const struct xattr_handler *au_xattr_handlers[] = {
	&au_xattr_handler
=======
static int au_xattr_get(const struct xattr_handler *handler,
			struct dentry *dentry, struct inode *inode,
			const char *name, void *buffer, size_t size)
{
	return au_getxattr(dentry, inode, name, buffer, size);
}

static int au_xattr_set(const struct xattr_handler *handler,
			struct dentry *dentry, struct inode *inode,
			const char *name, const void *value, size_t size,
			int flags)
{
	return au_setxattr(dentry, inode, name, value, size, flags);
}

static const struct xattr_handler au_xattr_handler = {
	.name	= "",
	.prefix	= "",
	.get	= au_xattr_get,
	.set	= au_xattr_set
};

static const struct xattr_handler *au_xattr_handlers[] = {
#ifdef CONFIG_FS_POSIX_ACL
	&posix_acl_access_xattr_handler,
	&posix_acl_default_xattr_handler,
#endif
	&au_xattr_handler, /* must be last */
	NULL
>>>>>>> temp
};

void au_xattr_init(struct super_block *sb)
{
<<<<<<< HEAD
	/* sb->s_xattr = au_xattr_handlers; */
}
#endif
=======
	sb->s_xattr = au_xattr_handlers;
}
>>>>>>> temp
