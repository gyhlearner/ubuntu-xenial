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
 * Copyright (c) 2012, 2014 by Delphix. All rights reserved.
 * Copyright (c) 2013 by Saso Kiselkov. All rights reserved.
 * Copyright (c) 2013, Joyent, Inc. All rights reserved.
 * Copyright (c) 2014 Spectra Logic Corporation, All rights reserved.
 * Copyright (c) 2015 Nexenta Systems, Inc. All rights reserved.
 * Copyright (c) 2015, STRATO AG, Inc. All rights reserved.
 * Copyright (c) 2016 Actifio, Inc. All rights reserved.
=======

/*
 * Copyright (c) 2005, 2010, Oracle and/or its affiliates. All rights reserved.
 * Copyright (c) 2012, 2017 by Delphix. All rights reserved.
 * Copyright (c) 2013 by Saso Kiselkov. All rights reserved.
 * Copyright (c) 2013, Joyent, Inc. All rights reserved.
 * Copyright (c) 2014 Spectra Logic Corporation, All rights reserved.
 * Copyright (c) 2015, STRATO AG, Inc. All rights reserved.
 * Copyright (c) 2016 Actifio, Inc. All rights reserved.
 * Copyright 2017 Nexenta Systems, Inc.
>>>>>>> temp
 */

/* Portions Copyright 2010 Robert Milkowski */

<<<<<<< HEAD
=======
#include <sys/zfeature.h>
>>>>>>> temp
#include <sys/cred.h>
#include <sys/zfs_context.h>
#include <sys/dmu_objset.h>
#include <sys/dsl_dir.h>
#include <sys/dsl_dataset.h>
#include <sys/dsl_prop.h>
#include <sys/dsl_pool.h>
#include <sys/dsl_synctask.h>
#include <sys/dsl_deleg.h>
#include <sys/dnode.h>
#include <sys/dbuf.h>
#include <sys/zvol.h>
#include <sys/dmu_tx.h>
#include <sys/zap.h>
#include <sys/zil.h>
#include <sys/dmu_impl.h>
#include <sys/zfs_ioctl.h>
#include <sys/sa.h>
#include <sys/zfs_onexit.h>
#include <sys/dsl_destroy.h>
#include <sys/vdev.h>
<<<<<<< HEAD
=======
#include <sys/policy.h>
#include <sys/spa_impl.h>
>>>>>>> temp

/*
 * Needed to close a window in dnode_move() that allows the objset to be freed
 * before it can be safely accessed.
 */
krwlock_t os_lock;

/*
<<<<<<< HEAD
 * Tunable to overwrite the maximum number of threads for the parallization
=======
 * Tunable to overwrite the maximum number of threads for the parallelization
>>>>>>> temp
 * of dmu_objset_find_dp, needed to speed up the import of pools with many
 * datasets.
 * Default is 4 times the number of leaf vdevs.
 */
int dmu_find_threads = 0;

<<<<<<< HEAD
static void dmu_objset_find_dp_cb(void *arg);

=======
/*
 * Backfill lower metadnode objects after this many have been freed.
 * Backfilling negatively impacts object creation rates, so only do it
 * if there are enough holes to fill.
 */
int dmu_rescan_dnode_threshold = 1 << DN_MAX_INDBLKSHIFT;

static void dmu_objset_find_dp_cb(void *arg);

static void dmu_objset_upgrade(objset_t *os, dmu_objset_upgrade_cb_t cb);
static void dmu_objset_upgrade_stop(objset_t *os);

>>>>>>> temp
void
dmu_objset_init(void)
{
	rw_init(&os_lock, NULL, RW_DEFAULT, NULL);
}

void
dmu_objset_fini(void)
{
	rw_destroy(&os_lock);
}

spa_t *
dmu_objset_spa(objset_t *os)
{
	return (os->os_spa);
}

zilog_t *
dmu_objset_zil(objset_t *os)
{
	return (os->os_zil);
}

dsl_pool_t *
dmu_objset_pool(objset_t *os)
{
	dsl_dataset_t *ds;

	if ((ds = os->os_dsl_dataset) != NULL && ds->ds_dir)
		return (ds->ds_dir->dd_pool);
	else
		return (spa_get_dsl(os->os_spa));
}

dsl_dataset_t *
dmu_objset_ds(objset_t *os)
{
	return (os->os_dsl_dataset);
}

dmu_objset_type_t
dmu_objset_type(objset_t *os)
{
	return (os->os_phys->os_type);
}

void
dmu_objset_name(objset_t *os, char *buf)
{
	dsl_dataset_name(os->os_dsl_dataset, buf);
}

uint64_t
dmu_objset_id(objset_t *os)
{
	dsl_dataset_t *ds = os->os_dsl_dataset;

	return (ds ? ds->ds_object : 0);
}

<<<<<<< HEAD
=======
uint64_t
dmu_objset_dnodesize(objset_t *os)
{
	return (os->os_dnodesize);
}

>>>>>>> temp
zfs_sync_type_t
dmu_objset_syncprop(objset_t *os)
{
	return (os->os_sync);
}

zfs_logbias_op_t
dmu_objset_logbias(objset_t *os)
{
	return (os->os_logbias);
}

static void
checksum_changed_cb(void *arg, uint64_t newval)
{
	objset_t *os = arg;

	/*
	 * Inheritance should have been done by now.
	 */
	ASSERT(newval != ZIO_CHECKSUM_INHERIT);

	os->os_checksum = zio_checksum_select(newval, ZIO_CHECKSUM_ON_VALUE);
}

static void
compression_changed_cb(void *arg, uint64_t newval)
{
	objset_t *os = arg;

	/*
	 * Inheritance and range checking should have been done by now.
	 */
	ASSERT(newval != ZIO_COMPRESS_INHERIT);

	os->os_compress = zio_compress_select(os->os_spa, newval,
	    ZIO_COMPRESS_ON);
}

static void
copies_changed_cb(void *arg, uint64_t newval)
{
	objset_t *os = arg;

	/*
	 * Inheritance and range checking should have been done by now.
	 */
	ASSERT(newval > 0);
	ASSERT(newval <= spa_max_replication(os->os_spa));

	os->os_copies = newval;
}

static void
dedup_changed_cb(void *arg, uint64_t newval)
{
	objset_t *os = arg;
	spa_t *spa = os->os_spa;
	enum zio_checksum checksum;

	/*
	 * Inheritance should have been done by now.
	 */
	ASSERT(newval != ZIO_CHECKSUM_INHERIT);

	checksum = zio_checksum_dedup_select(spa, newval, ZIO_CHECKSUM_OFF);

	os->os_dedup_checksum = checksum & ZIO_CHECKSUM_MASK;
	os->os_dedup_verify = !!(checksum & ZIO_CHECKSUM_VERIFY);
}

static void
primary_cache_changed_cb(void *arg, uint64_t newval)
{
	objset_t *os = arg;

	/*
	 * Inheritance and range checking should have been done by now.
	 */
	ASSERT(newval == ZFS_CACHE_ALL || newval == ZFS_CACHE_NONE ||
	    newval == ZFS_CACHE_METADATA);

	os->os_primary_cache = newval;
}

static void
secondary_cache_changed_cb(void *arg, uint64_t newval)
{
	objset_t *os = arg;

	/*
	 * Inheritance and range checking should have been done by now.
	 */
	ASSERT(newval == ZFS_CACHE_ALL || newval == ZFS_CACHE_NONE ||
	    newval == ZFS_CACHE_METADATA);

	os->os_secondary_cache = newval;
}

static void
sync_changed_cb(void *arg, uint64_t newval)
{
	objset_t *os = arg;

	/*
	 * Inheritance and range checking should have been done by now.
	 */
	ASSERT(newval == ZFS_SYNC_STANDARD || newval == ZFS_SYNC_ALWAYS ||
	    newval == ZFS_SYNC_DISABLED);

	os->os_sync = newval;
	if (os->os_zil)
		zil_set_sync(os->os_zil, newval);
}

static void
redundant_metadata_changed_cb(void *arg, uint64_t newval)
{
	objset_t *os = arg;

	/*
	 * Inheritance and range checking should have been done by now.
	 */
	ASSERT(newval == ZFS_REDUNDANT_METADATA_ALL ||
	    newval == ZFS_REDUNDANT_METADATA_MOST);

	os->os_redundant_metadata = newval;
}

static void
<<<<<<< HEAD
=======
dnodesize_changed_cb(void *arg, uint64_t newval)
{
	objset_t *os = arg;

	switch (newval) {
	case ZFS_DNSIZE_LEGACY:
		os->os_dnodesize = DNODE_MIN_SIZE;
		break;
	case ZFS_DNSIZE_AUTO:
		/*
		 * Choose a dnode size that will work well for most
		 * workloads if the user specified "auto". Future code
		 * improvements could dynamically select a dnode size
		 * based on observed workload patterns.
		 */
		os->os_dnodesize = DNODE_MIN_SIZE * 2;
		break;
	case ZFS_DNSIZE_1K:
	case ZFS_DNSIZE_2K:
	case ZFS_DNSIZE_4K:
	case ZFS_DNSIZE_8K:
	case ZFS_DNSIZE_16K:
		os->os_dnodesize = newval;
		break;
	}
}

static void
>>>>>>> temp
logbias_changed_cb(void *arg, uint64_t newval)
{
	objset_t *os = arg;

	ASSERT(newval == ZFS_LOGBIAS_LATENCY ||
	    newval == ZFS_LOGBIAS_THROUGHPUT);
	os->os_logbias = newval;
	if (os->os_zil)
		zil_set_logbias(os->os_zil, newval);
}

static void
recordsize_changed_cb(void *arg, uint64_t newval)
{
	objset_t *os = arg;

	os->os_recordsize = newval;
}

void
dmu_objset_byteswap(void *buf, size_t size)
{
	objset_phys_t *osp = buf;

	ASSERT(size == OBJSET_OLD_PHYS_SIZE || size == sizeof (objset_phys_t));
	dnode_byteswap(&osp->os_meta_dnode);
	byteswap_uint64_array(&osp->os_zil_header, sizeof (zil_header_t));
	osp->os_type = BSWAP_64(osp->os_type);
	osp->os_flags = BSWAP_64(osp->os_flags);
	if (size == sizeof (objset_phys_t)) {
		dnode_byteswap(&osp->os_userused_dnode);
		dnode_byteswap(&osp->os_groupused_dnode);
	}
}

<<<<<<< HEAD
=======
/*
 * The hash is a CRC-based hash of the objset_t pointer and the object number.
 */
static uint64_t
dnode_hash(const objset_t *os, uint64_t obj)
{
	uintptr_t osv = (uintptr_t)os;
	uint64_t crc = -1ULL;

	ASSERT(zfs_crc64_table[128] == ZFS_CRC64_POLY);
	/*
	 * The low 6 bits of the pointer don't have much entropy, because
	 * the objset_t is larger than 2^6 bytes long.
	 */
	crc = (crc >> 8) ^ zfs_crc64_table[(crc ^ (osv >> 6)) & 0xFF];
	crc = (crc >> 8) ^ zfs_crc64_table[(crc ^ (obj >> 0)) & 0xFF];
	crc = (crc >> 8) ^ zfs_crc64_table[(crc ^ (obj >> 8)) & 0xFF];
	crc = (crc >> 8) ^ zfs_crc64_table[(crc ^ (obj >> 16)) & 0xFF];

	crc ^= (osv>>14) ^ (obj>>24);

	return (crc);
}

unsigned int
dnode_multilist_index_func(multilist_t *ml, void *obj)
{
	dnode_t *dn = obj;
	return (dnode_hash(dn->dn_objset, dn->dn_object) %
	    multilist_get_num_sublists(ml));
}

