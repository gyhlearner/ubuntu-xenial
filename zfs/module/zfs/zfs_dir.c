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
<<<<<<< HEAD
/*
 * Copyright (c) 2005, 2010, Oracle and/or its affiliates. All rights reserved.
 * Copyright (c) 2013 by Delphix. All rights reserved.
 */


=======

/*
 * Copyright (c) 2005, 2010, Oracle and/or its affiliates. All rights reserved.
 * Copyright (c) 2013, 2016 by Delphix. All rights reserved.
 * Copyright 2017 Nexenta Systems, Inc.
 */

>>>>>>> temp
#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/systm.h>
#include <sys/sysmacros.h>
#include <sys/resource.h>
#include <sys/vfs.h>
#include <sys/vnode.h>
#include <sys/file.h>
#include <sys/mode.h>
#include <sys/kmem.h>
#include <sys/uio.h>
#include <sys/pathname.h>
#include <sys/cmn_err.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <sys/sunddi.h>
#include <sys/random.h>
#include <sys/policy.h>
#include <sys/zfs_dir.h>
#include <sys/zfs_acl.h>
#include <sys/zfs_vnops.h>
#include <sys/fs/zfs.h>
#include "fs/fs_subr.h"
#include <sys/zap.h>
#include <sys/dmu.h>
#include <sys/atomic.h>
#include <sys/zfs_ctldir.h>
#include <sys/zfs_fuid.h>
#include <sys/sa.h>
#include <sys/zfs_sa.h>
#include <sys/dnlc.h>
#include <sys/extdirent.h>

/*
 * zfs_match_find() is used by zfs_dirent_lock() to peform zap lookups
 * of names after deciding which is the appropriate lookup interface.
 */
static int
<<<<<<< HEAD
zfs_match_find(zfs_sb_t *zsb, znode_t *dzp, char *name, boolean_t exact,
=======
zfs_match_find(zfsvfs_t *zfsvfs, znode_t *dzp, char *name, matchtype_t mt,
>>>>>>> temp
    boolean_t update, int *deflags, pathname_t *rpnp, uint64_t *zoid)
{
	boolean_t conflict = B_FALSE;
	int error;

<<<<<<< HEAD
	if (zsb->z_norm) {
		matchtype_t mt = MT_FIRST;
=======
	if (zfsvfs->z_norm) {
>>>>>>> temp
		size_t bufsz = 0;
		char *buf = NULL;

		if (rpnp) {
			buf = rpnp->pn_buf;
			bufsz = rpnp->pn_bufsize;
		}
<<<<<<< HEAD
		if (exact)
			mt = MT_EXACT;
=======

>>>>>>> temp
		/*
		 * In the non-mixed case we only expect there would ever
		 * be one match, but we need to use the normalizing lookup.
		 */
<<<<<<< HEAD
		error = zap_lookup_norm(zsb->z_os, dzp->z_id, name, 8, 1,
		    zoid, mt, buf, bufsz, &conflict);
	} else {
		error = zap_lookup(zsb->z_os, dzp->z_id, name, 8, 1, zoid);
=======
		error = zap_lookup_norm(zfsvfs->z_os, dzp->z_id, name, 8, 1,
		    zoid, mt, buf, bufsz, &conflict);
	} else {
		error = zap_lookup(zfsvfs->z_os, dzp->z_id, name, 8, 1, zoid);
>>>>>>> temp
	}

	/*
	 * Allow multiple entries provided the first entry is
	 * the object id.  Non-zpl consumers may safely make
	 * use of the additional space.
	 *
	 * XXX: This should be a feature flag for compatibility
	 */
	if (error == EOVERFLOW)
		error = 0;

<<<<<<< HEAD
	if (zsb->z_norm && !error && deflags)
=======
	if (zfsvfs->z_norm && !error && deflags)
>>>>>>> temp
		*deflags = conflict ? ED_CASE_CONFLICT : 0;

	*zoid = ZFS_DIRENT_OBJ(*zoid);

#ifdef HAVE_DNLC
	if (error == ENOENT && update)
		dnlc_update(ZTOI(dzp), name, DNLC_NO_VNODE);
#endif /* HAVE_DNLC */

	return (error);
}

/*
 * Lock a directory entry.  A dirlock on <dzp, name> protects that name
 * in dzp's directory zap object.  As long as you hold a dirlock, you can
 * assume two things: (1) dzp cannot be reaped, and (2) no other thread
 * can change the zap entry for (i.e. link or unlink) this name.
 *
 * Input arguments:
 *	dzp	- znode for directory
 *	name	- name of entry to lock
 *	flag	- ZNEW: if the entry already exists, fail with EEXIST.
 *		  ZEXISTS: if the entry does not exist, fail with ENOENT.
 *		  ZSHARED: allow concurrent access with other ZSHARED callers.
 *		  ZXATTR: we want dzp's xattr directory
 *		  ZCILOOK: On a mixed sensitivity file system,
 *			   this lookup should be case-insensitive.
 *		  ZCIEXACT: On a purely case-insensitive file system,
 *			    this lookup should be case-sensitive.
 *		  ZRENAMING: we are locking for renaming, force narrow locks
 *		  ZHAVELOCK: Don't grab the z_name_lock for this call. The
 *			     current thread already holds it.
 *
 * Output arguments:
 *	zpp	- pointer to the znode for the entry (NULL if there isn't one)
 *	dlpp	- pointer to the dirlock for this entry (NULL on error)
 *      direntflags - (case-insensitive lookup only)
 *		flags if multiple case-sensitive matches exist in directory
 *      realpnp     - (case-insensitive lookup only)
 *		actual name matched within the directory
 *
 * Return value: 0 on success or errno on failure.
 *
 * NOTE: Always checks for, and rejects, '.' and '..'.
 * NOTE: For case-insensitive file systems we take wide locks (see below),
 *	 but return znode pointers to a single match.
 */
int
zfs_dirent_lock(zfs_dirlock_t **dlpp, znode_t *dzp, char *name, znode_t **zpp,
    int flag, int *direntflags, pathname_t *realpnp)
{
<<<<<<< HEAD
	zfs_sb_t	*zsb = ZTOZSB(dzp);
	zfs_dirlock_t	*dl;
	boolean_t	update;
	boolean_t	exact;
=======
	zfsvfs_t	*zfsvfs = ZTOZSB(dzp);
	zfs_dirlock_t	*dl;
	boolean_t	update;
	matchtype_t	mt = 0;
>>>>>>> temp
	uint64_t	zoid;
#ifdef HAVE_DNLC
	vnode_t		*vp = NULL;
#endif /* HAVE_DNLC */
	int		error = 0;
	int		cmpflags;

	*zpp = NULL;
	*dlpp = NULL;

	/*
	 * Verify that we are not trying to lock '.', '..', or '.zfs'
	 */
	if ((name[0] == '.' &&
	    (name[1] == '\0' || (name[1] == '.' && name[2] == '\0'))) ||
	    (zfs_has_ctldir(dzp) && strcmp(name, ZFS_CTLDIR_NAME) == 0))
		return (SET_ERROR(EEXIST));

	/*
	 * Case sensitivity and normalization preferences are set when
	 * the file system is created.  These are stored in the
<<<<<<< HEAD
	 * zsb->z_case and zsb->z_norm fields.  These choices
=======
	 * zfsvfs->z_case and zfsvfs->z_norm fields.  These choices
>>>>>>> temp
	 * affect what vnodes can be cached in the DNLC, how we
	 * perform zap lookups, and the "width" of our dirlocks.
	 *
	 * A normal dirlock locks a single name.  Note that with
	 * normalization a name can be composed multiple ways, but
	 * when normalized, these names all compare equal.  A wide
	 * dirlock locks multiple names.  We need these when the file
	 * system is supporting mixed-mode access.  It is sometimes
	 * necessary to lock all case permutations of file name at
	 * once so that simultaneous case-insensitive/case-sensitive
	 * behaves as rationally as possible.
	 */

	/*
<<<<<<< HEAD
	 * Decide if exact matches should be requested when performing
	 * a zap lookup on file systems supporting case-insensitive
	 * access.
	 */
	exact =
	    ((zsb->z_case == ZFS_CASE_INSENSITIVE) && (flag & ZCIEXACT)) ||
	    ((zsb->z_case == ZFS_CASE_MIXED) && !(flag & ZCILOOK));
=======
	 * When matching we may need to normalize & change case according to
	 * FS settings.
	 *
	 * Note that a normalized match is necessary for a case insensitive
	 * filesystem when the lookup request is not exact because normalization
	 * can fold case independent of normalizing code point sequences.
	 *
	 * See the table above zfs_dropname().
	 */
	if (zfsvfs->z_norm != 0) {
		mt = MT_NORMALIZE;

		/*
		 * Determine if the match needs to honor the case specified in
		 * lookup, and if so keep track of that so that during
		 * normalization we don't fold case.
		 */
		if ((zfsvfs->z_case == ZFS_CASE_INSENSITIVE &&
		    (flag & ZCIEXACT)) ||
		    (zfsvfs->z_case == ZFS_CASE_MIXED && !(flag & ZCILOOK))) {
			mt |= MT_MATCH_CASE;
		}
	}
>>>>>>> temp

	/*
	 * Only look in or update the DNLC if we are looking for the
	 * name on a file system that does not require normalization
	 * or case folding.  We can also look there if we happen to be
	 * on a non-normalizing, mixed sensitivity file system IF we
	 * are looking for the exact name.
	 *
	 * Maybe can add TO-UPPERed version of name to dnlc in ci-only
	 * case for performance improvement?
	 */
<<<<<<< HEAD
	update = !zsb->z_norm ||
	    ((zsb->z_case == ZFS_CASE_MIXED) &&
	    !(zsb->z_norm & ~U8_TEXTPREP_TOUPPER) && !(flag & ZCILOOK));
=======
	update = !zfsvfs->z_norm ||
	    (zfsvfs->z_case == ZFS_CASE_MIXED &&
	    !(zfsvfs->z_norm & ~U8_TEXTPREP_TOUPPER) && !(flag & ZCILOOK));
>>>>>>> temp

	/*
	 * ZRENAMING indicates we are in a situation where we should
	 * take narrow locks regardless of the file system's
	 * preferences for normalizing and case folding.  This will
	 * prevent us deadlocking trying to grab the same wide lock
	 * twice if the two names happen to be case-insensitive
	 * matches.
	 */
	if (flag & ZRENAMING)
		cmpflags = 0;
	else
<<<<<<< HEAD
		cmpflags = zsb->z_norm;
=======
		cmpflags = zfsvfs->z_norm;
>>>>>>> temp

	/*
	 * Wait until there are no locks on this name.
	 *
	 * Don't grab the the lock if it is already held. However, cannot
	 * have both ZSHARED and ZHAVELOCK together.
	 */
	ASSERT(!(flag & ZSHARED) || !(flag & ZHAVELOCK));
	if (!(flag & ZHAVELOCK))
		rw_enter(&dzp->z_name_lock, RW_READER);

	mutex_enter(&dzp->z_lock);
	for (;;) {
<<<<<<< HEAD
		if (dzp->z_unlinked) {
=======
		if (dzp->z_unlinked && !(flag & ZXATTR)) {
>>>>>>> temp
			mutex_exit(&dzp->z_lock);
			if (!(flag & ZHAVELOCK))
				rw_exit(&dzp->z_name_lock);
			return (SET_ERROR(ENOENT));
		}
		for (dl = dzp->z_dirlocks; dl != NULL; dl = dl->dl_next) {
			if ((u8_strcmp(name, dl->dl_name, 0, cmpflags,
			    U8_UNICODE_LATEST, &error) == 0) || error != 0)
				break;
		}
		if (error != 0) {
			mutex_exit(&dzp->z_lock);
			if (!(flag & ZHAVELOCK))
				rw_exit(&dzp->z_name_lock);
			return (SET_ERROR(ENOENT));
		}
		if (dl == NULL)	{
			/*
			 * Allocate a new dirlock and add it to the list.
			 */
			dl = kmem_alloc(sizeof (zfs_dirlock_t), KM_SLEEP);
			cv_init(&dl->dl_cv, NULL, CV_DEFAULT, NULL);
			dl->dl_name = name;
			dl->dl_sharecnt = 0;
			dl->dl_namelock = 0;
			dl->dl_namesize = 0;
			dl->dl_dzp = dzp;
			dl->dl_next = dzp->z_dirlocks;
			dzp->z_dirlocks = dl;
			break;
		}
		if ((flag & ZSHARED) && dl->dl_sharecnt != 0)
			break;
		cv_wait(&dl->dl_cv, &dzp->z_lock);
	}

	/*
	 * If the z_name_lock was NOT held for this dirlock record it.
	 */
	if (flag & ZHAVELOCK)
		dl->dl_namelock = 1;

	if ((flag & ZSHARED) && ++dl->dl_sharecnt > 1 && dl->dl_namesize == 0) {
		/*
		 * We're the second shared reference to dl.  Make a copy of
		 * dl_name in case the first thread goes away before we do.
		 * Note that we initialize the new name before storing its
		 * pointer into dl_name, because the first thread may load
<<<<<<< HEAD
		 * dl->dl_name at any time.  He'll either see the old value,
		 * which is his, or the new shared copy; either is OK.
=======
		 * dl->dl_name at any time.  It'll either see the old value,
		 * which belongs to it, or the new shared copy; either is OK.
>>>>>>> temp
		 */
		dl->dl_namesize = strlen(dl->dl_name) + 1;
		name = kmem_alloc(dl->dl_namesize, KM_SLEEP);
		bcopy(dl->dl_name, name, dl->dl_namesize);
		dl->dl_name = name;
	}

	mutex_exit(&dzp->z_lock);

	/*
	 * We have a dirlock on the name.  (Note that it is the dirlock,
	 * not the dzp's z_lock, that protects the name in the zap object.)
	 * See if there's an object by this name; if so, put a hold on it.
	 */
	if (flag & ZXATTR) {
<<<<<<< HEAD
		error = sa_lookup(dzp->z_sa_hdl, SA_ZPL_XATTR(zsb), &zoid,
=======
		error = sa_lookup(dzp->z_sa_hdl, SA_ZPL_XATTR(zfsvfs), &zoid,
>>>>>>> temp
		    sizeof (zoid));
		if (error == 0)
			error = (zoid == 0 ? SET_ERROR(ENOENT) : 0);
	} else {
#ifdef HAVE_DNLC
		if (update)
			vp = dnlc_lookup(ZTOI(dzp), name);
		if (vp == DNLC_NO_VNODE) {
			iput(vp);
			error = SET_ERROR(ENOENT);
		} else if (vp) {
			if (flag & ZNEW) {
				zfs_dirent_unlock(dl);
				iput(vp);
				return (SET_ERROR(EEXIST));
			}
			*dlpp = dl;
			*zpp = VTOZ(vp);
			return (0);
		} else {
<<<<<<< HEAD
			error = zfs_match_find(zsb, dzp, name, exact,
			    update, direntflags, realpnp, &zoid);
		}
#else
		error = zfs_match_find(zsb, dzp, name, exact,
=======
			error = zfs_match_find(zfsvfs, dzp, name, mt,
			    update, direntflags, realpnp, &zoid);
		}
#else
		error = zfs_match_find(zfsvfs, dzp, name, mt,
>>>>>>> temp
		    update, direntflags, realpnp, &zoid);
#endif /* HAVE_DNLC */
	}
	if (error) {
		if (error != ENOENT || (flag & ZEXISTS)) {
			zfs_dirent_unlock(dl);
			return (error);
		}
	} else {
		if (flag & ZNEW) {
			zfs_dirent_unlock(dl);
			return (SET_ERROR(EEXIST));
		}
<<<<<<< HEAD
		error = zfs_zget(zsb, zoid, zpp);
=======
		error = zfs_zget(zfsvfs, zoid, zpp);
>>>>>>> temp
		if (error) {
			zfs_dirent_unlock(dl);
			return (error);
		}
#ifdef HAVE_DNLC
		if (!(flag & ZXATTR) && update)
			dnlc_update(ZTOI(dzp), name, ZTOI(*zpp));
#endif /* HAVE_DNLC */
	}

	*dlpp = dl;

	return (0);
}

/*
 * Unlock this directory entry and wake anyone who was waiting for it.
 */
void
zfs_dirent_unlock(zfs_dirlock_t *dl)
{
	znode_t *dzp = dl->dl_dzp;
	zfs_dirlock_t **prev_dl, *cur_dl;

	mutex_enter(&dzp->z_lock);

	if (!dl->dl_namelock)
		rw_exit(&dzp->z_name_lock);

	if (dl->dl_sharecnt > 1) {
		dl->dl_sharecnt--;
		mutex_exit(&dzp->z_lock);
		return;
	}
	prev_dl = &dzp->z_dirlocks;
	while ((cur_dl = *prev_dl) != dl)
		prev_dl = &cur_dl->dl_next;
	*prev_dl = dl->dl_next;
	cv_broadcast(&dl->dl_cv);
	mutex_exit(&dzp->z_lock);

	if (dl->dl_namesize != 0)
		kmem_free(dl->dl_name, dl->dl_namesize);
	cv_destroy(&dl->dl_cv);
	kmem_free(dl, sizeof (*dl));
}

/*
 * Look up an entry in a directory.
 *
 * NOTE: '.' and '..' are handled as special cases because
 *	no directory entries are actually stored for them.  If this is
 *	the root of a filesystem, then '.zfs' is also treated as a
 *	special pseudo-directory.
 */
int
zfs_dirlook(znode_t *dzp, char *name, struct inode **ipp, int flags,
    int *deflg, pathname_t *rpnp)
{
	zfs_dirlock_t *dl;
	znode_t *zp;
	int error = 0;
	uint64_t parent;

	if (name[0] == 0 || (name[0] == '.' && name[1] == 0)) {
		*ipp = ZTOI(dzp);
		igrab(*ipp);
	} else if (name[0] == '.' && name[1] == '.' && name[2] == 0) {
<<<<<<< HEAD
		zfs_sb_t *zsb = ZTOZSB(dzp);
=======
		zfsvfs_t *zfsvfs = ZTOZSB(dzp);
>>>>>>> temp

		/*
		 * If we are a snapshot mounted under .zfs, return
		 * the inode pointer for the snapshot directory.
		 */
		if ((error = sa_lookup(dzp->z_sa_hdl,
<<<<<<< HEAD
		    SA_ZPL_PARENT(zsb), &parent, sizeof (parent))) != 0)
			return (error);

		if (parent == dzp->z_id && zsb->z_parent != zsb) {
			error = zfsctl_root_lookup(zsb->z_parent->z_ctldir,
=======
		    SA_ZPL_PARENT(zfsvfs), &parent, sizeof (parent))) != 0)
			return (error);

		if (parent == dzp->z_id && zfsvfs->z_parent != zfsvfs) {
			error = zfsctl_root_lookup(zfsvfs->z_parent->z_ctldir,
>>>>>>> temp
			    "snapshot", ipp, 0, kcred, NULL, NULL);
			return (error);
		}
		rw_enter(&dzp->z_parent_lock, RW_READER);
<<<<<<< HEAD
		error = zfs_zget(zsb, parent, &zp);
=======
		error = zfs_zget(zfsvfs, parent, &zp);
>>>>>>> temp
		if (error == 0)
			*ipp = ZTOI(zp);
		rw_exit(&dzp->z_parent_lock);
	} else if (zfs_has_ctldir(dzp) && strcmp(name, ZFS_CTLDIR_NAME) == 0) {
		*ipp = zfsctl_root(dzp);
	} else {
		int zf;

		zf = ZEXISTS | ZSHARED;
		if (flags & FIGNORECASE)
			zf |= ZCILOOK;

		error = zfs_dirent_lock(&dl, dzp, name, &zp, zf, deflg, rpnp);
		if (error == 0) {
			*ipp = ZTOI(zp);
			zfs_dirent_unlock(dl);
			dzp->z_zn_prefetch = B_TRUE; /* enable prefetching */
		}
		rpnp = NULL;
	}

	if ((flags & FIGNORECASE) && rpnp && !error)
		(void) strlcpy(rpnp->pn_buf, name, rpnp->pn_bufsize);

	return (error);
}

/*
 * unlinked Set (formerly known as the "delete queue") Error Handling
 *
 * When dealing with the unlinked set, we dmu_tx_hold_zap(), but we
 * don't specify the name of the entry that we will be manipulating.  We
 * also fib and say that we won't be adding any new entries to the
 * unlinked set, even though we might (this is to lower the minimum file
 * size that can be deleted in a full filesystem).  So on the small
 * chance that the nlink list is using a fat zap (ie. has more than
 * 2000 entries), we *may* not pre-read a block that's needed.
 * Therefore it is remotely possible for some of the assertions
 * regarding the unlinked set below to fail due to i/o error.  On a
 * nondebug system, this will result in the space being leaked.
 */
void
zfs_unlinked_add(znode_t *zp, dmu_tx_t *tx)
{
<<<<<<< HEAD
	zfs_sb_t *zsb = ZTOZSB(zp);

	ASSERT(zp->z_unlinked);
	ASSERT(zp->z_links == 0);

	VERIFY3U(0, ==,
	    zap_add_int(zsb->z_os, zsb->z_unlinkedobj, zp->z_id, tx));
=======
	zfsvfs_t *zfsvfs = ZTOZSB(zp);

	ASSERT(zp->z_unlinked);
	ASSERT(ZTOI(zp)->i_nlink == 0);

	VERIFY3U(0, ==,
	    zap_add_int(zfsvfs->z_os, zfsvfs->z_unlinkedobj, zp->z_id, tx));
>>>>>>> temp
}

/*
 * Clean up any znodes that had no links when we either crashed or
 * (force) umounted the file system.
 */
void
<<<<<<< HEAD
zfs_unlinked_drain(zfs_sb_t *zsb)
=======
zfs_unlinked_drain(zfsvfs_t *zfsvfs)
>>>>>>> temp
{
	zap_cursor_t	zc;
	zap_attribute_t zap;
	dmu_object_info_t doi;
	znode_t		*zp;
	int		error;

	/*
	 * Iterate over the contents of the unlinked set.
	 */
<<<<<<< HEAD
	for (zap_cursor_init(&zc, zsb->z_os, zsb->z_unlinkedobj);
=======
	for (zap_cursor_init(&zc, zfsvfs->z_os, zfsvfs->z_unlinkedobj);
>>>>>>> temp
	    zap_cursor_retrieve(&zc, &zap) == 0;
	    zap_cursor_advance(&zc)) {

		/*
		 * See what kind of object we have in list
		 */

<<<<<<< HEAD
		error = dmu_object_info(zsb->z_os, zap.za_first_integer, &doi);
=======
		error = dmu_object_info(zfsvfs->z_os,
		    zap.za_first_integer, &doi);
>>>>>>> temp
		if (error != 0)
			continue;

		ASSERT((doi.doi_type == DMU_OT_PLAIN_FILE_CONTENTS) ||
		    (doi.doi_type == DMU_OT_DIRECTORY_CONTENTS));
		/*
		 * We need to re-mark these list entries for deletion,
		 * so we pull them back into core and set zp->z_unlinked.
		 */
<<<<<<< HEAD
		error = zfs_zget(zsb, zap.za_first_integer, &zp);
=======
		error = zfs_zget(zfsvfs, zap.za_first_integer, &zp);
>>>>>>> temp

		/*
		 * We may pick up znodes that are already marked for deletion.
		 * This could happen during the purge of an extended attribute
		 * directory.  All we need to do is skip over them, since they
		 * are already in the system marked z_unlinked.
		 */
		if (error != 0)
			continue;

		zp->z_unlinked = B_TRUE;
		iput(ZTOI(zp));
	}
	zap_cursor_fini(&zc);
}

/*
 * Delete the entire contents of a directory.  Return a count
 * of the number of entries that could not be deleted. If we encounter
 * an error, return a count of at least one so that the directory stays
 * in the unlinked set.
 *
 * NOTE: this function assumes that the directory is inactive,
 *	so there is no need to lock its entries before deletion.
 *	Also, it assumes the directory contents is *only* regular
 *	files.
 */
static int
zfs_purgedir(znode_t *dzp)
{
	zap_cursor_t	zc;
	zap_attribute_t	zap;
	znode_t		*xzp;
	dmu_tx_t	*tx;
<<<<<<< HEAD
	zfs_sb_t	*zsb = ZTOZSB(dzp);
=======
	zfsvfs_t	*zfsvfs = ZTOZSB(dzp);
>>>>>>> temp
	zfs_dirlock_t	dl;
	int skipped = 0;
	int error;

<<<<<<< HEAD
	for (zap_cursor_init(&zc, zsb->z_os, dzp->z_id);
	    (error = zap_cursor_retrieve(&zc, &zap)) == 0;
	    zap_cursor_advance(&zc)) {
		error = zfs_zget(zsb,
=======
	for (zap_cursor_init(&zc, zfsvfs->z_os, dzp->z_id);
	    (error = zap_cursor_retrieve(&zc, &zap)) == 0;
	    zap_cursor_advance(&zc)) {
		error = zfs_zget(zfsvfs,
>>>>>>> temp
		    ZFS_DIRENT_OBJ(zap.za_first_integer), &xzp);
		if (error) {
			skipped += 1;
			continue;
		}

		ASSERT(S_ISREG(ZTOI(xzp)->i_mode) ||
		    S_ISLNK(ZTOI(xzp)->i_mode));

<<<<<<< HEAD
		tx = dmu_tx_create(zsb->z_os);
		dmu_tx_hold_sa(tx, dzp->z_sa_hdl, B_FALSE);
		dmu_tx_hold_zap(tx, dzp->z_id, FALSE, zap.za_name);
		dmu_tx_hold_sa(tx, xzp->z_sa_hdl, B_FALSE);
		dmu_tx_hold_zap(tx, zsb->z_unlinkedobj, FALSE, NULL);
		/* Is this really needed ? */
		zfs_sa_upgrade_txholds(tx, xzp);
=======
		tx = dmu_tx_create(zfsvfs->z_os);
		dmu_tx_hold_sa(tx, dzp->z_sa_hdl, B_FALSE);
		dmu_tx_hold_zap(tx, dzp->z_id, FALSE, zap.za_name);
		dmu_tx_hold_sa(tx, xzp->z_sa_hdl, B_FALSE);
		dmu_tx_hold_zap(tx, zfsvfs->z_unlinkedobj, FALSE, NULL);
		/* Is this really needed ? */
		zfs_sa_upgrade_txholds(tx, xzp);
		dmu_tx_mark_netfree(tx);
>>>>>>> temp
		error = dmu_tx_assign(tx, TXG_WAIT);
		if (error) {
			dmu_tx_abort(tx);
			zfs_iput_async(ZTOI(xzp));
			skipped += 1;
			continue;
		}
		bzero(&dl, sizeof (dl));
		dl.dl_dzp = dzp;
		dl.dl_name = zap.za_name;

		error = zfs_link_destroy(&dl, xzp, tx, 0, NULL);
		if (error)
			skipped += 1;
		dmu_tx_commit(tx);

		zfs_iput_async(ZTOI(xzp));
	}
	zap_cursor_fini(&zc);
	if (error != ENOENT)
		skipped += 1;
	return (skipped);
}

void
zfs_rmnode(znode_t *zp)
{
<<<<<<< HEAD
	zfs_sb_t	*zsb = ZTOZSB(zp);
	objset_t	*os = zsb->z_os;
=======
	zfsvfs_t	*zfsvfs = ZTOZSB(zp);
	objset_t	*os = zfsvfs->z_os;
>>>>>>> temp
	znode_t		*xzp = NULL;
	dmu_tx_t	*tx;
	uint64_t	acl_obj;
	uint64_t	xattr_obj;
<<<<<<< HEAD
	int		error;

	ASSERT(zp->z_links == 0);
=======
	uint64_t	links;
	int		error;

	ASSERT(ZTOI(zp)->i_nlink == 0);
>>>>>>> temp
	ASSERT(atomic_read(&ZTOI(zp)->i_count) == 0);

	/*
	 * If this is an attribute directory, purge its contents.
	 */
	if (S_ISDIR(ZTOI(zp)->i_mode) && (zp->z_pflags & ZFS_XATTR)) {
		if (zfs_purgedir(zp) != 0) {
			/*
			 * Not enough space to delete some xattrs.
			 * Leave it in the unlinked set.
			 */
			zfs_znode_dmu_fini(zp);

			return;
		}
	}

	/*
	 * Free up all the data in the file.  We don't do this for directories
	 * because we need truncate and remove to be in the same tx, like in
	 * zfs_znode_delete(). Otherwise, if we crash here we'll end up with
	 * an inconsistent truncated zap object in the delete queue.  Note a
	 * truncated file is harmless since it only contains user data.
	 */
	if (S_ISREG(ZTOI(zp)->i_mode)) {
		error = dmu_free_long_range(os, zp->z_id, 0, DMU_OBJECT_END);
		if (error) {
			/*
<<<<<<< HEAD
			 * Not enough space.  Leave the file in the unlinked
			 * set.
=======
			 * Not enough space or we were interrupted by unmount.
			 * Leave the file in the unlinked set.
>>>>>>> temp
			 */
			zfs_znode_dmu_fini(zp);
			return;
		}
	}

	/*
	 * If the file has extended attributes, we're going to unlink
	 * the xattr dir.
	 */
<<<<<<< HEAD
	error = sa_lookup(zp->z_sa_hdl, SA_ZPL_XATTR(zsb),
	    &xattr_obj, sizeof (xattr_obj));
	if (error == 0 && xattr_obj) {
		error = zfs_zget(zsb, xattr_obj, &xzp);
=======
	error = sa_lookup(zp->z_sa_hdl, SA_ZPL_XATTR(zfsvfs),
	    &xattr_obj, sizeof (xattr_obj));
	if (error == 0 && xattr_obj) {
		error = zfs_zget(zfsvfs, xattr_obj, &xzp);
>>>>>>> temp
		ASSERT(error == 0);
	}

	acl_obj = zfs_external_acl(zp);

	/*
	 * Set up the final transaction.
	 */
	tx = dmu_tx_create(os);
	dmu_tx_hold_free(tx, zp->z_id, 0, DMU_OBJECT_END);
<<<<<<< HEAD
	dmu_tx_hold_zap(tx, zsb->z_unlinkedobj, FALSE, NULL);
	if (xzp) {
		dmu_tx_hold_zap(tx, zsb->z_unlinkedobj, TRUE, NULL);
=======
	dmu_tx_hold_zap(tx, zfsvfs->z_unlinkedobj, FALSE, NULL);
	if (xzp) {
		dmu_tx_hold_zap(tx, zfsvfs->z_unlinkedobj, TRUE, NULL);
>>>>>>> temp
		dmu_tx_hold_sa(tx, xzp->z_sa_hdl, B_FALSE);
	}
	if (acl_obj)
		dmu_tx_hold_free(tx, acl_obj, 0, DMU_OBJECT_END);

	zfs_sa_upgrade_txholds(tx, zp);
	error = dmu_tx_assign(tx, TXG_WAIT);
	if (error) {
		/*
		 * Not enough space to delete the file.  Leave it in the
		 * unlinked set, leaking it until the fs is remounted (at
		 * which point we'll call zfs_unlinked_drain() to process it).
		 */
		dmu_tx_abort(tx);
		zfs_znode_dmu_fini(zp);
		goto out;
	}

	if (xzp) {
		ASSERT(error == 0);
		mutex_enter(&xzp->z_lock);
		xzp->z_unlinked = B_TRUE;	/* mark xzp for deletion */
<<<<<<< HEAD
		xzp->z_links = 0;	/* no more links to it */
		VERIFY(0 == sa_update(xzp->z_sa_hdl, SA_ZPL_LINKS(zsb),
		    &xzp->z_links, sizeof (xzp->z_links), tx));
=======
		clear_nlink(ZTOI(xzp));		/* no more links to it */
		links = 0;
		VERIFY(0 == sa_update(xzp->z_sa_hdl, SA_ZPL_LINKS(zfsvfs),
		    &links, sizeof (links), tx));
>>>>>>> temp
		mutex_exit(&xzp->z_lock);
		zfs_unlinked_add(xzp, tx);
	}

	/* Remove this znode from the unlinked set */
	VERIFY3U(0, ==,
<<<<<<< HEAD
	    zap_remove_int(zsb->z_os, zsb->z_unlinkedobj, zp->z_id, tx));
=======
	    zap_remove_int(zfsvfs->z_os, zfsvfs->z_unlinkedobj, zp->z_id, tx));
>>>>>>> temp

	zfs_znode_delete(zp, tx);

	dmu_tx_commit(tx);
out:
	if (xzp)
		zfs_iput_async(ZTOI(xzp));
}

static uint64_t
zfs_dirent(znode_t *zp, uint64_t mode)
{
	uint64_t de = zp->z_id;

	if (ZTOZSB(zp)->z_version >= ZPL_VERSION_DIRENT_TYPE)
		de |= IFTODT(mode) << 60;
	return (de);
}

/*
 * Link zp into dl.  Can only fail if zp has been unlinked.
 */
int
zfs_link_create(zfs_dirlock_t *dl, znode_t *zp, dmu_tx_t *tx, int flag)
{
	znode_t *dzp = dl->dl_dzp;
<<<<<<< HEAD
	zfs_sb_t *zsb = ZTOZSB(zp);
=======
	zfsvfs_t *zfsvfs = ZTOZSB(zp);
>>>>>>> temp
	uint64_t value;
	int zp_is_dir = S_ISDIR(ZTOI(zp)->i_mode);
	sa_bulk_attr_t bulk[5];
	uint64_t mtime[2], ctime[2];
<<<<<<< HEAD
=======
	uint64_t links;
>>>>>>> temp
	int count = 0;
	int error;

	mutex_enter(&zp->z_lock);

	if (!(flag & ZRENAMING)) {
		if (zp->z_unlinked) {	/* no new links to unlinked zp */
			ASSERT(!(flag & (ZNEW | ZEXISTS)));
			mutex_exit(&zp->z_lock);
			return (SET_ERROR(ENOENT));
		}
<<<<<<< HEAD
		zp->z_links++;
		SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_LINKS(zsb), NULL,
		    &zp->z_links, sizeof (zp->z_links));

	}
	SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_PARENT(zsb), NULL,
	    &dzp->z_id, sizeof (dzp->z_id));
	SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_FLAGS(zsb), NULL,
	    &zp->z_pflags, sizeof (zp->z_pflags));

	if (!(flag & ZNEW)) {
		SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_CTIME(zsb), NULL,
		    ctime, sizeof (ctime));
		zfs_tstamp_update_setup(zp, STATE_CHANGED, mtime,
		    ctime, B_TRUE);
=======
		if (!(flag & ZNEW)) {
			/*
			 * ZNEW nodes come from zfs_mknode() where the link
			 * count has already been initialised
			 */
			inc_nlink(ZTOI(zp));
			links = ZTOI(zp)->i_nlink;
			SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_LINKS(zfsvfs),
			    NULL, &links, sizeof (links));
		}
	}
	SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_PARENT(zfsvfs), NULL,
	    &dzp->z_id, sizeof (dzp->z_id));
	SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_FLAGS(zfsvfs), NULL,
	    &zp->z_pflags, sizeof (zp->z_pflags));

	if (!(flag & ZNEW)) {
		SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_CTIME(zfsvfs), NULL,
		    ctime, sizeof (ctime));
		zfs_tstamp_update_setup(zp, STATE_CHANGED, mtime,
		    ctime);
>>>>>>> temp
	}
	error = sa_bulk_update(zp->z_sa_hdl, bulk, count, tx);
	ASSERT(error == 0);

	mutex_exit(&zp->z_lock);

	mutex_enter(&dzp->z_lock);
	dzp->z_size++;