>>>>>>> temp
int
dmu_objset_open_impl(spa_t *spa, dsl_dataset_t *ds, blkptr_t *bp,
    objset_t **osp)
{
	objset_t *os;
	int i, err;

	ASSERT(ds == NULL || MUTEX_HELD(&ds->ds_opening_lock));

	os = kmem_zalloc(sizeof (objset_t), KM_SLEEP);
	os->os_dsl_dataset = ds;
	os->os_spa = spa;
	os->os_rootbp = bp;
	if (!BP_IS_HOLE(os->os_rootbp)) {
		arc_flags_t aflags = ARC_FLAG_WAIT;
		zbookmark_phys_t zb;
		SET_BOOKMARK(&zb, ds ? ds->ds_object : DMU_META_OBJSET,
		    ZB_ROOT_OBJECT, ZB_ROOT_LEVEL, ZB_ROOT_BLKID);

		if (DMU_OS_IS_L2CACHEABLE(os))
			aflags |= ARC_FLAG_L2CACHE;
<<<<<<< HEAD
		if (DMU_OS_IS_L2COMPRESSIBLE(os))
			aflags |= ARC_FLAG_L2COMPRESS;
=======
>>>>>>> temp

		dprintf_bp(os->os_rootbp, "reading %s", "");
		err = arc_read(NULL, spa, os->os_rootbp,
		    arc_getbuf_func, &os->os_phys_buf,
		    ZIO_PRIORITY_SYNC_READ, ZIO_FLAG_CANFAIL, &aflags, &zb);
		if (err != 0) {
			kmem_free(os, sizeof (objset_t));
			/* convert checksum errors into IO errors */
			if (err == ECKSUM)
				err = SET_ERROR(EIO);
			return (err);
		}

		/* Increase the blocksize if we are permitted. */
		if (spa_version(spa) >= SPA_VERSION_USERSPACE &&
		    arc_buf_size(os->os_phys_buf) < sizeof (objset_phys_t)) {
<<<<<<< HEAD
			arc_buf_t *buf = arc_buf_alloc(spa,
			    sizeof (objset_phys_t), &os->os_phys_buf,
			    ARC_BUFC_METADATA);
			bzero(buf->b_data, sizeof (objset_phys_t));
			bcopy(os->os_phys_buf->b_data, buf->b_data,
			    arc_buf_size(os->os_phys_buf));
			(void) arc_buf_remove_ref(os->os_phys_buf,
			    &os->os_phys_buf);
=======
			arc_buf_t *buf = arc_alloc_buf(spa, &os->os_phys_buf,
			    ARC_BUFC_METADATA, sizeof (objset_phys_t));
			bzero(buf->b_data, sizeof (objset_phys_t));
			bcopy(os->os_phys_buf->b_data, buf->b_data,
			    arc_buf_size(os->os_phys_buf));
			arc_buf_destroy(os->os_phys_buf, &os->os_phys_buf);
>>>>>>> temp
			os->os_phys_buf = buf;
		}

		os->os_phys = os->os_phys_buf->b_data;
		os->os_flags = os->os_phys->os_flags;
	} else {
		int size = spa_version(spa) >= SPA_VERSION_USERSPACE ?
		    sizeof (objset_phys_t) : OBJSET_OLD_PHYS_SIZE;
<<<<<<< HEAD
		os->os_phys_buf = arc_buf_alloc(spa, size,
		    &os->os_phys_buf, ARC_BUFC_METADATA);
=======
		os->os_phys_buf = arc_alloc_buf(spa, &os->os_phys_buf,
		    ARC_BUFC_METADATA, size);
>>>>>>> temp
		os->os_phys = os->os_phys_buf->b_data;
		bzero(os->os_phys, size);
	}

	/*
	 * Note: the changed_cb will be called once before the register
	 * func returns, thus changing the checksum/compression from the
	 * default (fletcher2/off).  Snapshots don't need to know about
	 * checksum/compression/copies.
	 */
	if (ds != NULL) {
<<<<<<< HEAD
=======
		boolean_t needlock = B_FALSE;

		/*
		 * Note: it's valid to open the objset if the dataset is
		 * long-held, in which case the pool_config lock will not
		 * be held.
		 */
		if (!dsl_pool_config_held(dmu_objset_pool(os))) {
			needlock = B_TRUE;
			dsl_pool_config_enter(dmu_objset_pool(os), FTAG);
		}
>>>>>>> temp
		err = dsl_prop_register(ds,
		    zfs_prop_to_name(ZFS_PROP_PRIMARYCACHE),
		    primary_cache_changed_cb, os);
		if (err == 0) {
			err = dsl_prop_register(ds,
			    zfs_prop_to_name(ZFS_PROP_SECONDARYCACHE),
			    secondary_cache_changed_cb, os);
		}
		if (!ds->ds_is_snapshot) {
			if (err == 0) {
				err = dsl_prop_register(ds,
				    zfs_prop_to_name(ZFS_PROP_CHECKSUM),
				    checksum_changed_cb, os);
			}
			if (err == 0) {
				err = dsl_prop_register(ds,
				    zfs_prop_to_name(ZFS_PROP_COMPRESSION),
				    compression_changed_cb, os);
			}
			if (err == 0) {
				err = dsl_prop_register(ds,
				    zfs_prop_to_name(ZFS_PROP_COPIES),
				    copies_changed_cb, os);
			}
			if (err == 0) {
				err = dsl_prop_register(ds,
				    zfs_prop_to_name(ZFS_PROP_DEDUP),
				    dedup_changed_cb, os);
			}
			if (err == 0) {
				err = dsl_prop_register(ds,
				    zfs_prop_to_name(ZFS_PROP_LOGBIAS),
				    logbias_changed_cb, os);
			}
			if (err == 0) {
				err = dsl_prop_register(ds,
				    zfs_prop_to_name(ZFS_PROP_SYNC),
				    sync_changed_cb, os);
			}
			if (err == 0) {
				err = dsl_prop_register(ds,
				    zfs_prop_to_name(
				    ZFS_PROP_REDUNDANT_METADATA),
				    redundant_metadata_changed_cb, os);
			}
			if (err == 0) {
				err = dsl_prop_register(ds,
				    zfs_prop_to_name(ZFS_PROP_RECORDSIZE),
				    recordsize_changed_cb, os);
			}
<<<<<<< HEAD
		}
		if (err != 0) {
			VERIFY(arc_buf_remove_ref(os->os_phys_buf,
			    &os->os_phys_buf));
=======
			if (err == 0) {
				err = dsl_prop_register(ds,
				    zfs_prop_to_name(ZFS_PROP_DNODESIZE),
				    dnodesize_changed_cb, os);
			}
		}
		if (needlock)
			dsl_pool_config_exit(dmu_objset_pool(os), FTAG);
		if (err != 0) {
			arc_buf_destroy(os->os_phys_buf, &os->os_phys_buf);
>>>>>>> temp
			kmem_free(os, sizeof (objset_t));
			return (err);
		}
	} else {
		/* It's the meta-objset. */
		os->os_checksum = ZIO_CHECKSUM_FLETCHER_4;
		os->os_compress = ZIO_COMPRESS_ON;
		os->os_copies = spa_max_replication(spa);
		os->os_dedup_checksum = ZIO_CHECKSUM_OFF;
		os->os_dedup_verify = B_FALSE;
		os->os_logbias = ZFS_LOGBIAS_LATENCY;
		os->os_sync = ZFS_SYNC_STANDARD;
		os->os_primary_cache = ZFS_CACHE_ALL;
		os->os_secondary_cache = ZFS_CACHE_ALL;
<<<<<<< HEAD
=======
		os->os_dnodesize = DNODE_MIN_SIZE;
>>>>>>> temp
	}

	if (ds == NULL || !ds->ds_is_snapshot)
		os->os_zil_header = os->os_phys->os_zil_header;
	os->os_zil = zil_alloc(os, &os->os_zil_header);

	for (i = 0; i < TXG_SIZE; i++) {
<<<<<<< HEAD
		list_create(&os->os_dirty_dnodes[i], sizeof (dnode_t),
		    offsetof(dnode_t, dn_dirty_link[i]));
		list_create(&os->os_free_dnodes[i], sizeof (dnode_t),
		    offsetof(dnode_t, dn_dirty_link[i]));
=======
		os->os_dirty_dnodes[i] = multilist_create(sizeof (dnode_t),
		    offsetof(dnode_t, dn_dirty_link[i]),
		    dnode_multilist_index_func);
>>>>>>> temp
	}
	list_create(&os->os_dnodes, sizeof (dnode_t),
	    offsetof(dnode_t, dn_link));
	list_create(&os->os_downgraded_dbufs, sizeof (dmu_buf_impl_t),
	    offsetof(dmu_buf_impl_t, db_link));

	list_link_init(&os->os_evicting_node);

	mutex_init(&os->os_lock, NULL, MUTEX_DEFAULT, NULL);
<<<<<<< HEAD
	mutex_init(&os->os_obj_lock, NULL, MUTEX_DEFAULT, NULL);
	mutex_init(&os->os_user_ptr_lock, NULL, MUTEX_DEFAULT, NULL);
=======
	mutex_init(&os->os_userused_lock, NULL, MUTEX_DEFAULT, NULL);
	mutex_init(&os->os_obj_lock, NULL, MUTEX_DEFAULT, NULL);
	mutex_init(&os->os_user_ptr_lock, NULL, MUTEX_DEFAULT, NULL);
	os->os_obj_next_percpu_len = boot_ncpus;
	os->os_obj_next_percpu = kmem_zalloc(os->os_obj_next_percpu_len *
	    sizeof (os->os_obj_next_percpu[0]), KM_SLEEP);
>>>>>>> temp

	dnode_special_open(os, &os->os_phys->os_meta_dnode,
	    DMU_META_DNODE_OBJECT, &os->os_meta_dnode);
	if (arc_buf_size(os->os_phys_buf) >= sizeof (objset_phys_t)) {
		dnode_special_open(os, &os->os_phys->os_userused_dnode,
		    DMU_USERUSED_OBJECT, &os->os_userused_dnode);
		dnode_special_open(os, &os->os_phys->os_groupused_dnode,
		    DMU_GROUPUSED_OBJECT, &os->os_groupused_dnode);
	}

<<<<<<< HEAD
=======
	mutex_init(&os->os_upgrade_lock, NULL, MUTEX_DEFAULT, NULL);

>>>>>>> temp
	*osp = os;
	return (0);
}