<<<<<<< HEAD
	dzp->z_links += zp_is_dir;
	count = 0;
	SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_SIZE(zsb), NULL,
	    &dzp->z_size, sizeof (dzp->z_size));
	SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_LINKS(zsb), NULL,
	    &dzp->z_links, sizeof (dzp->z_links));
	SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_MTIME(zsb), NULL,
	    mtime, sizeof (mtime));
	SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_CTIME(zsb), NULL,
	    ctime, sizeof (ctime));
	SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_FLAGS(zsb), NULL,
	    &dzp->z_pflags, sizeof (dzp->z_pflags));
	zfs_tstamp_update_setup(dzp, CONTENT_MODIFIED, mtime, ctime, B_TRUE);
=======
	if (zp_is_dir)
		inc_nlink(ZTOI(dzp));
	links = ZTOI(dzp)->i_nlink;
	count = 0;
	SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_SIZE(zfsvfs), NULL,
	    &dzp->z_size, sizeof (dzp->z_size));
	SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_LINKS(zfsvfs), NULL,
	    &links, sizeof (links));
	SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_MTIME(zfsvfs), NULL,
	    mtime, sizeof (mtime));
	SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_CTIME(zfsvfs), NULL,
	    ctime, sizeof (ctime));
	SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_FLAGS(zfsvfs), NULL,
	    &dzp->z_pflags, sizeof (dzp->z_pflags));
	zfs_tstamp_update_setup(dzp, CONTENT_MODIFIED, mtime, ctime);
>>>>>>> temp
	error = sa_bulk_update(dzp->z_sa_hdl, bulk, count, tx);
	ASSERT(error == 0);
	mutex_exit(&dzp->z_lock);

	value = zfs_dirent(zp, zp->z_mode);
	error = zap_add(ZTOZSB(zp)->z_os, dzp->z_id, dl->dl_name,
	    8, 1, &value, tx);
	ASSERT(error == 0);

	return (0);
}

<<<<<<< HEAD
=======
/*
 * The match type in the code for this function should conform to:
 *
 * ------------------------------------------------------------------------
 * fs type  | z_norm      | lookup type | match type
 * ---------|-------------|-------------|----------------------------------
 * CS !norm | 0           |           0 | 0 (exact)
 * CS  norm | formX       |           0 | MT_NORMALIZE
 * CI !norm | upper       |   !ZCIEXACT | MT_NORMALIZE
 * CI !norm | upper       |    ZCIEXACT | MT_NORMALIZE | MT_MATCH_CASE
 * CI  norm | upper|formX |   !ZCIEXACT | MT_NORMALIZE
 * CI  norm | upper|formX |    ZCIEXACT | MT_NORMALIZE | MT_MATCH_CASE
 * CM !norm | upper       |    !ZCILOOK | MT_NORMALIZE | MT_MATCH_CASE
 * CM !norm | upper       |     ZCILOOK | MT_NORMALIZE
 * CM  norm | upper|formX |    !ZCILOOK | MT_NORMALIZE | MT_MATCH_CASE
 * CM  norm | upper|formX |     ZCILOOK | MT_NORMALIZE
 *
 * Abbreviations:
 *    CS = Case Sensitive, CI = Case Insensitive, CM = Case Mixed
 *    upper = case folding set by fs type on creation (U8_TEXTPREP_TOUPPER)
 *    formX = unicode normalization form set on fs creation
 */