int
dmu_objset_from_ds(dsl_dataset_t *ds, objset_t **osp)
{
	int err = 0;

<<<<<<< HEAD
	mutex_enter(&ds->ds_opening_lock);
	if (ds->ds_objset == NULL) {
		objset_t *os;
		err = dmu_objset_open_impl(dsl_dataset_get_spa(ds),
		    ds, dsl_dataset_get_blkptr(ds), &os);
=======
	/*
	 * We shouldn't be doing anything with dsl_dataset_t's unless the
	 * pool_config lock is held, or the dataset is long-held.
	 */
	ASSERT(dsl_pool_config_held(ds->ds_dir->dd_pool) ||
	    dsl_dataset_long_held(ds));

	mutex_enter(&ds->ds_opening_lock);
	if (ds->ds_objset == NULL) {
		objset_t *os;
		rrw_enter(&ds->ds_bp_rwlock, RW_READER, FTAG);
		err = dmu_objset_open_impl(dsl_dataset_get_spa(ds),
		    ds, dsl_dataset_get_blkptr(ds), &os);
		rrw_exit(&ds->ds_bp_rwlock, FTAG);
>>>>>>> temp

		if (err == 0) {
			mutex_enter(&ds->ds_lock);
			ASSERT(ds->ds_objset == NULL);
			ds->ds_objset = os;
			mutex_exit(&ds->ds_lock);
		}
	}
	*osp = ds->ds_objset;
	mutex_exit(&ds->ds_opening_lock);
	return (err);
}

/*
 * Holds the pool while the objset is held.  Therefore only one objset
 * can be held at a time.
 */
int
dmu_objset_hold(const char *name, void *tag, objset_t **osp)
{
	dsl_pool_t *dp;
	dsl_dataset_t *ds;
	int err;

	err = dsl_pool_hold(name, tag, &dp);
	if (err != 0)
		return (err);
	err = dsl_dataset_hold(dp, name, tag, &ds);
	if (err != 0) {
		dsl_pool_rele(dp, tag);
		return (err);
	}

	err = dmu_objset_from_ds(ds, osp);
	if (err != 0) {
		dsl_dataset_rele(ds, tag);
		dsl_pool_rele(dp, tag);
	}

	return (err);
}

static int
dmu_objset_own_impl(dsl_dataset_t *ds, dmu_objset_type_t type,
    boolean_t readonly, void *tag, objset_t **osp)
{
	int err;

	err = dmu_objset_from_ds(ds, osp);
	if (err != 0) {
		dsl_dataset_disown(ds, tag);
	} else if (type != DMU_OST_ANY && type != (*osp)->os_phys->os_type) {
		dsl_dataset_disown(ds, tag);
		return (SET_ERROR(EINVAL));
	} else if (!readonly && dsl_dataset_is_snapshot(ds)) {
		dsl_dataset_disown(ds, tag);
		return (SET_ERROR(EROFS));
	}
	return (err);
}

/*
 * dsl_pool must not be held when this is called.
 * Upon successful return, there will be a longhold on the dataset,
 * and the dsl_pool will not be held.
 */
int
dmu_objset_own(const char *name, dmu_objset_type_t type,
    boolean_t readonly, void *tag, objset_t **osp)
{
	dsl_pool_t *dp;
	dsl_dataset_t *ds;
	int err;

	err = dsl_pool_hold(name, FTAG, &dp);
	if (err != 0)
		return (err);
	err = dsl_dataset_own(dp, name, tag, &ds);
	if (err != 0) {
		dsl_pool_rele(dp, FTAG);
		return (err);
	}
	err = dmu_objset_own_impl(ds, type, readonly, tag, osp);
	dsl_pool_rele(dp, FTAG);

<<<<<<< HEAD
=======
	if (err == 0 && dmu_objset_userobjspace_upgradable(*osp))
		dmu_objset_userobjspace_upgrade(*osp);

>>>>>>> temp
	return (err);
}

int
dmu_objset_own_obj(dsl_pool_t *dp, uint64_t obj, dmu_objset_type_t type,
    boolean_t readonly, void *tag, objset_t **osp)
{
	dsl_dataset_t *ds;
	int err;

	err = dsl_dataset_own_obj(dp, obj, tag, &ds);
	if (err != 0)
		return (err);

	return (dmu_objset_own_impl(ds, type, readonly, tag, osp));
}

void
dmu_objset_rele(objset_t *os, void *tag)
{
	dsl_pool_t *dp = dmu_objset_pool(os);
	dsl_dataset_rele(os->os_dsl_dataset, tag);
	dsl_pool_rele(dp, tag);
}

/*
 * When we are called, os MUST refer to an objset associated with a dataset
 * that is owned by 'tag'; that is, is held and long held by 'tag' and ds_owner
 * == tag.  We will then release and reacquire ownership of the dataset while
 * holding the pool config_rwlock to avoid intervening namespace or ownership
 * changes may occur.
 *
 * This exists solely to accommodate zfs_ioc_userspace_upgrade()'s desire to
 * release the hold on its dataset and acquire a new one on the dataset of the
 * same name so that it can be partially torn down and reconstructed.
 */
void
dmu_objset_refresh_ownership(objset_t *os, void *tag)
{
	dsl_pool_t *dp;
	dsl_dataset_t *ds, *newds;
<<<<<<< HEAD
	char name[MAXNAMELEN];
=======
	char name[ZFS_MAX_DATASET_NAME_LEN];
>>>>>>> temp

	ds = os->os_dsl_dataset;
	VERIFY3P(ds, !=, NULL);
	VERIFY3P(ds->ds_owner, ==, tag);
	VERIFY(dsl_dataset_long_held(ds));

	dsl_dataset_name(ds, name);
	dp = dmu_objset_pool(os);
	dsl_pool_config_enter(dp, FTAG);
	dmu_objset_disown(os, tag);
	VERIFY0(dsl_dataset_own(dp, name, tag, &newds));
	VERIFY3P(newds, ==, os->os_dsl_dataset);
	dsl_pool_config_exit(dp, FTAG);
}

void
dmu_objset_disown(objset_t *os, void *tag)
{
<<<<<<< HEAD
=======
	/*
	 * Stop upgrading thread
	 */
	dmu_objset_upgrade_stop(os);
>>>>>>> temp
	dsl_dataset_disown(os->os_dsl_dataset, tag);
}

void
dmu_objset_evict_dbufs(objset_t *os)
{
	dnode_t *dn_marker;
	dnode_t *dn;

	dn_marker = kmem_alloc(sizeof (dnode_t), KM_SLEEP);

	mutex_enter(&os->os_lock);
	dn = list_head(&os->os_dnodes);
	while (dn != NULL) {
		/*
		 * Skip dnodes without holds.  We have to do this dance
		 * because dnode_add_ref() only works if there is already a
		 * hold.  If the dnode has no holds, then it has no dbufs.
		 */
		if (dnode_add_ref(dn, FTAG)) {
			list_insert_after(&os->os_dnodes, dn, dn_marker);
			mutex_exit(&os->os_lock);

			dnode_evict_dbufs(dn);
			dnode_rele(dn, FTAG);

			mutex_enter(&os->os_lock);
			dn = list_next(&os->os_dnodes, dn_marker);
			list_remove(&os->os_dnodes, dn_marker);
		} else {
			dn = list_next(&os->os_dnodes, dn);
		}
	}
	mutex_exit(&os->os_lock);

	kmem_free(dn_marker, sizeof (dnode_t));

	if (DMU_USERUSED_DNODE(os) != NULL) {
		dnode_evict_dbufs(DMU_GROUPUSED_DNODE(os));
		dnode_evict_dbufs(DMU_USERUSED_DNODE(os));
	}
	dnode_evict_dbufs(DMU_META_DNODE(os));
}

/*
 * Objset eviction processing is split into into two pieces.
 * The first marks the objset as evicting, evicts any dbufs that
 * have a refcount of zero, and then queues up the objset for the
 * second phase of eviction.  Once os->os_dnodes has been cleared by
 * dnode_buf_pageout()->dnode_destroy(), the second phase is executed.
 * The second phase closes the special dnodes, dequeues the objset from
 * the list of those undergoing eviction, and finally frees the objset.
 *
 * NOTE: Due to asynchronous eviction processing (invocation of
 *       dnode_buf_pageout()), it is possible for the meta dnode for the
 *       objset to have no holds even though os->os_dnodes is not empty.
 */
void
dmu_objset_evict(objset_t *os)
{
	int t;

	dsl_dataset_t *ds = os->os_dsl_dataset;

	for (t = 0; t < TXG_SIZE; t++)
		ASSERT(!dmu_objset_is_dirty(os, t));

<<<<<<< HEAD
	if (ds) {
		if (!ds->ds_is_snapshot) {
			VERIFY0(dsl_prop_unregister(ds,
			    zfs_prop_to_name(ZFS_PROP_CHECKSUM),
			    checksum_changed_cb, os));
			VERIFY0(dsl_prop_unregister(ds,
			    zfs_prop_to_name(ZFS_PROP_COMPRESSION),
			    compression_changed_cb, os));
			VERIFY0(dsl_prop_unregister(ds,
			    zfs_prop_to_name(ZFS_PROP_COPIES),
			    copies_changed_cb, os));
			VERIFY0(dsl_prop_unregister(ds,
			    zfs_prop_to_name(ZFS_PROP_DEDUP),
			    dedup_changed_cb, os));
			VERIFY0(dsl_prop_unregister(ds,
			    zfs_prop_to_name(ZFS_PROP_LOGBIAS),
			    logbias_changed_cb, os));
			VERIFY0(dsl_prop_unregister(ds,
			    zfs_prop_to_name(ZFS_PROP_SYNC),
			    sync_changed_cb, os));
			VERIFY0(dsl_prop_unregister(ds,
			    zfs_prop_to_name(ZFS_PROP_REDUNDANT_METADATA),
			    redundant_metadata_changed_cb, os));
			VERIFY0(dsl_prop_unregister(ds,
			    zfs_prop_to_name(ZFS_PROP_RECORDSIZE),
			    recordsize_changed_cb, os));
		}
		VERIFY0(dsl_prop_unregister(ds,
		    zfs_prop_to_name(ZFS_PROP_PRIMARYCACHE),
		    primary_cache_changed_cb, os));
		VERIFY0(dsl_prop_unregister(ds,
		    zfs_prop_to_name(ZFS_PROP_SECONDARYCACHE),
		    secondary_cache_changed_cb, os));
	}
=======
	if (ds)
		dsl_prop_unregister_all(ds, os);
>>>>>>> temp

	if (os->os_sa)
		sa_tear_down(os);

	dmu_objset_evict_dbufs(os);

	mutex_enter(&os->os_lock);
	spa_evicting_os_register(os->os_spa, os);
	if (list_is_empty(&os->os_dnodes)) {
		mutex_exit(&os->os_lock);
		dmu_objset_evict_done(os);
	} else {
		mutex_exit(&os->os_lock);
	}
}

void
dmu_objset_evict_done(objset_t *os)
{
	ASSERT3P(list_head(&os->os_dnodes), ==, NULL);

	dnode_special_close(&os->os_meta_dnode);
	if (DMU_USERUSED_DNODE(os)) {
		dnode_special_close(&os->os_userused_dnode);
		dnode_special_close(&os->os_groupused_dnode);
	}
	zil_free(os->os_zil);

<<<<<<< HEAD
	VERIFY(arc_buf_remove_ref(os->os_phys_buf, &os->os_phys_buf));
=======
	arc_buf_destroy(os->os_phys_buf, &os->os_phys_buf);
>>>>>>> temp

	/*
	 * This is a barrier to prevent the objset from going away in
	 * dnode_move() until we can safely ensure that the objset is still in
	 * use. We consider the objset valid before the barrier and invalid
	 * after the barrier.
	 */
	rw_enter(&os_lock, RW_READER);
	rw_exit(&os_lock);

<<<<<<< HEAD
	mutex_destroy(&os->os_lock);
	mutex_destroy(&os->os_obj_lock);
	mutex_destroy(&os->os_user_ptr_lock);
=======
	kmem_free(os->os_obj_next_percpu,
	    os->os_obj_next_percpu_len * sizeof (os->os_obj_next_percpu[0]));

	mutex_destroy(&os->os_lock);
	mutex_destroy(&os->os_userused_lock);
	mutex_destroy(&os->os_obj_lock);
	mutex_destroy(&os->os_user_ptr_lock);
	mutex_destroy(&os->os_upgrade_lock);
	for (int i = 0; i < TXG_SIZE; i++) {
		multilist_destroy(os->os_dirty_dnodes[i]);
	}
>>>>>>> temp
	spa_evicting_os_deregister(os->os_spa, os);
	kmem_free(os, sizeof (objset_t));
}

timestruc_t
dmu_objset_snap_cmtime(objset_t *os)
{
	return (dsl_dir_snap_cmtime(os->os_dsl_dataset->ds_dir));
}

/* called from dsl for meta-objset */
objset_t *
dmu_objset_create_impl(spa_t *spa, dsl_dataset_t *ds, blkptr_t *bp,
    dmu_objset_type_t type, dmu_tx_t *tx)
{
	objset_t *os;
	dnode_t *mdn;

	ASSERT(dmu_tx_is_syncing(tx));

	if (ds != NULL)
		VERIFY0(dmu_objset_from_ds(ds, &os));
	else
		VERIFY0(dmu_objset_open_impl(spa, NULL, bp, &os));

	mdn = DMU_META_DNODE(os);

<<<<<<< HEAD
	dnode_allocate(mdn, DMU_OT_DNODE, 1 << DNODE_BLOCK_SHIFT,
	    DN_MAX_INDBLKSHIFT, DMU_OT_NONE, 0, tx);
=======
	dnode_allocate(mdn, DMU_OT_DNODE, DNODE_BLOCK_SIZE, DN_MAX_INDBLKSHIFT,
	    DMU_OT_NONE, 0, DNODE_MIN_SLOTS, tx);
>>>>>>> temp

	/*
	 * We don't want to have to increase the meta-dnode's nlevels
	 * later, because then we could do it in quescing context while
	 * we are also accessing it in open context.
	 *
	 * This precaution is not necessary for the MOS (ds == NULL),
	 * because the MOS is only updated in syncing context.
	 * This is most fortunate: the MOS is the only objset that
	 * needs to be synced multiple times as spa_sync() iterates
	 * to convergence, so minimizing its dn_nlevels matters.
	 */
	if (ds != NULL) {
		int levels = 1;

		/*
		 * Determine the number of levels necessary for the meta-dnode
<<<<<<< HEAD
		 * to contain DN_MAX_OBJECT dnodes.
		 */
		while ((uint64_t)mdn->dn_nblkptr << (mdn->dn_datablkshift +
		    (levels - 1) * (mdn->dn_indblkshift - SPA_BLKPTRSHIFT)) <
		    DN_MAX_OBJECT * sizeof (dnode_phys_t))
=======
		 * to contain DN_MAX_OBJECT dnodes.  Note that in order to
		 * ensure that we do not overflow 64 bits, there has to be
		 * a nlevels that gives us a number of blocks > DN_MAX_OBJECT
		 * but < 2^64.  Therefore,
		 * (mdn->dn_indblkshift - SPA_BLKPTRSHIFT) (10) must be
		 * less than (64 - log2(DN_MAX_OBJECT)) (16).
		 */
		while ((uint64_t)mdn->dn_nblkptr <<
		    (mdn->dn_datablkshift - DNODE_SHIFT +
		    (levels - 1) * (mdn->dn_indblkshift - SPA_BLKPTRSHIFT)) <
		    DN_MAX_OBJECT)
>>>>>>> temp
			levels++;

		mdn->dn_next_nlevels[tx->tx_txg & TXG_MASK] =
		    mdn->dn_nlevels = levels;
	}

	ASSERT(type != DMU_OST_NONE);
	ASSERT(type != DMU_OST_ANY);
	ASSERT(type < DMU_OST_NUMTYPES);
	os->os_phys->os_type = type;
	if (dmu_objset_userused_enabled(os)) {
		os->os_phys->os_flags |= OBJSET_FLAG_USERACCOUNTING_COMPLETE;
<<<<<<< HEAD
=======
		if (dmu_objset_userobjused_enabled(os)) {
			ds->ds_feature_activation_needed[
			    SPA_FEATURE_USEROBJ_ACCOUNTING] = B_TRUE;
			os->os_phys->os_flags |=
			    OBJSET_FLAG_USEROBJACCOUNTING_COMPLETE;
		}
>>>>>>> temp
		os->os_flags = os->os_phys->os_flags;
	}

	dsl_dataset_dirty(ds, tx);

	return (os);
}

typedef struct dmu_objset_create_arg {
	const char *doca_name;
	cred_t *doca_cred;
	void (*doca_userfunc)(objset_t *os, void *arg,
	    cred_t *cr, dmu_tx_t *tx);
	void *doca_userarg;
	dmu_objset_type_t doca_type;
	uint64_t doca_flags;
} dmu_objset_create_arg_t;

/*ARGSUSED*/
static int
dmu_objset_create_check(void *arg, dmu_tx_t *tx)
{
	dmu_objset_create_arg_t *doca = arg;
	dsl_pool_t *dp = dmu_tx_pool(tx);
	dsl_dir_t *pdd;
	const char *tail;
	int error;

	if (strchr(doca->doca_name, '@') != NULL)
		return (SET_ERROR(EINVAL));

<<<<<<< HEAD
=======
	if (strlen(doca->doca_name) >= ZFS_MAX_DATASET_NAME_LEN)
		return (SET_ERROR(ENAMETOOLONG));

>>>>>>> temp
	error = dsl_dir_hold(dp, doca->doca_name, FTAG, &pdd, &tail);
	if (error != 0)
		return (error);
	if (tail == NULL) {
		dsl_dir_rele(pdd, FTAG);
		return (SET_ERROR(EEXIST));
	}
	error = dsl_fs_ss_limit_check(pdd, 1, ZFS_PROP_FILESYSTEM_LIMIT, NULL,
	    doca->doca_cred);
	dsl_dir_rele(pdd, FTAG);

	return (error);
}

static void
dmu_objset_create_sync(void *arg, dmu_tx_t *tx)
{
	dmu_objset_create_arg_t *doca = arg;
	dsl_pool_t *dp = dmu_tx_pool(tx);
	dsl_dir_t *pdd;
	const char *tail;
	dsl_dataset_t *ds;
	uint64_t obj;
	blkptr_t *bp;
	objset_t *os;

	VERIFY0(dsl_dir_hold(dp, doca->doca_name, FTAG, &pdd, &tail));

	obj = dsl_dataset_create_sync(pdd, tail, NULL, doca->doca_flags,
	    doca->doca_cred, tx);

	VERIFY0(dsl_dataset_hold_obj(pdd->dd_pool, obj, FTAG, &ds));
<<<<<<< HEAD
	bp = dsl_dataset_get_blkptr(ds);
	os = dmu_objset_create_impl(pdd->dd_pool->dp_spa,
	    ds, bp, doca->doca_type, tx);
=======
	rrw_enter(&ds->ds_bp_rwlock, RW_READER, FTAG);
	bp = dsl_dataset_get_blkptr(ds);
	os = dmu_objset_create_impl(pdd->dd_pool->dp_spa,
	    ds, bp, doca->doca_type, tx);
	rrw_exit(&ds->ds_bp_rwlock, FTAG);
>>>>>>> temp

	if (doca->doca_userfunc != NULL) {
		doca->doca_userfunc(os, doca->doca_userarg,
		    doca->doca_cred, tx);
	}

	spa_history_log_internal_ds(ds, "create", tx, "");
	zvol_create_minors(dp->dp_spa, doca->doca_name, B_TRUE);

	dsl_dataset_rele(ds, FTAG);
	dsl_dir_rele(pdd, FTAG);
}

int
dmu_objset_create(const char *name, dmu_objset_type_t type, uint64_t flags,
    void (*func)(objset_t *os, void *arg, cred_t *cr, dmu_tx_t *tx), void *arg)
{
	dmu_objset_create_arg_t doca;

	doca.doca_name = name;
	doca.doca_cred = CRED();
	doca.doca_flags = flags;
	doca.doca_userfunc = func;
	doca.doca_userarg = arg;
	doca.doca_type = type;

	return (dsl_sync_task(name,
	    dmu_objset_create_check, dmu_objset_create_sync, &doca,
	    5, ZFS_SPACE_CHECK_NORMAL));
}

typedef struct dmu_objset_clone_arg {
	const char *doca_clone;
	const char *doca_origin;
	cred_t *doca_cred;
} dmu_objset_clone_arg_t;

/*ARGSUSED*/
static int
dmu_objset_clone_check(void *arg, dmu_tx_t *tx)
{
	dmu_objset_clone_arg_t *doca = arg;
	dsl_dir_t *pdd;
	const char *tail;
	int error;
	dsl_dataset_t *origin;
	dsl_pool_t *dp = dmu_tx_pool(tx);

	if (strchr(doca->doca_clone, '@') != NULL)
		return (SET_ERROR(EINVAL));

<<<<<<< HEAD
=======
	if (strlen(doca->doca_clone) >= ZFS_MAX_DATASET_NAME_LEN)
		return (SET_ERROR(ENAMETOOLONG));

>>>>>>> temp
	error = dsl_dir_hold(dp, doca->doca_clone, FTAG, &pdd, &tail);
	if (error != 0)
		return (error);
	if (tail == NULL) {
		dsl_dir_rele(pdd, FTAG);
		return (SET_ERROR(EEXIST));
	}

	error = dsl_fs_ss_limit_check(pdd, 1, ZFS_PROP_FILESYSTEM_LIMIT, NULL,
	    doca->doca_cred);
	if (error != 0) {
		dsl_dir_rele(pdd, FTAG);
		return (SET_ERROR(EDQUOT));
	}
	dsl_dir_rele(pdd, FTAG);

	error = dsl_dataset_hold(dp, doca->doca_origin, FTAG, &origin);
	if (error != 0)
		return (error);

	/* You can only clone snapshots, not the head datasets. */
	if (!origin->ds_is_snapshot) {
		dsl_dataset_rele(origin, FTAG);
		return (SET_ERROR(EINVAL));
	}
	dsl_dataset_rele(origin, FTAG);

	return (0);
}

static void
dmu_objset_clone_sync(void *arg, dmu_tx_t *tx)
{
	dmu_objset_clone_arg_t *doca = arg;
	dsl_pool_t *dp = dmu_tx_pool(tx);
	dsl_dir_t *pdd;
	const char *tail;
	dsl_dataset_t *origin, *ds;
	uint64_t obj;
<<<<<<< HEAD
	char namebuf[MAXNAMELEN];
=======
	char namebuf[ZFS_MAX_DATASET_NAME_LEN];
>>>>>>> temp

	VERIFY0(dsl_dir_hold(dp, doca->doca_clone, FTAG, &pdd, &tail));
	VERIFY0(dsl_dataset_hold(dp, doca->doca_origin, FTAG, &origin));

	obj = dsl_dataset_create_sync(pdd, tail, origin, 0,
	    doca->doca_cred, tx);

	VERIFY0(dsl_dataset_hold_obj(pdd->dd_pool, obj, FTAG, &ds));
	dsl_dataset_name(origin, namebuf);
	spa_history_log_internal_ds(ds, "clone", tx,
	    "origin=%s (%llu)", namebuf, origin->ds_object);
	zvol_create_minors(dp->dp_spa, doca->doca_clone, B_TRUE);
	dsl_dataset_rele(ds, FTAG);
	dsl_dataset_rele(origin, FTAG);
	dsl_dir_rele(pdd, FTAG);
}

int
dmu_objset_clone(const char *clone, const char *origin)
{
	dmu_objset_clone_arg_t doca;

	doca.doca_clone = clone;
	doca.doca_origin = origin;
	doca.doca_cred = CRED();

	return (dsl_sync_task(clone,
	    dmu_objset_clone_check, dmu_objset_clone_sync, &doca,
	    5, ZFS_SPACE_CHECK_NORMAL));
}

int
dmu_objset_snapshot_one(const char *fsname, const char *snapname)
{
	int err;
	char *longsnap = kmem_asprintf("%s@%s", fsname, snapname);
	nvlist_t *snaps = fnvlist_alloc();

	fnvlist_add_boolean(snaps, longsnap);
	strfree(longsnap);
	err = dsl_dataset_snapshot(snaps, NULL, NULL);
	fnvlist_free(snaps);
	return (err);
}

static void
<<<<<<< HEAD
dmu_objset_sync_dnodes(list_t *list, list_t *newlist, dmu_tx_t *tx)
{
	dnode_t *dn;

	while ((dn = list_head(list))) {
=======
dmu_objset_upgrade_task_cb(void *data)
{
	objset_t *os = data;

	mutex_enter(&os->os_upgrade_lock);
	os->os_upgrade_status = EINTR;
	if (!os->os_upgrade_exit) {
		mutex_exit(&os->os_upgrade_lock);

		os->os_upgrade_status = os->os_upgrade_cb(os);
		mutex_enter(&os->os_upgrade_lock);
	}
	os->os_upgrade_exit = B_TRUE;
	os->os_upgrade_id = 0;
	mutex_exit(&os->os_upgrade_lock);
}

static void
dmu_objset_upgrade(objset_t *os, dmu_objset_upgrade_cb_t cb)
{
	if (os->os_upgrade_id != 0)
		return;

	mutex_enter(&os->os_upgrade_lock);
	if (os->os_upgrade_id == 0 && os->os_upgrade_status == 0) {
		os->os_upgrade_exit = B_FALSE;
		os->os_upgrade_cb = cb;
		os->os_upgrade_id = taskq_dispatch(
		    os->os_spa->spa_upgrade_taskq,
		    dmu_objset_upgrade_task_cb, os, TQ_SLEEP);
		if (os->os_upgrade_id == TASKQID_INVALID)
			os->os_upgrade_status = ENOMEM;
	}
	mutex_exit(&os->os_upgrade_lock);
}

static void
dmu_objset_upgrade_stop(objset_t *os)
{
	mutex_enter(&os->os_upgrade_lock);
	os->os_upgrade_exit = B_TRUE;
	if (os->os_upgrade_id != 0) {
		taskqid_t id = os->os_upgrade_id;

		os->os_upgrade_id = 0;
		mutex_exit(&os->os_upgrade_lock);

		taskq_cancel_id(os->os_spa->spa_upgrade_taskq, id);
	} else {
		mutex_exit(&os->os_upgrade_lock);
	}
}

static void
dmu_objset_sync_dnodes(multilist_sublist_t *list, dmu_tx_t *tx)
{
	dnode_t *dn;

	while ((dn = multilist_sublist_head(list)) != NULL) {
>>>>>>> temp
		ASSERT(dn->dn_object != DMU_META_DNODE_OBJECT);
		ASSERT(dn->dn_dbuf->db_data_pending);
		/*
		 * Initialize dn_zio outside dnode_sync() because the
		 * meta-dnode needs to set it ouside dnode_sync().
		 */
		dn->dn_zio = dn->dn_dbuf->db_data_pending->dr_zio;
		ASSERT(dn->dn_zio);

		ASSERT3U(dn->dn_nlevels, <=, DN_MAX_LEVELS);
<<<<<<< HEAD
		list_remove(list, dn);

		if (newlist) {
			(void) dnode_add_ref(dn, newlist);
			list_insert_tail(newlist, dn);
=======
		multilist_sublist_remove(list, dn);

		multilist_t *newlist = dn->dn_objset->os_synced_dnodes;
		if (newlist != NULL) {
			(void) dnode_add_ref(dn, newlist);
			multilist_insert(newlist, dn);
>>>>>>> temp
		}

		dnode_sync(dn, tx);
	}
}

/* ARGSUSED */
static void
dmu_objset_write_ready(zio_t *zio, arc_buf_t *abuf, void *arg)
{
	int i;

	blkptr_t *bp = zio->io_bp;
	objset_t *os = arg;
	dnode_phys_t *dnp = &os->os_phys->os_meta_dnode;

	ASSERT(!BP_IS_EMBEDDED(bp));
<<<<<<< HEAD
	ASSERT3P(bp, ==, os->os_rootbp);
=======
>>>>>>> temp
	ASSERT3U(BP_GET_TYPE(bp), ==, DMU_OT_OBJSET);
	ASSERT0(BP_GET_LEVEL(bp));

	/*
	 * Update rootbp fill count: it should be the number of objects
	 * allocated in the object set (not counting the "special"
	 * objects that are stored in the objset_phys_t -- the meta
	 * dnode and user/group accounting objects).
	 */
	bp->blk_fill = 0;
	for (i = 0; i < dnp->dn_nblkptr; i++)
		bp->blk_fill += BP_GET_FILL(&dnp->dn_blkptr[i]);
<<<<<<< HEAD
=======
	if (os->os_dsl_dataset != NULL)
		rrw_enter(&os->os_dsl_dataset->ds_bp_rwlock, RW_WRITER, FTAG);
	*os->os_rootbp = *bp;
	if (os->os_dsl_dataset != NULL)
		rrw_exit(&os->os_dsl_dataset->ds_bp_rwlock, FTAG);
>>>>>>> temp
}

/* ARGSUSED */
static void
dmu_objset_write_done(zio_t *zio, arc_buf_t *abuf, void *arg)
{
	blkptr_t *bp = zio->io_bp;
	blkptr_t *bp_orig = &zio->io_bp_orig;
	objset_t *os = arg;

	if (zio->io_flags & ZIO_FLAG_IO_REWRITE) {
		ASSERT(BP_EQUAL(bp, bp_orig));
	} else {
		dsl_dataset_t *ds = os->os_dsl_dataset;
		dmu_tx_t *tx = os->os_synctx;

		(void) dsl_dataset_block_kill(ds, bp_orig, tx, B_TRUE);
		dsl_dataset_block_born(ds, bp, tx);
	}
<<<<<<< HEAD
}

=======
	kmem_free(bp, sizeof (*bp));
}

typedef struct sync_dnodes_arg {
	multilist_t *sda_list;
	int sda_sublist_idx;
	multilist_t *sda_newlist;
	dmu_tx_t *sda_tx;
} sync_dnodes_arg_t;

static void
sync_dnodes_task(void *arg)
{
	sync_dnodes_arg_t *sda = arg;

	multilist_sublist_t *ms =
	    multilist_sublist_lock(sda->sda_list, sda->sda_sublist_idx);

	dmu_objset_sync_dnodes(ms, sda->sda_tx);

	multilist_sublist_unlock(ms);

	kmem_free(sda, sizeof (*sda));
}


>>>>>>> temp
/* called from dsl */
void
dmu_objset_sync(objset_t *os, zio_t *pio, dmu_tx_t *tx)
{
	int txgoff;
	zbookmark_phys_t zb;
	zio_prop_t zp;
	zio_t *zio;
	list_t *list;
<<<<<<< HEAD
	list_t *newlist = NULL;
	dbuf_dirty_record_t *dr;
=======
	dbuf_dirty_record_t *dr;
	blkptr_t *blkptr_copy = kmem_alloc(sizeof (*os->os_rootbp), KM_SLEEP);
	*blkptr_copy = *os->os_rootbp;
>>>>>>> temp

	dprintf_ds(os->os_dsl_dataset, "txg=%llu\n", tx->tx_txg);

	ASSERT(dmu_tx_is_syncing(tx));
	/* XXX the write_done callback should really give us the tx... */
	os->os_synctx = tx;

	if (os->os_dsl_dataset == NULL) {
		/*
		 * This is the MOS.  If we have upgraded,
		 * spa_max_replication() could change, so reset
		 * os_copies here.
		 */
		os->os_copies = spa_max_replication(os->os_spa);
	}

	/*
	 * Create the root block IO
	 */
	SET_BOOKMARK(&zb, os->os_dsl_dataset ?
	    os->os_dsl_dataset->ds_object : DMU_META_OBJSET,
	    ZB_ROOT_OBJECT, ZB_ROOT_LEVEL, ZB_ROOT_BLKID);
	arc_release(os->os_phys_buf, &os->os_phys_buf);

	dmu_write_policy(os, NULL, 0, 0, &zp);

	zio = arc_write(pio, os->os_spa, tx->tx_txg,
<<<<<<< HEAD
	    os->os_rootbp, os->os_phys_buf, DMU_OS_IS_L2CACHEABLE(os),
	    DMU_OS_IS_L2COMPRESSIBLE(os), &zp, dmu_objset_write_ready,
	    NULL, dmu_objset_write_done, os, ZIO_PRIORITY_ASYNC_WRITE,
	    ZIO_FLAG_MUSTSUCCEED, &zb);
=======
	    blkptr_copy, os->os_phys_buf, DMU_OS_IS_L2CACHEABLE(os),
	    &zp, dmu_objset_write_ready, NULL, NULL, dmu_objset_write_done,
	    os, ZIO_PRIORITY_ASYNC_WRITE, ZIO_FLAG_MUSTSUCCEED, &zb);
>>>>>>> temp

	/*
	 * Sync special dnodes - the parent IO for the sync is the root block
	 */
	DMU_META_DNODE(os)->dn_zio = zio;
	dnode_sync(DMU_META_DNODE(os), tx);

	os->os_phys->os_flags = os->os_flags;

	if (DMU_USERUSED_DNODE(os) &&
	    DMU_USERUSED_DNODE(os)->dn_type != DMU_OT_NONE) {
		DMU_USERUSED_DNODE(os)->dn_zio = zio;
		dnode_sync(DMU_USERUSED_DNODE(os), tx);
		DMU_GROUPUSED_DNODE(os)->dn_zio = zio;
		dnode_sync(DMU_GROUPUSED_DNODE(os), tx);
	}

	txgoff = tx->tx_txg & TXG_MASK;

	if (dmu_objset_userused_enabled(os)) {
<<<<<<< HEAD
		newlist = &os->os_synced_dnodes;
		/*
		 * We must create the list here because it uses the
		 * dn_dirty_link[] of this txg.
		 */
		list_create(newlist, sizeof (dnode_t),
		    offsetof(dnode_t, dn_dirty_link[txgoff]));
	}

	dmu_objset_sync_dnodes(&os->os_free_dnodes[txgoff], newlist, tx);
	dmu_objset_sync_dnodes(&os->os_dirty_dnodes[txgoff], newlist, tx);

	list = &DMU_META_DNODE(os)->dn_dirty_records[txgoff];
	while ((dr = list_head(list))) {
=======
		/*
		 * We must create the list here because it uses the
		 * dn_dirty_link[] of this txg.  But it may already
		 * exist because we call dsl_dataset_sync() twice per txg.
		 */
		if (os->os_synced_dnodes == NULL) {
			os->os_synced_dnodes =
			    multilist_create(sizeof (dnode_t),
			    offsetof(dnode_t, dn_dirty_link[txgoff]),
			    dnode_multilist_index_func);
		} else {
			ASSERT3U(os->os_synced_dnodes->ml_offset, ==,
			    offsetof(dnode_t, dn_dirty_link[txgoff]));
		}
	}

	for (int i = 0;
	    i < multilist_get_num_sublists(os->os_dirty_dnodes[txgoff]); i++) {
		sync_dnodes_arg_t *sda = kmem_alloc(sizeof (*sda), KM_SLEEP);
		sda->sda_list = os->os_dirty_dnodes[txgoff];
		sda->sda_sublist_idx = i;
		sda->sda_tx = tx;
		(void) taskq_dispatch(dmu_objset_pool(os)->dp_sync_taskq,
		    sync_dnodes_task, sda, 0);
		/* callback frees sda */
	}
	taskq_wait(dmu_objset_pool(os)->dp_sync_taskq);

	list = &DMU_META_DNODE(os)->dn_dirty_records[txgoff];
	while ((dr = list_head(list)) != NULL) {
>>>>>>> temp
		ASSERT0(dr->dr_dbuf->db_level);
		list_remove(list, dr);
		if (dr->dr_zio)
			zio_nowait(dr->dr_zio);
	}
<<<<<<< HEAD
=======

	/* Enable dnode backfill if enough objects have been freed. */
	if (os->os_freed_dnodes >= dmu_rescan_dnode_threshold) {
		os->os_rescan_dnodes = B_TRUE;
		os->os_freed_dnodes = 0;
	}

>>>>>>> temp
	/*
	 * Free intent log blocks up to this tx.
	 */
	zil_sync(os->os_zil, tx);
	os->os_phys->os_zil_header = os->os_zil_header;
	zio_nowait(zio);
}

boolean_t
dmu_objset_is_dirty(objset_t *os, uint64_t txg)
{
<<<<<<< HEAD
	return (!list_is_empty(&os->os_dirty_dnodes[txg & TXG_MASK]) ||
	    !list_is_empty(&os->os_free_dnodes[txg & TXG_MASK]));
=======
	return (!multilist_is_empty(os->os_dirty_dnodes[txg & TXG_MASK]));
>>>>>>> temp
}

static objset_used_cb_t *used_cbs[DMU_OST_NUMTYPES];

void
dmu_objset_register_type(dmu_objset_type_t ost, objset_used_cb_t *cb)
{
	used_cbs[ost] = cb;
}

boolean_t
dmu_objset_userused_enabled(objset_t *os)
{
	return (spa_version(os->os_spa) >= SPA_VERSION_USERSPACE &&
	    used_cbs[os->os_phys->os_type] != NULL &&
	    DMU_USERUSED_DNODE(os) != NULL);
}

<<<<<<< HEAD
static void
do_userquota_update(objset_t *os, uint64_t used, uint64_t flags,
    uint64_t user, uint64_t group, boolean_t subtract, dmu_tx_t *tx)
{
	if ((flags & DNODE_FLAG_USERUSED_ACCOUNTED)) {
		int64_t delta = DNODE_SIZE + used;
		if (subtract)
			delta = -delta;
		VERIFY3U(0, ==, zap_increment_int(os, DMU_USERUSED_OBJECT,
		    user, delta, tx));
		VERIFY3U(0, ==, zap_increment_int(os, DMU_GROUPUSED_OBJECT,
		    group, delta, tx));
	}
}

void
dmu_objset_do_userquota_updates(objset_t *os, dmu_tx_t *tx)
{
	dnode_t *dn;
	list_t *list = &os->os_synced_dnodes;

	ASSERT(list_head(list) == NULL || dmu_objset_userused_enabled(os));

	while ((dn = list_head(list))) {
=======
boolean_t
dmu_objset_userobjused_enabled(objset_t *os)
{
	return (dmu_objset_userused_enabled(os) &&
	    spa_feature_is_enabled(os->os_spa, SPA_FEATURE_USEROBJ_ACCOUNTING));
}

typedef struct userquota_node {
	/* must be in the first filed, see userquota_update_cache() */
	char		uqn_id[20 + DMU_OBJACCT_PREFIX_LEN];
	int64_t		uqn_delta;
	avl_node_t	uqn_node;
} userquota_node_t;

typedef struct userquota_cache {
	avl_tree_t uqc_user_deltas;
	avl_tree_t uqc_group_deltas;
} userquota_cache_t;

static int
userquota_compare(const void *l, const void *r)
{
	const userquota_node_t *luqn = l;
	const userquota_node_t *ruqn = r;
	int rv;

	/*
	 * NB: can only access uqn_id because userquota_update_cache() doesn't
	 * pass in an entire userquota_node_t.
	 */
	rv = strcmp(luqn->uqn_id, ruqn->uqn_id);

	return (AVL_ISIGN(rv));
}

static void
do_userquota_cacheflush(objset_t *os, userquota_cache_t *cache, dmu_tx_t *tx)
{
	void *cookie;
	userquota_node_t *uqn;

	ASSERT(dmu_tx_is_syncing(tx));

	cookie = NULL;
	while ((uqn = avl_destroy_nodes(&cache->uqc_user_deltas,
	    &cookie)) != NULL) {
		/*
		 * os_userused_lock protects against concurrent calls to
		 * zap_increment_int().  It's needed because zap_increment_int()
		 * is not thread-safe (i.e. not atomic).
		 */
		mutex_enter(&os->os_userused_lock);
		VERIFY0(zap_increment(os, DMU_USERUSED_OBJECT,
		    uqn->uqn_id, uqn->uqn_delta, tx));
		mutex_exit(&os->os_userused_lock);
		kmem_free(uqn, sizeof (*uqn));
	}
	avl_destroy(&cache->uqc_user_deltas);

	cookie = NULL;
	while ((uqn = avl_destroy_nodes(&cache->uqc_group_deltas,
	    &cookie)) != NULL) {
		mutex_enter(&os->os_userused_lock);
		VERIFY0(zap_increment(os, DMU_GROUPUSED_OBJECT,
		    uqn->uqn_id, uqn->uqn_delta, tx));
		mutex_exit(&os->os_userused_lock);
		kmem_free(uqn, sizeof (*uqn));
	}
	avl_destroy(&cache->uqc_group_deltas);
}

static void
userquota_update_cache(avl_tree_t *avl, const char *id, int64_t delta)
{
	userquota_node_t *uqn;
	avl_index_t idx;

	ASSERT(strlen(id) < sizeof (uqn->uqn_id));
	/*
	 * Use id directly for searching because uqn_id is the first field of
	 * userquota_node_t and fields after uqn_id won't be accessed in
	 * avl_find().
	 */
	uqn = avl_find(avl, (const void *)id, &idx);
	if (uqn == NULL) {
		uqn = kmem_zalloc(sizeof (*uqn), KM_SLEEP);
		strlcpy(uqn->uqn_id, id, sizeof (uqn->uqn_id));
		avl_insert(avl, uqn, idx);
	}
	uqn->uqn_delta += delta;
}

static void
do_userquota_update(userquota_cache_t *cache, uint64_t used, uint64_t flags,
    uint64_t user, uint64_t group, boolean_t subtract)
{
	if ((flags & DNODE_FLAG_USERUSED_ACCOUNTED)) {
		int64_t delta = DNODE_MIN_SIZE + used;
		char name[20];

		if (subtract)
			delta = -delta;

		(void) sprintf(name, "%llx", (longlong_t)user);
		userquota_update_cache(&cache->uqc_user_deltas, name, delta);

		(void) sprintf(name, "%llx", (longlong_t)group);
		userquota_update_cache(&cache->uqc_group_deltas, name, delta);
	}
}

static void
do_userobjquota_update(userquota_cache_t *cache, uint64_t flags,
    uint64_t user, uint64_t group, boolean_t subtract)
{
	if (flags & DNODE_FLAG_USEROBJUSED_ACCOUNTED) {
		char name[20 + DMU_OBJACCT_PREFIX_LEN];
		int delta = subtract ? -1 : 1;

		(void) snprintf(name, sizeof (name), DMU_OBJACCT_PREFIX "%llx",
		    (longlong_t)user);
		userquota_update_cache(&cache->uqc_user_deltas, name, delta);

		(void) snprintf(name, sizeof (name), DMU_OBJACCT_PREFIX "%llx",
		    (longlong_t)group);
		userquota_update_cache(&cache->uqc_group_deltas, name, delta);
	}
}

typedef struct userquota_updates_arg {
	objset_t *uua_os;
	int uua_sublist_idx;
	dmu_tx_t *uua_tx;
} userquota_updates_arg_t;

static void
userquota_updates_task(void *arg)
{
	userquota_updates_arg_t *uua = arg;
	objset_t *os = uua->uua_os;
	dmu_tx_t *tx = uua->uua_tx;
	dnode_t *dn;
	userquota_cache_t cache = { { 0 } };

	multilist_sublist_t *list =
	    multilist_sublist_lock(os->os_synced_dnodes, uua->uua_sublist_idx);

	ASSERT(multilist_sublist_head(list) == NULL ||
	    dmu_objset_userused_enabled(os));
	avl_create(&cache.uqc_user_deltas, userquota_compare,
	    sizeof (userquota_node_t), offsetof(userquota_node_t, uqn_node));
	avl_create(&cache.uqc_group_deltas, userquota_compare,
	    sizeof (userquota_node_t), offsetof(userquota_node_t, uqn_node));

	while ((dn = multilist_sublist_head(list)) != NULL) {
>>>>>>> temp
		int flags;
		ASSERT(!DMU_OBJECT_IS_SPECIAL(dn->dn_object));
		ASSERT(dn->dn_phys->dn_type == DMU_OT_NONE ||
		    dn->dn_phys->dn_flags &
		    DNODE_FLAG_USERUSED_ACCOUNTED);

<<<<<<< HEAD
		/* Allocate the user/groupused objects if necessary. */
		if (DMU_USERUSED_DNODE(os)->dn_type == DMU_OT_NONE) {
			VERIFY(0 == zap_create_claim(os,
			    DMU_USERUSED_OBJECT,
			    DMU_OT_USERGROUP_USED, DMU_OT_NONE, 0, tx));
			VERIFY(0 == zap_create_claim(os,
			    DMU_GROUPUSED_OBJECT,
			    DMU_OT_USERGROUP_USED, DMU_OT_NONE, 0, tx));
		}

		/*
		 * We intentionally modify the zap object even if the
		 * net delta is zero.  Otherwise
		 * the block of the zap obj could be shared between
		 * datasets but need to be different between them after
		 * a bprewrite.
		 */

		flags = dn->dn_id_flags;
		ASSERT(flags);
		if (flags & DN_ID_OLD_EXIST)  {
			do_userquota_update(os, dn->dn_oldused, dn->dn_oldflags,
			    dn->dn_olduid, dn->dn_oldgid, B_TRUE, tx);
		}
		if (flags & DN_ID_NEW_EXIST) {
			do_userquota_update(os, DN_USED_BYTES(dn->dn_phys),
			    dn->dn_phys->dn_flags,  dn->dn_newuid,
			    dn->dn_newgid, B_FALSE, tx);
=======
		flags = dn->dn_id_flags;
		ASSERT(flags);
		if (flags & DN_ID_OLD_EXIST)  {
			do_userquota_update(&cache,
			    dn->dn_oldused, dn->dn_oldflags,
			    dn->dn_olduid, dn->dn_oldgid, B_TRUE);
			do_userobjquota_update(&cache, dn->dn_oldflags,
			    dn->dn_olduid, dn->dn_oldgid, B_TRUE);
		}
		if (flags & DN_ID_NEW_EXIST) {
			do_userquota_update(&cache,
			    DN_USED_BYTES(dn->dn_phys), dn->dn_phys->dn_flags,
			    dn->dn_newuid, dn->dn_newgid, B_FALSE);
			do_userobjquota_update(&cache, dn->dn_phys->dn_flags,
			    dn->dn_newuid, dn->dn_newgid, B_FALSE);
>>>>>>> temp
		}

		mutex_enter(&dn->dn_mtx);
		dn->dn_oldused = 0;
		dn->dn_oldflags = 0;
		if (dn->dn_id_flags & DN_ID_NEW_EXIST) {
			dn->dn_olduid = dn->dn_newuid;
			dn->dn_oldgid = dn->dn_newgid;
			dn->dn_id_flags |= DN_ID_OLD_EXIST;
			if (dn->dn_bonuslen == 0)
				dn->dn_id_flags |= DN_ID_CHKED_SPILL;
			else
				dn->dn_id_flags |= DN_ID_CHKED_BONUS;
		}
		dn->dn_id_flags &= ~(DN_ID_NEW_EXIST);
		mutex_exit(&dn->dn_mtx);

<<<<<<< HEAD
		list_remove(list, dn);
		dnode_rele(dn, list);
=======
		multilist_sublist_remove(list, dn);
		dnode_rele(dn, os->os_synced_dnodes);
	}
	do_userquota_cacheflush(os, &cache, tx);
	multilist_sublist_unlock(list);
	kmem_free(uua, sizeof (*uua));
}

void
dmu_objset_do_userquota_updates(objset_t *os, dmu_tx_t *tx)
{
	if (!dmu_objset_userused_enabled(os))
		return;

	/* Allocate the user/groupused objects if necessary. */
	if (DMU_USERUSED_DNODE(os)->dn_type == DMU_OT_NONE) {
		VERIFY0(zap_create_claim(os,
		    DMU_USERUSED_OBJECT,
		    DMU_OT_USERGROUP_USED, DMU_OT_NONE, 0, tx));
		VERIFY0(zap_create_claim(os,
		    DMU_GROUPUSED_OBJECT,
		    DMU_OT_USERGROUP_USED, DMU_OT_NONE, 0, tx));
	}

	for (int i = 0;
	    i < multilist_get_num_sublists(os->os_synced_dnodes); i++) {
		userquota_updates_arg_t *uua =
		    kmem_alloc(sizeof (*uua), KM_SLEEP);
		uua->uua_os = os;
		uua->uua_sublist_idx = i;
		uua->uua_tx = tx;
		/* note: caller does taskq_wait() */
		(void) taskq_dispatch(dmu_objset_pool(os)->dp_sync_taskq,
		    userquota_updates_task, uua, 0);
		/* callback frees uua */
>>>>>>> temp
	}
}

/*
 * Returns a pointer to data to find uid/gid from
 *
 * If a dirty record for transaction group that is syncing can't
 * be found then NULL is returned.  In the NULL case it is assumed
 * the uid/gid aren't changing.
 */
static void *
dmu_objset_userquota_find_data(dmu_buf_impl_t *db, dmu_tx_t *tx)
{
	dbuf_dirty_record_t *dr, **drp;
	void *data;

	if (db->db_dirtycnt == 0)
		return (db->db.db_data);  /* Nothing is changing */

	for (drp = &db->db_last_dirty; (dr = *drp) != NULL; drp = &dr->dr_next)
		if (dr->dr_txg == tx->tx_txg)
			break;

	if (dr == NULL) {
		data = NULL;
	} else {
		dnode_t *dn;

		DB_DNODE_ENTER(dr->dr_dbuf);
		dn = DB_DNODE(dr->dr_dbuf);

		if (dn->dn_bonuslen == 0 &&
		    dr->dr_dbuf->db_blkid == DMU_SPILL_BLKID)
			data = dr->dt.dl.dr_data->b_data;
		else
			data = dr->dt.dl.dr_data;

		DB_DNODE_EXIT(dr->dr_dbuf);
	}

	return (data);
}

void
dmu_objset_userquota_get_ids(dnode_t *dn, boolean_t before, dmu_tx_t *tx)
{
	objset_t *os = dn->dn_objset;
	void *data = NULL;
	dmu_buf_impl_t *db = NULL;
	uint64_t *user = NULL;
	uint64_t *group = NULL;
	int flags = dn->dn_id_flags;
	int error;
	boolean_t have_spill = B_FALSE;

	if (!dmu_objset_userused_enabled(dn->dn_objset))
		return;

	if (before && (flags & (DN_ID_CHKED_BONUS|DN_ID_OLD_EXIST|
	    DN_ID_CHKED_SPILL)))
		return;

	if (before && dn->dn_bonuslen != 0)
		data = DN_BONUS(dn->dn_phys);
	else if (!before && dn->dn_bonuslen != 0) {
		if (dn->dn_bonus) {
			db = dn->dn_bonus;
			mutex_enter(&db->db_mtx);
			data = dmu_objset_userquota_find_data(db, tx);
		} else {
			data = DN_BONUS(dn->dn_phys);
		}
	} else if (dn->dn_bonuslen == 0 && dn->dn_bonustype == DMU_OT_SA) {
			int rf = 0;

			if (RW_WRITE_HELD(&dn->dn_struct_rwlock))
				rf |= DB_RF_HAVESTRUCT;
			error = dmu_spill_hold_by_dnode(dn,
			    rf | DB_RF_MUST_SUCCEED,
			    FTAG, (dmu_buf_t **)&db);
			ASSERT(error == 0);
			mutex_enter(&db->db_mtx);
			data = (before) ? db->db.db_data :
			    dmu_objset_userquota_find_data(db, tx);
			have_spill = B_TRUE;
	} else {
		mutex_enter(&dn->dn_mtx);
		dn->dn_id_flags |= DN_ID_CHKED_BONUS;
		mutex_exit(&dn->dn_mtx);
		return;
	}

	if (before) {
		ASSERT(data);
		user = &dn->dn_olduid;
		group = &dn->dn_oldgid;
	} else if (data) {
		user = &dn->dn_newuid;
		group = &dn->dn_newgid;
	}

	/*
	 * Must always call the callback in case the object
	 * type has changed and that type isn't an object type to track
	 */
	error = used_cbs[os->os_phys->os_type](dn->dn_bonustype, data,
	    user, group);

	/*
	 * Preserve existing uid/gid when the callback can't determine
	 * what the new uid/gid are and the callback returned EEXIST.
	 * The EEXIST error tells us to just use the existing uid/gid.
	 * If we don't know what the old values are then just assign
	 * them to 0, since that is a new file  being created.
	 */
	if (!before && data == NULL && error == EEXIST) {
		if (flags & DN_ID_OLD_EXIST) {
			dn->dn_newuid = dn->dn_olduid;
			dn->dn_newgid = dn->dn_oldgid;
		} else {
			dn->dn_newuid = 0;
			dn->dn_newgid = 0;
		}
		error = 0;
	}

	if (db)
		mutex_exit(&db->db_mtx);

	mutex_enter(&dn->dn_mtx);
	if (error == 0 && before)
		dn->dn_id_flags |= DN_ID_OLD_EXIST;
	if (error == 0 && !before)
		dn->dn_id_flags |= DN_ID_NEW_EXIST;

	if (have_spill) {
		dn->dn_id_flags |= DN_ID_CHKED_SPILL;
	} else {
		dn->dn_id_flags |= DN_ID_CHKED_BONUS;
	}
	mutex_exit(&dn->dn_mtx);
	if (have_spill)
		dmu_buf_rele((dmu_buf_t *)db, FTAG);
}

boolean_t
dmu_objset_userspace_present(objset_t *os)
{
	return (os->os_phys->os_flags &
	    OBJSET_FLAG_USERACCOUNTING_COMPLETE);
}

<<<<<<< HEAD
int
dmu_objset_userspace_upgrade(objset_t *os)
=======
boolean_t
dmu_objset_userobjspace_present(objset_t *os)
{
	return (os->os_phys->os_flags &
	    OBJSET_FLAG_USEROBJACCOUNTING_COMPLETE);
}

static int
dmu_objset_space_upgrade(objset_t *os)
>>>>>>> temp
{
	uint64_t obj;
	int err = 0;

<<<<<<< HEAD
	if (dmu_objset_userspace_present(os))
		return (0);
	if (!dmu_objset_userused_enabled(os))
		return (SET_ERROR(ENOTSUP));
	if (dmu_objset_is_snapshot(os))
		return (SET_ERROR(EINVAL));

=======
>>>>>>> temp
	/*
	 * We simply need to mark every object dirty, so that it will be
	 * synced out and now accounted.  If this is called
	 * concurrently, or if we already did some work before crashing,
	 * that's fine, since we track each object's accounted state
	 * independently.
	 */

	for (obj = 0; err == 0; err = dmu_object_next(os, &obj, FALSE, 0)) {
		dmu_tx_t *tx;
		dmu_buf_t *db;
		int objerr;

<<<<<<< HEAD
=======
		mutex_enter(&os->os_upgrade_lock);
		if (os->os_upgrade_exit)
			err = SET_ERROR(EINTR);
		mutex_exit(&os->os_upgrade_lock);
		if (err != 0)
			return (err);

>>>>>>> temp
		if (issig(JUSTLOOKING) && issig(FORREAL))
			return (SET_ERROR(EINTR));

		objerr = dmu_bonus_hold(os, obj, FTAG, &db);
		if (objerr != 0)
			continue;
		tx = dmu_tx_create(os);
		dmu_tx_hold_bonus(tx, obj);
		objerr = dmu_tx_assign(tx, TXG_WAIT);
		if (objerr != 0) {
<<<<<<< HEAD
=======
			dmu_buf_rele(db, FTAG);
>>>>>>> temp
			dmu_tx_abort(tx);
			continue;
		}
		dmu_buf_will_dirty(db, tx);
		dmu_buf_rele(db, FTAG);
		dmu_tx_commit(tx);
	}
<<<<<<< HEAD
=======
	return (0);
}

int
dmu_objset_userspace_upgrade(objset_t *os)
{
	int err = 0;

	if (dmu_objset_userspace_present(os))
		return (0);
	if (dmu_objset_is_snapshot(os))
		return (SET_ERROR(EINVAL));
	if (!dmu_objset_userused_enabled(os))
		return (SET_ERROR(ENOTSUP));

	err = dmu_objset_space_upgrade(os);
	if (err)
		return (err);
>>>>>>> temp

	os->os_flags |= OBJSET_FLAG_USERACCOUNTING_COMPLETE;
	txg_wait_synced(dmu_objset_pool(os), 0);
	return (0);
}

<<<<<<< HEAD
=======
static int
dmu_objset_userobjspace_upgrade_cb(objset_t *os)
{
	int err = 0;

	if (dmu_objset_userobjspace_present(os))
		return (0);
	if (dmu_objset_is_snapshot(os))
		return (SET_ERROR(EINVAL));
	if (!dmu_objset_userobjused_enabled(os))
		return (SET_ERROR(ENOTSUP));

	dmu_objset_ds(os)->ds_feature_activation_needed[
	    SPA_FEATURE_USEROBJ_ACCOUNTING] = B_TRUE;

	err = dmu_objset_space_upgrade(os);
	if (err)
		return (err);

	os->os_flags |= OBJSET_FLAG_USEROBJACCOUNTING_COMPLETE;
	txg_wait_synced(dmu_objset_pool(os), 0);
	return (0);
}

void
dmu_objset_userobjspace_upgrade(objset_t *os)
{
	dmu_objset_upgrade(os, dmu_objset_userobjspace_upgrade_cb);
}

boolean_t
dmu_objset_userobjspace_upgradable(objset_t *os)
{
	return (dmu_objset_type(os) == DMU_OST_ZFS &&
	    !dmu_objset_is_snapshot(os) &&
	    dmu_objset_userobjused_enabled(os) &&
	    !dmu_objset_userobjspace_present(os));
}

>>>>>>> temp
void
dmu_objset_space(objset_t *os, uint64_t *refdbytesp, uint64_t *availbytesp,
    uint64_t *usedobjsp, uint64_t *availobjsp)
{
	dsl_dataset_space(os->os_dsl_dataset, refdbytesp, availbytesp,
	    usedobjsp, availobjsp);
}

uint64_t
dmu_objset_fsid_guid(objset_t *os)
{
	return (dsl_dataset_fsid_guid(os->os_dsl_dataset));
}

void
dmu_objset_fast_stat(objset_t *os, dmu_objset_stats_t *stat)
{
	stat->dds_type = os->os_phys->os_type;
	if (os->os_dsl_dataset)
		dsl_dataset_fast_stat(os->os_dsl_dataset, stat);
}

void
dmu_objset_stats(objset_t *os, nvlist_t *nv)
{
	ASSERT(os->os_dsl_dataset ||
	    os->os_phys->os_type == DMU_OST_META);

	if (os->os_dsl_dataset != NULL)
		dsl_dataset_stats(os->os_dsl_dataset, nv);

	dsl_prop_nvlist_add_uint64(nv, ZFS_PROP_TYPE,
	    os->os_phys->os_type);
	dsl_prop_nvlist_add_uint64(nv, ZFS_PROP_USERACCOUNTING,
	    dmu_objset_userspace_present(os));
}

int
dmu_objset_is_snapshot(objset_t *os)
{
	if (os->os_dsl_dataset != NULL)
		return (os->os_dsl_dataset->ds_is_snapshot);
	else
		return (B_FALSE);
}

int
dmu_snapshot_realname(objset_t *os, char *name, char *real, int maxlen,
    boolean_t *conflict)
{
	dsl_dataset_t *ds = os->os_dsl_dataset;
	uint64_t ignored;

	if (dsl_dataset_phys(ds)->ds_snapnames_zapobj == 0)
		return (SET_ERROR(ENOENT));

	return (zap_lookup_norm(ds->ds_dir->dd_pool->dp_meta_objset,
	    dsl_dataset_phys(ds)->ds_snapnames_zapobj, name, 8, 1, &ignored,
<<<<<<< HEAD
	    MT_FIRST, real, maxlen, conflict));
=======
	    MT_NORMALIZE, real, maxlen, conflict));
>>>>>>> temp
}

int
dmu_snapshot_list_next(objset_t *os, int namelen, char *name,
    uint64_t *idp, uint64_t *offp, boolean_t *case_conflict)
{
	dsl_dataset_t *ds = os->os_dsl_dataset;
	zap_cursor_t cursor;
	zap_attribute_t attr;

	ASSERT(dsl_pool_config_held(dmu_objset_pool(os)));

	if (dsl_dataset_phys(ds)->ds_snapnames_zapobj == 0)
		return (SET_ERROR(ENOENT));

	zap_cursor_init_serialized(&cursor,
	    ds->ds_dir->dd_pool->dp_meta_objset,
	    dsl_dataset_phys(ds)->ds_snapnames_zapobj, *offp);

	if (zap_cursor_retrieve(&cursor, &attr) != 0) {
		zap_cursor_fini(&cursor);
		return (SET_ERROR(ENOENT));
	}

	if (strlen(attr.za_name) + 1 > namelen) {
		zap_cursor_fini(&cursor);
		return (SET_ERROR(ENAMETOOLONG));
	}

	(void) strcpy(name, attr.za_name);
	if (idp)
		*idp = attr.za_first_integer;
	if (case_conflict)
		*case_conflict = attr.za_normalization_conflict;
	zap_cursor_advance(&cursor);
	*offp = zap_cursor_serialize(&cursor);
	zap_cursor_fini(&cursor);

	return (0);
}

int
dmu_snapshot_lookup(objset_t *os, const char *name, uint64_t *value)
{
	return (dsl_dataset_snap_lookup(os->os_dsl_dataset, name, value));
}

int
dmu_dir_list_next(objset_t *os, int namelen, char *name,
    uint64_t *idp, uint64_t *offp)
{
	dsl_dir_t *dd = os->os_dsl_dataset->ds_dir;
	zap_cursor_t cursor;
	zap_attribute_t attr;

	/* there is no next dir on a snapshot! */
	if (os->os_dsl_dataset->ds_object !=
	    dsl_dir_phys(dd)->dd_head_dataset_obj)
		return (SET_ERROR(ENOENT));

	zap_cursor_init_serialized(&cursor,
	    dd->dd_pool->dp_meta_objset,
	    dsl_dir_phys(dd)->dd_child_dir_zapobj, *offp);

	if (zap_cursor_retrieve(&cursor, &attr) != 0) {
		zap_cursor_fini(&cursor);
		return (SET_ERROR(ENOENT));
	}

	if (strlen(attr.za_name) + 1 > namelen) {
		zap_cursor_fini(&cursor);
		return (SET_ERROR(ENAMETOOLONG));
	}

	(void) strcpy(name, attr.za_name);
	if (idp)
		*idp = attr.za_first_integer;
	zap_cursor_advance(&cursor);
	*offp = zap_cursor_serialize(&cursor);
	zap_cursor_fini(&cursor);

	return (0);
}

typedef struct dmu_objset_find_ctx {
	taskq_t		*dc_tq;
	dsl_pool_t	*dc_dp;
	uint64_t	dc_ddobj;
<<<<<<< HEAD
=======
	char		*dc_ddname; /* last component of ddobj's name */
>>>>>>> temp
	int		(*dc_func)(dsl_pool_t *, dsl_dataset_t *, void *);
	void		*dc_arg;
	int		dc_flags;
	kmutex_t	*dc_error_lock;
	int		*dc_error;
} dmu_objset_find_ctx_t;

static void
dmu_objset_find_dp_impl(dmu_objset_find_ctx_t *dcp)
{
	dsl_pool_t *dp = dcp->dc_dp;
	dmu_objset_find_ctx_t *child_dcp;
	dsl_dir_t *dd;
	dsl_dataset_t *ds;
	zap_cursor_t zc;
	zap_attribute_t *attr;
	uint64_t thisobj;
	int err = 0;

	/* don't process if there already was an error */
	if (*dcp->dc_error != 0)
		goto out;

<<<<<<< HEAD
	err = dsl_dir_hold_obj(dp, dcp->dc_ddobj, NULL, FTAG, &dd);
=======
	/*
	 * Note: passing the name (dc_ddname) here is optional, but it
	 * improves performance because we don't need to call
	 * zap_value_search() to determine the name.
	 */
	err = dsl_dir_hold_obj(dp, dcp->dc_ddobj, dcp->dc_ddname, FTAG, &dd);
>>>>>>> temp
	if (err != 0)
		goto out;

	/* Don't visit hidden ($MOS & $ORIGIN) objsets. */
	if (dd->dd_myname[0] == '$') {
		dsl_dir_rele(dd, FTAG);
		goto out;
	}

	thisobj = dsl_dir_phys(dd)->dd_head_dataset_obj;
	attr = kmem_alloc(sizeof (zap_attribute_t), KM_SLEEP);

	/*
	 * Iterate over all children.
	 */
	if (dcp->dc_flags & DS_FIND_CHILDREN) {
		for (zap_cursor_init(&zc, dp->dp_meta_objset,
		    dsl_dir_phys(dd)->dd_child_dir_zapobj);
		    zap_cursor_retrieve(&zc, attr) == 0;
		    (void) zap_cursor_advance(&zc)) {
			ASSERT3U(attr->za_integer_length, ==,
			    sizeof (uint64_t));
			ASSERT3U(attr->za_num_integers, ==, 1);

<<<<<<< HEAD
			child_dcp = kmem_alloc(sizeof (*child_dcp), KM_SLEEP);
			*child_dcp = *dcp;
			child_dcp->dc_ddobj = attr->za_first_integer;
=======
			child_dcp =
			    kmem_alloc(sizeof (*child_dcp), KM_SLEEP);
			*child_dcp = *dcp;
			child_dcp->dc_ddobj = attr->za_first_integer;
			child_dcp->dc_ddname = spa_strdup(attr->za_name);
>>>>>>> temp
			if (dcp->dc_tq != NULL)
				(void) taskq_dispatch(dcp->dc_tq,
				    dmu_objset_find_dp_cb, child_dcp, TQ_SLEEP);
			else
				dmu_objset_find_dp_impl(child_dcp);
		}
		zap_cursor_fini(&zc);
	}

	/*
	 * Iterate over all snapshots.
	 */
	if (dcp->dc_flags & DS_FIND_SNAPSHOTS) {
		dsl_dataset_t *ds;
		err = dsl_dataset_hold_obj(dp, thisobj, FTAG, &ds);

		if (err == 0) {
			uint64_t snapobj;

			snapobj = dsl_dataset_phys(ds)->ds_snapnames_zapobj;
			dsl_dataset_rele(ds, FTAG);

			for (zap_cursor_init(&zc, dp->dp_meta_objset, snapobj);
			    zap_cursor_retrieve(&zc, attr) == 0;
			    (void) zap_cursor_advance(&zc)) {
				ASSERT3U(attr->za_integer_length, ==,
				    sizeof (uint64_t));
				ASSERT3U(attr->za_num_integers, ==, 1);

				err = dsl_dataset_hold_obj(dp,
				    attr->za_first_integer, FTAG, &ds);
				if (err != 0)
					break;
				err = dcp->dc_func(dp, ds, dcp->dc_arg);
				dsl_dataset_rele(ds, FTAG);
				if (err != 0)
					break;
			}
			zap_cursor_fini(&zc);
		}
	}

<<<<<<< HEAD
	dsl_dir_rele(dd, FTAG);
	kmem_free(attr, sizeof (zap_attribute_t));

	if (err != 0)
		goto out;
=======
	kmem_free(attr, sizeof (zap_attribute_t));

	if (err != 0) {
		dsl_dir_rele(dd, FTAG);
		goto out;
	}
>>>>>>> temp

	/*
	 * Apply to self.
	 */
	err = dsl_dataset_hold_obj(dp, thisobj, FTAG, &ds);
<<<<<<< HEAD
=======

	/*
	 * Note: we hold the dir while calling dsl_dataset_hold_obj() so
	 * that the dir will remain cached, and we won't have to re-instantiate
	 * it (which could be expensive due to finding its name via
	 * zap_value_search()).
	 */
	dsl_dir_rele(dd, FTAG);
>>>>>>> temp
	if (err != 0)
		goto out;
	err = dcp->dc_func(dp, ds, dcp->dc_arg);
	dsl_dataset_rele(ds, FTAG);

out:
	if (err != 0) {
		mutex_enter(dcp->dc_error_lock);
		/* only keep first error */
		if (*dcp->dc_error == 0)
			*dcp->dc_error = err;
		mutex_exit(dcp->dc_error_lock);
	}

<<<<<<< HEAD
=======
	if (dcp->dc_ddname != NULL)
		spa_strfree(dcp->dc_ddname);
>>>>>>> temp
	kmem_free(dcp, sizeof (*dcp));
}

static void
dmu_objset_find_dp_cb(void *arg)
{
	dmu_objset_find_ctx_t *dcp = arg;
	dsl_pool_t *dp = dcp->dc_dp;

	/*
	 * We need to get a pool_config_lock here, as there are several
	 * asssert(pool_config_held) down the stack. Getting a lock via
	 * dsl_pool_config_enter is risky, as it might be stalled by a
	 * pending writer. This would deadlock, as the write lock can
	 * only be granted when our parent thread gives up the lock.
	 * The _prio interface gives us priority over a pending writer.
	 */
	dsl_pool_config_enter_prio(dp, FTAG);

	dmu_objset_find_dp_impl(dcp);

	dsl_pool_config_exit(dp, FTAG);
}

/*
 * Find objsets under and including ddobj, call func(ds) on each.
 * The order for the enumeration is completely undefined.
 * func is called with dsl_pool_config held.
 */
int
dmu_objset_find_dp(dsl_pool_t *dp, uint64_t ddobj,
    int func(dsl_pool_t *, dsl_dataset_t *, void *), void *arg, int flags)
{
	int error = 0;
	taskq_t *tq = NULL;
	int ntasks;
	dmu_objset_find_ctx_t *dcp;
	kmutex_t err_lock;

	mutex_init(&err_lock, NULL, MUTEX_DEFAULT, NULL);
	dcp = kmem_alloc(sizeof (*dcp), KM_SLEEP);
	dcp->dc_tq = NULL;
	dcp->dc_dp = dp;
	dcp->dc_ddobj = ddobj;
<<<<<<< HEAD
=======
	dcp->dc_ddname = NULL;
>>>>>>> temp
	dcp->dc_func = func;
	dcp->dc_arg = arg;
	dcp->dc_flags = flags;
	dcp->dc_error_lock = &err_lock;
	dcp->dc_error = &error;

	if ((flags & DS_FIND_SERIALIZE) || dsl_pool_config_held_writer(dp)) {
		/*
		 * In case a write lock is held we can't make use of
		 * parallelism, as down the stack of the worker threads
		 * the lock is asserted via dsl_pool_config_held.
		 * In case of a read lock this is solved by getting a read
		 * lock in each worker thread, which isn't possible in case
		 * of a writer lock. So we fall back to the synchronous path
		 * here.
		 * In the future it might be possible to get some magic into
		 * dsl_pool_config_held in a way that it returns true for
		 * the worker threads so that a single lock held from this
		 * thread suffices. For now, stay single threaded.
		 */
		dmu_objset_find_dp_impl(dcp);
<<<<<<< HEAD
=======
		mutex_destroy(&err_lock);
>>>>>>> temp

		return (error);
	}

	ntasks = dmu_find_threads;
	if (ntasks == 0)
		ntasks = vdev_count_leaves(dp->dp_spa) * 4;
	tq = taskq_create("dmu_objset_find", ntasks, maxclsyspri, ntasks,
	    INT_MAX, 0);
	if (tq == NULL) {
		kmem_free(dcp, sizeof (*dcp));
<<<<<<< HEAD
=======
		mutex_destroy(&err_lock);

>>>>>>> temp
		return (SET_ERROR(ENOMEM));
	}
	dcp->dc_tq = tq;

	/* dcp will be freed by task */
	(void) taskq_dispatch(tq, dmu_objset_find_dp_cb, dcp, TQ_SLEEP);

	/*
	 * PORTING: this code relies on the property of taskq_wait to wait
	 * until no more tasks are queued and no more tasks are active. As
	 * we always queue new tasks from within other tasks, task_wait
	 * reliably waits for the full recursion to finish, even though we
	 * enqueue new tasks after taskq_wait has been called.
	 * On platforms other than illumos, taskq_wait may not have this
	 * property.
	 */
	taskq_wait(tq);
	taskq_destroy(tq);
	mutex_destroy(&err_lock);

	return (error);
}

/*
 * Find all objsets under name, and for each, call 'func(child_name, arg)'.
 * The dp_config_rwlock must not be held when this is called, and it
 * will not be held when the callback is called.
 * Therefore this function should only be used when the pool is not changing
 * (e.g. in syncing context), or the callback can deal with the possible races.
 */
static int
dmu_objset_find_impl(spa_t *spa, const char *name,
    int func(const char *, void *), void *arg, int flags)
{
	dsl_dir_t *dd;
	dsl_pool_t *dp = spa_get_dsl(spa);
	dsl_dataset_t *ds;
	zap_cursor_t zc;
	zap_attribute_t *attr;
	char *child;
	uint64_t thisobj;
	int err;

	dsl_pool_config_enter(dp, FTAG);

	err = dsl_dir_hold(dp, name, FTAG, &dd, NULL);
	if (err != 0) {
		dsl_pool_config_exit(dp, FTAG);
		return (err);
	}

	/* Don't visit hidden ($MOS & $ORIGIN) objsets. */
	if (dd->dd_myname[0] == '$') {
		dsl_dir_rele(dd, FTAG);
		dsl_pool_config_exit(dp, FTAG);
		return (0);
	}

	thisobj = dsl_dir_phys(dd)->dd_head_dataset_obj;
	attr = kmem_alloc(sizeof (zap_attribute_t), KM_SLEEP);

	/*
	 * Iterate over all children.
	 */
	if (flags & DS_FIND_CHILDREN) {
		for (zap_cursor_init(&zc, dp->dp_meta_objset,
		    dsl_dir_phys(dd)->dd_child_dir_zapobj);
		    zap_cursor_retrieve(&zc, attr) == 0;
		    (void) zap_cursor_advance(&zc)) {
			ASSERT3U(attr->za_integer_length, ==,
			    sizeof (uint64_t));
			ASSERT3U(attr->za_num_integers, ==, 1);

			child = kmem_asprintf("%s/%s", name, attr->za_name);
			dsl_pool_config_exit(dp, FTAG);
			err = dmu_objset_find_impl(spa, child,
			    func, arg, flags);
			dsl_pool_config_enter(dp, FTAG);
			strfree(child);
			if (err != 0)
				break;
		}
		zap_cursor_fini(&zc);

		if (err != 0) {
			dsl_dir_rele(dd, FTAG);
			dsl_pool_config_exit(dp, FTAG);
			kmem_free(attr, sizeof (zap_attribute_t));
			return (err);
		}
	}

	/*
	 * Iterate over all snapshots.
	 */
	if (flags & DS_FIND_SNAPSHOTS) {
		err = dsl_dataset_hold_obj(dp, thisobj, FTAG, &ds);

		if (err == 0) {
			uint64_t snapobj;

			snapobj = dsl_dataset_phys(ds)->ds_snapnames_zapobj;
			dsl_dataset_rele(ds, FTAG);

			for (zap_cursor_init(&zc, dp->dp_meta_objset, snapobj);
			    zap_cursor_retrieve(&zc, attr) == 0;
			    (void) zap_cursor_advance(&zc)) {
				ASSERT3U(attr->za_integer_length, ==,
				    sizeof (uint64_t));
				ASSERT3U(attr->za_num_integers, ==, 1);

				child = kmem_asprintf("%s@%s",
				    name, attr->za_name);
				dsl_pool_config_exit(dp, FTAG);
				err = func(child, arg);
				dsl_pool_config_enter(dp, FTAG);
				strfree(child);
				if (err != 0)
					break;
			}
			zap_cursor_fini(&zc);
		}
	}

	dsl_dir_rele(dd, FTAG);
	kmem_free(attr, sizeof (zap_attribute_t));
	dsl_pool_config_exit(dp, FTAG);

	if (err != 0)
		return (err);

	/* Apply to self. */
	return (func(name, arg));
}

/*
 * See comment above dmu_objset_find_impl().
 */
int
dmu_objset_find(char *name, int func(const char *, void *), void *arg,
    int flags)
{
	spa_t *spa;
	int error;

	error = spa_open(name, &spa, FTAG);
	if (error != 0)
		return (error);
	error = dmu_objset_find_impl(spa, name, func, arg, flags);
	spa_close(spa, FTAG);
	return (error);
}

void
dmu_objset_set_user(objset_t *os, void *user_ptr)
{
	ASSERT(MUTEX_HELD(&os->os_user_ptr_lock));
	os->os_user_ptr = user_ptr;
}

void *
dmu_objset_get_user(objset_t *os)
{
	ASSERT(MUTEX_HELD(&os->os_user_ptr_lock));
	return (os->os_user_ptr);
}

/*
 * Determine name of filesystem, given name of snapshot.
<<<<<<< HEAD
 * buf must be at least MAXNAMELEN bytes
=======
 * buf must be at least ZFS_MAX_DATASET_NAME_LEN bytes
>>>>>>> temp
 */
int
dmu_fsname(const char *snapname, char *buf)
{
	char *atp = strchr(snapname, '@');
	if (atp == NULL)
		return (SET_ERROR(EINVAL));
<<<<<<< HEAD
	if (atp - snapname >= MAXNAMELEN)
=======
	if (atp - snapname >= ZFS_MAX_DATASET_NAME_LEN)
>>>>>>> temp
		return (SET_ERROR(ENAMETOOLONG));
	(void) strlcpy(buf, snapname, atp - snapname + 1);
	return (0);
}

<<<<<<< HEAD
=======
/*
 * Call when we think we're going to write/free space in open context to track
 * the amount of dirty data in the open txg, which is also the amount
 * of memory that can not be evicted until this txg syncs.
 */
void
dmu_objset_willuse_space(objset_t *os, int64_t space, dmu_tx_t *tx)
{
	dsl_dataset_t *ds = os->os_dsl_dataset;
	int64_t aspace = spa_get_worst_case_asize(os->os_spa, space);

	if (ds != NULL) {
		dsl_dir_willuse_space(ds->ds_dir, aspace, tx);
		dsl_pool_dirty_space(dmu_tx_pool(tx), space, tx);
	}
}

>>>>>>> temp
#if defined(_KERNEL) && defined(HAVE_SPL)
EXPORT_SYMBOL(dmu_objset_zil);
EXPORT_SYMBOL(dmu_objset_pool);
EXPORT_SYMBOL(dmu_objset_ds);
EXPORT_SYMBOL(dmu_objset_type);
EXPORT_SYMBOL(dmu_objset_name);
EXPORT_SYMBOL(dmu_objset_hold);
EXPORT_SYMBOL(dmu_objset_own);
EXPORT_SYMBOL(dmu_objset_rele);
EXPORT_SYMBOL(dmu_objset_disown);
EXPORT_SYMBOL(dmu_objset_from_ds);
EXPORT_SYMBOL(dmu_objset_create);
EXPORT_SYMBOL(dmu_objset_clone);
EXPORT_SYMBOL(dmu_objset_stats);
EXPORT_SYMBOL(dmu_objset_fast_stat);
EXPORT_SYMBOL(dmu_objset_spa);
EXPORT_SYMBOL(dmu_objset_space);
EXPORT_SYMBOL(dmu_objset_fsid_guid);
EXPORT_SYMBOL(dmu_objset_find);
EXPORT_SYMBOL(dmu_objset_byteswap);
EXPORT_SYMBOL(dmu_objset_evict_dbufs);
EXPORT_SYMBOL(dmu_objset_snap_cmtime);
<<<<<<< HEAD
=======
EXPORT_SYMBOL(dmu_objset_dnodesize);
>>>>>>> temp

EXPORT_SYMBOL(dmu_objset_sync);
EXPORT_SYMBOL(dmu_objset_is_dirty);
EXPORT_SYMBOL(dmu_objset_create_impl);
EXPORT_SYMBOL(dmu_objset_open_impl);
EXPORT_SYMBOL(dmu_objset_evict);
EXPORT_SYMBOL(dmu_objset_register_type);
EXPORT_SYMBOL(dmu_objset_do_userquota_updates);
EXPORT_SYMBOL(dmu_objset_userquota_get_ids);
EXPORT_SYMBOL(dmu_objset_userused_enabled);
EXPORT_SYMBOL(dmu_objset_userspace_upgrade);
EXPORT_SYMBOL(dmu_objset_userspace_present);
<<<<<<< HEAD
=======
EXPORT_SYMBOL(dmu_objset_userobjused_enabled);
EXPORT_SYMBOL(dmu_objset_userobjspace_upgrade);
EXPORT_SYMBOL(dmu_objset_userobjspace_upgradable);
EXPORT_SYMBOL(dmu_objset_userobjspace_present);
>>>>>>> temp
#endif