>>>>>>> temp
static int
zfs_dropname(zfs_dirlock_t *dl, znode_t *zp, znode_t *dzp, dmu_tx_t *tx,
    int flag)
{
	int error;

	if (ZTOZSB(zp)->z_norm) {
<<<<<<< HEAD
		if (((ZTOZSB(zp)->z_case == ZFS_CASE_INSENSITIVE) &&
		    (flag & ZCIEXACT)) ||
		    ((ZTOZSB(zp)->z_case == ZFS_CASE_MIXED) &&
		    !(flag & ZCILOOK)))
			error = zap_remove_norm(ZTOZSB(zp)->z_os,
			    dzp->z_id, dl->dl_name, MT_EXACT, tx);
		else
			error = zap_remove_norm(ZTOZSB(zp)->z_os,
			    dzp->z_id, dl->dl_name, MT_FIRST, tx);
	} else {
		error = zap_remove(ZTOZSB(zp)->z_os,
		    dzp->z_id, dl->dl_name, tx);
=======
		matchtype_t mt = MT_NORMALIZE;

		if ((ZTOZSB(zp)->z_case == ZFS_CASE_INSENSITIVE &&
		    (flag & ZCIEXACT)) ||
		    (ZTOZSB(zp)->z_case == ZFS_CASE_MIXED &&
		    !(flag & ZCILOOK))) {
			mt |= MT_MATCH_CASE;
		}

		error = zap_remove_norm(ZTOZSB(zp)->z_os, dzp->z_id,
		    dl->dl_name, mt, tx);
	} else {
		error = zap_remove(ZTOZSB(zp)->z_os, dzp->z_id, dl->dl_name,
		    tx);
>>>>>>> temp
	}

	return (error);
}

/*
 * Unlink zp from dl, and mark zp for deletion if this was the last link. Can
 * fail if zp is a mount point (EBUSY) or a non-empty directory (ENOTEMPTY).
 * If 'unlinkedp' is NULL, we put unlinked znodes on the unlinked list.
 * If it's non-NULL, we use it to indicate whether the znode needs deletion,
 * and it's the caller's job to do it.
 */
int
zfs_link_destroy(zfs_dirlock_t *dl, znode_t *zp, dmu_tx_t *tx, int flag,
<<<<<<< HEAD
	boolean_t *unlinkedp)
{
	znode_t *dzp = dl->dl_dzp;
	zfs_sb_t *zsb = ZTOZSB(dzp);
=======
    boolean_t *unlinkedp)
{
	znode_t *dzp = dl->dl_dzp;
	zfsvfs_t *zfsvfs = ZTOZSB(dzp);
>>>>>>> temp
	int zp_is_dir = S_ISDIR(ZTOI(zp)->i_mode);
	boolean_t unlinked = B_FALSE;
	sa_bulk_attr_t bulk[5];
	uint64_t mtime[2], ctime[2];
<<<<<<< HEAD
=======
	uint64_t links;
>>>>>>> temp
	int count = 0;
	int error;

#ifdef HAVE_DNLC
	dnlc_remove(ZTOI(dzp), dl->dl_name);
#endif /* HAVE_DNLC */

	if (!(flag & ZRENAMING)) {
		mutex_enter(&zp->z_lock);

		if (zp_is_dir && !zfs_dirempty(zp)) {
			mutex_exit(&zp->z_lock);
			return (SET_ERROR(ENOTEMPTY));
		}

		/*
		 * If we get here, we are going to try to remove the object.
		 * First try removing the name from the directory; if that
		 * fails, return the error.
		 */
		error = zfs_dropname(dl, zp, dzp, tx, flag);
		if (error != 0) {
			mutex_exit(&zp->z_lock);
			return (error);
		}

<<<<<<< HEAD
		if (zp->z_links <= zp_is_dir) {
			zfs_panic_recover("zfs: link count on %lu is %u, "
			    "should be at least %u", zp->z_id,
			    (int)zp->z_links, zp_is_dir + 1);
			zp->z_links = zp_is_dir + 1;
		}
		if (--zp->z_links == zp_is_dir) {
			zp->z_unlinked = B_TRUE;
			zp->z_links = 0;
			unlinked = B_TRUE;
		} else {
			SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_CTIME(zsb),
			    NULL, &ctime, sizeof (ctime));
			SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_FLAGS(zsb),
			    NULL, &zp->z_pflags, sizeof (zp->z_pflags));
			zfs_tstamp_update_setup(zp, STATE_CHANGED, mtime, ctime,
			    B_TRUE);
		}
		SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_LINKS(zsb),
		    NULL, &zp->z_links, sizeof (zp->z_links));
=======
		if (ZTOI(zp)->i_nlink <= zp_is_dir) {
			zfs_panic_recover("zfs: link count on %lu is %u, "
			    "should be at least %u", zp->z_id,
			    (int)ZTOI(zp)->i_nlink, zp_is_dir + 1);
			set_nlink(ZTOI(zp), zp_is_dir + 1);
		}
		drop_nlink(ZTOI(zp));
		if (ZTOI(zp)->i_nlink == zp_is_dir) {
			zp->z_unlinked = B_TRUE;
			clear_nlink(ZTOI(zp));
			unlinked = B_TRUE;
		} else {
			SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_CTIME(zfsvfs),
			    NULL, &ctime, sizeof (ctime));
			SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_FLAGS(zfsvfs),
			    NULL, &zp->z_pflags, sizeof (zp->z_pflags));
			zfs_tstamp_update_setup(zp, STATE_CHANGED, mtime,
			    ctime);
		}
		links = ZTOI(zp)->i_nlink;
		SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_LINKS(zfsvfs),
		    NULL, &links, sizeof (links));
>>>>>>> temp
		error = sa_bulk_update(zp->z_sa_hdl, bulk, count, tx);
		count = 0;
		ASSERT(error == 0);
		mutex_exit(&zp->z_lock);
	} else {
		error = zfs_dropname(dl, zp, dzp, tx, flag);
		if (error != 0)
			return (error);
	}

	mutex_enter(&dzp->z_lock);
	dzp->z_size--;		/* one dirent removed */
<<<<<<< HEAD
	dzp->z_links -= zp_is_dir;	/* ".." link from zp */
	SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_LINKS(zsb),
	    NULL, &dzp->z_links, sizeof (dzp->z_links));
	SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_SIZE(zsb),
	    NULL, &dzp->z_size, sizeof (dzp->z_size));
	SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_CTIME(zsb),
	    NULL, ctime, sizeof (ctime));
	SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_MTIME(zsb),
	    NULL, mtime, sizeof (mtime));
	SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_FLAGS(zsb),
	    NULL, &dzp->z_pflags, sizeof (dzp->z_pflags));
	zfs_tstamp_update_setup(dzp, CONTENT_MODIFIED, mtime, ctime, B_TRUE);
=======
	if (zp_is_dir)
		drop_nlink(ZTOI(dzp));	/* ".." link from zp */
	links = ZTOI(dzp)->i_nlink;
	SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_LINKS(zfsvfs),
	    NULL, &links, sizeof (links));
	SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_SIZE(zfsvfs),
	    NULL, &dzp->z_size, sizeof (dzp->z_size));
	SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_CTIME(zfsvfs),
	    NULL, ctime, sizeof (ctime));
	SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_MTIME(zfsvfs),
	    NULL, mtime, sizeof (mtime));
	SA_ADD_BULK_ATTR(bulk, count, SA_ZPL_FLAGS(zfsvfs),
	    NULL, &dzp->z_pflags, sizeof (dzp->z_pflags));
	zfs_tstamp_update_setup(dzp, CONTENT_MODIFIED, mtime, ctime);
>>>>>>> temp
	error = sa_bulk_update(dzp->z_sa_hdl, bulk, count, tx);
	ASSERT(error == 0);
	mutex_exit(&dzp->z_lock);

	if (unlinkedp != NULL)
		*unlinkedp = unlinked;
	else if (unlinked)
		zfs_unlinked_add(zp, tx);

	return (0);
}

/*
 * Indicate whether the directory is empty.  Works with or without z_lock
 * held, but can only be consider a hint in the latter case.  Returns true
 * if only "." and ".." remain and there's no work in progress.
<<<<<<< HEAD
=======
 *
 * The internal ZAP size, rather than zp->z_size, needs to be checked since
 * some consumers (Lustre) do not strictly maintain an accurate SA_ZPL_SIZE.
>>>>>>> temp
 */
boolean_t
zfs_dirempty(znode_t *dzp)
{
<<<<<<< HEAD
	return (dzp->z_size == 2 && dzp->z_dirlocks == 0);
=======
	zfsvfs_t *zfsvfs = ZTOZSB(dzp);
	uint64_t count;
	int error;

	if (dzp->z_dirlocks != NULL)
		return (B_FALSE);

	error = zap_count(zfsvfs->z_os, dzp->z_id, &count);
	if (error != 0 || count != 0)
		return (B_FALSE);

	return (B_TRUE);
>>>>>>> temp
}

int
zfs_make_xattrdir(znode_t *zp, vattr_t *vap, struct inode **xipp, cred_t *cr)
{
<<<<<<< HEAD
	zfs_sb_t *zsb = ZTOZSB(zp);
=======
	zfsvfs_t *zfsvfs = ZTOZSB(zp);
>>>>>>> temp
	znode_t *xzp;
	dmu_tx_t *tx;
	int error;
	zfs_acl_ids_t acl_ids;
	boolean_t fuid_dirtied;
#ifdef DEBUG
	uint64_t parent;
#endif

	*xipp = NULL;

	if ((error = zfs_zaccess(zp, ACE_WRITE_NAMED_ATTRS, 0, B_FALSE, cr)))
		return (error);

	if ((error = zfs_acl_ids_create(zp, IS_XATTR, vap, cr, NULL,
	    &acl_ids)) != 0)
		return (error);
<<<<<<< HEAD
	if (zfs_acl_ids_overquota(zsb, &acl_ids)) {
=======
	if (zfs_acl_ids_overquota(zfsvfs, &acl_ids)) {
>>>>>>> temp
		zfs_acl_ids_free(&acl_ids);
		return (SET_ERROR(EDQUOT));
	}

<<<<<<< HEAD
	tx = dmu_tx_create(zsb->z_os);
=======
	tx = dmu_tx_create(zfsvfs->z_os);
>>>>>>> temp
	dmu_tx_hold_sa_create(tx, acl_ids.z_aclp->z_acl_bytes +
	    ZFS_SA_BASE_ATTR_SIZE);
	dmu_tx_hold_sa(tx, zp->z_sa_hdl, B_TRUE);
	dmu_tx_hold_zap(tx, DMU_NEW_OBJECT, FALSE, NULL);
<<<<<<< HEAD
	fuid_dirtied = zsb->z_fuid_dirty;
	if (fuid_dirtied)
		zfs_fuid_txhold(zsb, tx);
=======
	fuid_dirtied = zfsvfs->z_fuid_dirty;
	if (fuid_dirtied)
		zfs_fuid_txhold(zfsvfs, tx);
>>>>>>> temp
	error = dmu_tx_assign(tx, TXG_WAIT);
	if (error) {
		zfs_acl_ids_free(&acl_ids);
		dmu_tx_abort(tx);
		return (error);
	}
	zfs_mknode(zp, vap, tx, cr, IS_XATTR, &xzp, &acl_ids);

	if (fuid_dirtied)
<<<<<<< HEAD
		zfs_fuid_sync(zsb, tx);

#ifdef DEBUG
	error = sa_lookup(xzp->z_sa_hdl, SA_ZPL_PARENT(zsb),
=======
		zfs_fuid_sync(zfsvfs, tx);

#ifdef DEBUG
	error = sa_lookup(xzp->z_sa_hdl, SA_ZPL_PARENT(zfsvfs),
>>>>>>> temp
	    &parent, sizeof (parent));
	ASSERT(error == 0 && parent == zp->z_id);
#endif

<<<<<<< HEAD
	VERIFY(0 == sa_update(zp->z_sa_hdl, SA_ZPL_XATTR(zsb), &xzp->z_id,
	    sizeof (xzp->z_id), tx));

	(void) zfs_log_create(zsb->z_log, tx, TX_MKXATTR, zp,
	    xzp, "", NULL, acl_ids.z_fuidp, vap);
=======
	VERIFY(0 == sa_update(zp->z_sa_hdl, SA_ZPL_XATTR(zfsvfs), &xzp->z_id,
	    sizeof (xzp->z_id), tx));

	if (!zp->z_unlinked)
		(void) zfs_log_create(zfsvfs->z_log, tx, TX_MKXATTR, zp,
		    xzp, "", NULL, acl_ids.z_fuidp, vap);
>>>>>>> temp

	zfs_acl_ids_free(&acl_ids);
	dmu_tx_commit(tx);

	*xipp = ZTOI(xzp);

	return (0);
}

/*
 * Return a znode for the extended attribute directory for zp.
 * ** If the directory does not already exist, it is created **
 *
 *	IN:	zp	- znode to obtain attribute directory from
 *		cr	- credentials of caller
 *		flags	- flags from the VOP_LOOKUP call
 *
 *	OUT:	xipp	- pointer to extended attribute znode
 *
 *	RETURN:	0 on success
 *		error number on failure
 */
int
zfs_get_xattrdir(znode_t *zp, struct inode **xipp, cred_t *cr, int flags)
{
<<<<<<< HEAD
	zfs_sb_t	*zsb = ZTOZSB(zp);
=======
	zfsvfs_t	*zfsvfs = ZTOZSB(zp);
>>>>>>> temp
	znode_t		*xzp;
	zfs_dirlock_t	*dl;
	vattr_t		va;
	int		error;
top:
	error = zfs_dirent_lock(&dl, zp, "", &xzp, ZXATTR, NULL, NULL);
	if (error)
		return (error);

	if (xzp != NULL) {
		*xipp = ZTOI(xzp);
		zfs_dirent_unlock(dl);
		return (0);
	}

	if (!(flags & CREATE_XATTR_DIR)) {
		zfs_dirent_unlock(dl);
		return (SET_ERROR(ENOENT));
	}

<<<<<<< HEAD
	if (zfs_is_readonly(zsb)) {
=======
	if (zfs_is_readonly(zfsvfs)) {
>>>>>>> temp
		zfs_dirent_unlock(dl);
		return (SET_ERROR(EROFS));
	}

	/*
	 * The ability to 'create' files in an attribute
	 * directory comes from the write_xattr permission on the base file.
	 *
	 * The ability to 'search' an attribute directory requires
	 * read_xattr permission on the base file.
	 *
	 * Once in a directory the ability to read/write attributes
	 * is controlled by the permissions on the attribute file.
	 */
	va.va_mask = ATTR_MODE | ATTR_UID | ATTR_GID;
	va.va_mode = S_IFDIR | S_ISVTX | 0777;
	zfs_fuid_map_ids(zp, cr, &va.va_uid, &va.va_gid);

	va.va_dentry = NULL;
	error = zfs_make_xattrdir(zp, &va, xipp, cr);
	zfs_dirent_unlock(dl);

	if (error == ERESTART) {
		/* NB: we already did dmu_tx_wait() if necessary */
		goto top;
	}

	return (error);
}

/*
 * Decide whether it is okay to remove within a sticky directory.
 *
 * In sticky directories, write access is not sufficient;
 * you can remove entries from a directory only if:
 *
 *	you own the directory,
 *	you own the entry,
<<<<<<< HEAD
 *	the entry is a plain file and you have write access,
=======
 *	you have write access to the entry,
>>>>>>> temp
 *	or you are privileged (checked in secpolicy...).
 *
 * The function returns 0 if remove access is granted.
 */
int
zfs_sticky_remove_access(znode_t *zdp, znode_t *zp, cred_t *cr)
{
	uid_t		uid;
	uid_t		downer;
	uid_t		fowner;
<<<<<<< HEAD
	zfs_sb_t	*zsb = ZTOZSB(zdp);

	if (zsb->z_replay)
=======
	zfsvfs_t	*zfsvfs = ZTOZSB(zdp);

	if (zfsvfs->z_replay)
>>>>>>> temp
		return (0);

	if ((zdp->z_mode & S_ISVTX) == 0)
		return (0);

<<<<<<< HEAD
	downer = zfs_fuid_map_id(zsb, zdp->z_uid, cr, ZFS_OWNER);
	fowner = zfs_fuid_map_id(zsb, zp->z_uid, cr, ZFS_OWNER);

	if ((uid = crgetuid(cr)) == downer || uid == fowner ||
	    (S_ISDIR(ZTOI(zp)->i_mode) &&
	    zfs_zaccess(zp, ACE_WRITE_DATA, 0, B_FALSE, cr) == 0))
=======
	downer = zfs_fuid_map_id(zfsvfs, KUID_TO_SUID(ZTOI(zdp)->i_uid),
	    cr, ZFS_OWNER);
	fowner = zfs_fuid_map_id(zfsvfs, KUID_TO_SUID(ZTOI(zp)->i_uid),
	    cr, ZFS_OWNER);

	if ((uid = crgetuid(cr)) == downer || uid == fowner ||
	    zfs_zaccess(zp, ACE_WRITE_DATA, 0, B_FALSE, cr) == 0)
>>>>>>> temp
		return (0);
	else
		return (secpolicy_vnode_remove(cr));
}
