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
 * Copyright (c) 2005, 2010, Oracle and/or its affiliates. All rights reserved.
<<<<<<< HEAD
 * Copyright (c) 2013 by Delphix. All rights reserved.
=======
 * Copyright (c) 2012, 2015 by Delphix. All rights reserved.
>>>>>>> temp
 */

/*
 * Virtual Device Labels
 * ---------------------
 *
 * The vdev label serves several distinct purposes:
 *
 *	1. Uniquely identify this device as part of a ZFS pool and confirm its
 *	   identity within the pool.
 *
 * 	2. Verify that all the devices given in a configuration are present
 *         within the pool.
 *
 * 	3. Determine the uberblock for the pool.
 *
 * 	4. In case of an import operation, determine the configuration of the
 *         toplevel vdev of which it is a part.
 *
 * 	5. If an import operation cannot find all the devices in the pool,
 *         provide enough information to the administrator to determine which
 *         devices are missing.
 *
 * It is important to note that while the kernel is responsible for writing the
 * label, it only consumes the information in the first three cases.  The
 * latter information is only consumed in userland when determining the
 * configuration to import a pool.
 *
 *
 * Label Organization
 * ------------------
 *
 * Before describing the contents of the label, it's important to understand how
 * the labels are written and updated with respect to the uberblock.
 *
 * When the pool configuration is altered, either because it was newly created
 * or a device was added, we want to update all the labels such that we can deal
 * with fatal failure at any point.  To this end, each disk has two labels which
 * are updated before and after the uberblock is synced.  Assuming we have
 * labels and an uberblock with the following transaction groups:
 *
 *              L1          UB          L2
 *           +------+    +------+    +------+
 *           |      |    |      |    |      |
 *           | t10  |    | t10  |    | t10  |
 *           |      |    |      |    |      |
 *           +------+    +------+    +------+
 *
 * In this stable state, the labels and the uberblock were all updated within
 * the same transaction group (10).  Each label is mirrored and checksummed, so
 * that we can detect when we fail partway through writing the label.
 *
 * In order to identify which labels are valid, the labels are written in the
 * following manner:
 *
 * 	1. For each vdev, update 'L1' to the new label
 * 	2. Update the uberblock
 * 	3. For each vdev, update 'L2' to the new label
 *
 * Given arbitrary failure, we can determine the correct label to use based on
 * the transaction group.  If we fail after updating L1 but before updating the
 * UB, we will notice that L1's transaction group is greater than the uberblock,
 * so L2 must be valid.  If we fail after writing the uberblock but before
 * writing L2, we will notice that L2's transaction group is less than L1, and
 * therefore L1 is valid.
 *
 * Another added complexity is that not every label is updated when the config
 * is synced.  If we add a single device, we do not want to have to re-write
 * every label for every device in the pool.  This means that both L1 and L2 may
 * be older than the pool uberblock, because the necessary information is stored
 * on another vdev.
 *
 *
 * On-disk Format
 * --------------
 *
 * The vdev label consists of two distinct parts, and is wrapped within the
 * vdev_label_t structure.  The label includes 8k of padding to permit legacy
 * VTOC disk labels, but is otherwise ignored.
 *
 * The first half of the label is a packed nvlist which contains pool wide
 * properties, per-vdev properties, and configuration information.  It is
 * described in more detail below.
 *
 * The latter half of the label consists of a redundant array of uberblocks.
 * These uberblocks are updated whenever a transaction group is committed,
 * or when the configuration is updated.  When a pool is loaded, we scan each
 * vdev for the 'best' uberblock.
 *
 *
 * Configuration Information
 * -------------------------
 *
 * The nvlist describing the pool and vdev contains the following elements:
 *
 * 	version		ZFS on-disk version
 * 	name		Pool name
 * 	state		Pool state
 * 	txg		Transaction group in which this label was written
 * 	pool_guid	Unique identifier for this pool
 * 	vdev_tree	An nvlist describing vdev tree.
 *	features_for_read
 *			An nvlist of the features necessary for reading the MOS.
 *
 * Each leaf device label also contains the following:
 *
 * 	top_guid	Unique ID for top-level vdev in which this is contained
 * 	guid		Unique ID for the leaf vdev
 *
 * The 'vs' configuration follows the format described in 'spa_config.c'.
 */

#include <sys/zfs_context.h>
#include <sys/spa.h>
#include <sys/spa_impl.h>
#include <sys/dmu.h>
#include <sys/zap.h>
#include <sys/vdev.h>
#include <sys/vdev_impl.h>
#include <sys/uberblock_impl.h>
#include <sys/metaslab.h>
#include <sys/zio.h>
#include <sys/dsl_scan.h>
<<<<<<< HEAD
=======
#include <sys/abd.h>
>>>>>>> temp
#include <sys/fs/zfs.h>

/*
 * Basic routines to read and write from a vdev label.
 * Used throughout the rest of this file.
 */
uint64_t
vdev_label_offset(uint64_t psize, int l, uint64_t offset)
{
	ASSERT(offset < sizeof (vdev_label_t));
	ASSERT(P2PHASE_TYPED(psize, sizeof (vdev_label_t), uint64_t) == 0);

	return (offset + l * sizeof (vdev_label_t) + (l < VDEV_LABELS / 2 ?
	    0 : psize - VDEV_LABELS * sizeof (vdev_label_t)));
}

/*
 * Returns back the vdev label associated with the passed in offset.
 */
int
vdev_label_number(uint64_t psize, uint64_t offset)
{
	int l;

	if (offset >= psize - VDEV_LABEL_END_SIZE) {
		offset -= psize - VDEV_LABEL_END_SIZE;
		offset += (VDEV_LABELS / 2) * sizeof (vdev_label_t);
	}
	l = offset / sizeof (vdev_label_t);
	return (l < VDEV_LABELS ? l : -1);
}

static void
<<<<<<< HEAD
vdev_label_read(zio_t *zio, vdev_t *vd, int l, void *buf, uint64_t offset,
	uint64_t size, zio_done_func_t *done, void *private, int flags)
{
	ASSERT(spa_config_held(zio->io_spa, SCL_STATE_ALL, RW_WRITER) ==
	    SCL_STATE_ALL);
=======
vdev_label_read(zio_t *zio, vdev_t *vd, int l, abd_t *buf, uint64_t offset,
    uint64_t size, zio_done_func_t *done, void *private, int flags)
{
	ASSERT(
	    spa_config_held(zio->io_spa, SCL_STATE, RW_READER) == SCL_STATE ||
	    spa_config_held(zio->io_spa, SCL_STATE, RW_WRITER) == SCL_STATE);
>>>>>>> temp
	ASSERT(flags & ZIO_FLAG_CONFIG_WRITER);

	zio_nowait(zio_read_phys(zio, vd,
	    vdev_label_offset(vd->vdev_psize, l, offset),
	    size, buf, ZIO_CHECKSUM_LABEL, done, private,
	    ZIO_PRIORITY_SYNC_READ, flags, B_TRUE));
}

<<<<<<< HEAD
static void
vdev_label_write(zio_t *zio, vdev_t *vd, int l, void *buf, uint64_t offset,
	uint64_t size, zio_done_func_t *done, void *private, int flags)
{
	ASSERT(spa_config_held(zio->io_spa, SCL_ALL, RW_WRITER) == SCL_ALL ||
	    (spa_config_held(zio->io_spa, SCL_CONFIG | SCL_STATE, RW_READER) ==
	    (SCL_CONFIG | SCL_STATE) &&
	    dsl_pool_sync_context(spa_get_dsl(zio->io_spa))));
=======
void
vdev_label_write(zio_t *zio, vdev_t *vd, int l, abd_t *buf, uint64_t offset,
    uint64_t size, zio_done_func_t *done, void *private, int flags)
{
	ASSERT(
	    spa_config_held(zio->io_spa, SCL_STATE, RW_READER) == SCL_STATE ||
	    spa_config_held(zio->io_spa, SCL_STATE, RW_WRITER) == SCL_STATE);
>>>>>>> temp
	ASSERT(flags & ZIO_FLAG_CONFIG_WRITER);

	zio_nowait(zio_write_phys(zio, vd,
	    vdev_label_offset(vd->vdev_psize, l, offset),
	    size, buf, ZIO_CHECKSUM_LABEL, done, private,
	    ZIO_PRIORITY_SYNC_WRITE, flags, B_TRUE));
}

/*
<<<<<<< HEAD
=======
 * Generate the nvlist representing this vdev's stats
 */
void
vdev_config_generate_stats(vdev_t *vd, nvlist_t *nv)
{
	nvlist_t *nvx;
	vdev_stat_t *vs;
	vdev_stat_ex_t *vsx;

	vs = kmem_alloc(sizeof (*vs), KM_SLEEP);
	vsx = kmem_alloc(sizeof (*vsx), KM_SLEEP);

	vdev_get_stats_ex(vd, vs, vsx);
	fnvlist_add_uint64_array(nv, ZPOOL_CONFIG_VDEV_STATS,
	    (uint64_t *)vs, sizeof (*vs) / sizeof (uint64_t));

	kmem_free(vs, sizeof (*vs));

	/*
	 * Add extended stats into a special extended stats nvlist.  This keeps
	 * all the extended stats nicely grouped together.  The extended stats
	 * nvlist is then added to the main nvlist.
	 */
	nvx = fnvlist_alloc();

	/* ZIOs in flight to disk */
	fnvlist_add_uint64(nvx, ZPOOL_CONFIG_VDEV_SYNC_R_ACTIVE_QUEUE,
	    vsx->vsx_active_queue[ZIO_PRIORITY_SYNC_READ]);

	fnvlist_add_uint64(nvx, ZPOOL_CONFIG_VDEV_SYNC_W_ACTIVE_QUEUE,
	    vsx->vsx_active_queue[ZIO_PRIORITY_SYNC_WRITE]);

	fnvlist_add_uint64(nvx, ZPOOL_CONFIG_VDEV_ASYNC_R_ACTIVE_QUEUE,
	    vsx->vsx_active_queue[ZIO_PRIORITY_ASYNC_READ]);

	fnvlist_add_uint64(nvx, ZPOOL_CONFIG_VDEV_ASYNC_W_ACTIVE_QUEUE,
	    vsx->vsx_active_queue[ZIO_PRIORITY_ASYNC_WRITE]);

	fnvlist_add_uint64(nvx, ZPOOL_CONFIG_VDEV_SCRUB_ACTIVE_QUEUE,
	    vsx->vsx_active_queue[ZIO_PRIORITY_SCRUB]);

	/* ZIOs pending */
	fnvlist_add_uint64(nvx, ZPOOL_CONFIG_VDEV_SYNC_R_PEND_QUEUE,
	    vsx->vsx_pend_queue[ZIO_PRIORITY_SYNC_READ]);

	fnvlist_add_uint64(nvx, ZPOOL_CONFIG_VDEV_SYNC_W_PEND_QUEUE,
	    vsx->vsx_pend_queue[ZIO_PRIORITY_SYNC_WRITE]);

	fnvlist_add_uint64(nvx, ZPOOL_CONFIG_VDEV_ASYNC_R_PEND_QUEUE,
	    vsx->vsx_pend_queue[ZIO_PRIORITY_ASYNC_READ]);

	fnvlist_add_uint64(nvx, ZPOOL_CONFIG_VDEV_ASYNC_W_PEND_QUEUE,
	    vsx->vsx_pend_queue[ZIO_PRIORITY_ASYNC_WRITE]);

	fnvlist_add_uint64(nvx, ZPOOL_CONFIG_VDEV_SCRUB_PEND_QUEUE,
	    vsx->vsx_pend_queue[ZIO_PRIORITY_SCRUB]);

	/* Histograms */
	fnvlist_add_uint64_array(nvx, ZPOOL_CONFIG_VDEV_TOT_R_LAT_HISTO,
	    vsx->vsx_total_histo[ZIO_TYPE_READ],
	    ARRAY_SIZE(vsx->vsx_total_histo[ZIO_TYPE_READ]));

	fnvlist_add_uint64_array(nvx, ZPOOL_CONFIG_VDEV_TOT_W_LAT_HISTO,
	    vsx->vsx_total_histo[ZIO_TYPE_WRITE],
	    ARRAY_SIZE(vsx->vsx_total_histo[ZIO_TYPE_WRITE]));

	fnvlist_add_uint64_array(nvx, ZPOOL_CONFIG_VDEV_DISK_R_LAT_HISTO,
	    vsx->vsx_disk_histo[ZIO_TYPE_READ],
	    ARRAY_SIZE(vsx->vsx_disk_histo[ZIO_TYPE_READ]));

	fnvlist_add_uint64_array(nvx, ZPOOL_CONFIG_VDEV_DISK_W_LAT_HISTO,
	    vsx->vsx_disk_histo[ZIO_TYPE_WRITE],
	    ARRAY_SIZE(vsx->vsx_disk_histo[ZIO_TYPE_WRITE]));

	fnvlist_add_uint64_array(nvx, ZPOOL_CONFIG_VDEV_SYNC_R_LAT_HISTO,
	    vsx->vsx_queue_histo[ZIO_PRIORITY_SYNC_READ],
	    ARRAY_SIZE(vsx->vsx_queue_histo[ZIO_PRIORITY_SYNC_READ]));

	fnvlist_add_uint64_array(nvx, ZPOOL_CONFIG_VDEV_SYNC_W_LAT_HISTO,
	    vsx->vsx_queue_histo[ZIO_PRIORITY_SYNC_WRITE],
	    ARRAY_SIZE(vsx->vsx_queue_histo[ZIO_PRIORITY_SYNC_WRITE]));

	fnvlist_add_uint64_array(nvx, ZPOOL_CONFIG_VDEV_ASYNC_R_LAT_HISTO,
	    vsx->vsx_queue_histo[ZIO_PRIORITY_ASYNC_READ],
	    ARRAY_SIZE(vsx->vsx_queue_histo[ZIO_PRIORITY_ASYNC_READ]));

	fnvlist_add_uint64_array(nvx, ZPOOL_CONFIG_VDEV_ASYNC_W_LAT_HISTO,
	    vsx->vsx_queue_histo[ZIO_PRIORITY_ASYNC_WRITE],
	    ARRAY_SIZE(vsx->vsx_queue_histo[ZIO_PRIORITY_ASYNC_WRITE]));

	fnvlist_add_uint64_array(nvx, ZPOOL_CONFIG_VDEV_SCRUB_LAT_HISTO,
	    vsx->vsx_queue_histo[ZIO_PRIORITY_SCRUB],
	    ARRAY_SIZE(vsx->vsx_queue_histo[ZIO_PRIORITY_SCRUB]));

	/* Request sizes */
	fnvlist_add_uint64_array(nvx, ZPOOL_CONFIG_VDEV_SYNC_IND_R_HISTO,
	    vsx->vsx_ind_histo[ZIO_PRIORITY_SYNC_READ],
	    ARRAY_SIZE(vsx->vsx_ind_histo[ZIO_PRIORITY_SYNC_READ]));

	fnvlist_add_uint64_array(nvx, ZPOOL_CONFIG_VDEV_SYNC_IND_W_HISTO,
	    vsx->vsx_ind_histo[ZIO_PRIORITY_SYNC_WRITE],
	    ARRAY_SIZE(vsx->vsx_ind_histo[ZIO_PRIORITY_SYNC_WRITE]));

	fnvlist_add_uint64_array(nvx, ZPOOL_CONFIG_VDEV_ASYNC_IND_R_HISTO,
	    vsx->vsx_ind_histo[ZIO_PRIORITY_ASYNC_READ],
	    ARRAY_SIZE(vsx->vsx_ind_histo[ZIO_PRIORITY_ASYNC_READ]));

	fnvlist_add_uint64_array(nvx, ZPOOL_CONFIG_VDEV_ASYNC_IND_W_HISTO,
	    vsx->vsx_ind_histo[ZIO_PRIORITY_ASYNC_WRITE],
	    ARRAY_SIZE(vsx->vsx_ind_histo[ZIO_PRIORITY_ASYNC_WRITE]));

	fnvlist_add_uint64_array(nvx, ZPOOL_CONFIG_VDEV_IND_SCRUB_HISTO,
	    vsx->vsx_ind_histo[ZIO_PRIORITY_SCRUB],
	    ARRAY_SIZE(vsx->vsx_ind_histo[ZIO_PRIORITY_SCRUB]));

	fnvlist_add_uint64_array(nvx, ZPOOL_CONFIG_VDEV_SYNC_AGG_R_HISTO,
	    vsx->vsx_agg_histo[ZIO_PRIORITY_SYNC_READ],
	    ARRAY_SIZE(vsx->vsx_agg_histo[ZIO_PRIORITY_SYNC_READ]));

	fnvlist_add_uint64_array(nvx, ZPOOL_CONFIG_VDEV_SYNC_AGG_W_HISTO,
	    vsx->vsx_agg_histo[ZIO_PRIORITY_SYNC_WRITE],
	    ARRAY_SIZE(vsx->vsx_agg_histo[ZIO_PRIORITY_SYNC_WRITE]));

	fnvlist_add_uint64_array(nvx, ZPOOL_CONFIG_VDEV_ASYNC_AGG_R_HISTO,
	    vsx->vsx_agg_histo[ZIO_PRIORITY_ASYNC_READ],
	    ARRAY_SIZE(vsx->vsx_agg_histo[ZIO_PRIORITY_ASYNC_READ]));

	fnvlist_add_uint64_array(nvx, ZPOOL_CONFIG_VDEV_ASYNC_AGG_W_HISTO,
	    vsx->vsx_agg_histo[ZIO_PRIORITY_ASYNC_WRITE],
	    ARRAY_SIZE(vsx->vsx_agg_histo[ZIO_PRIORITY_ASYNC_WRITE]));

	fnvlist_add_uint64_array(nvx, ZPOOL_CONFIG_VDEV_AGG_SCRUB_HISTO,
	    vsx->vsx_agg_histo[ZIO_PRIORITY_SCRUB],
	    ARRAY_SIZE(vsx->vsx_agg_histo[ZIO_PRIORITY_SCRUB]));

	/* Add extended stats nvlist to main nvlist */
	fnvlist_add_nvlist(nv, ZPOOL_CONFIG_VDEV_STATS_EX, nvx);

	fnvlist_free(nvx);
	kmem_free(vsx, sizeof (*vsx));
}

/*
>>>>>>> temp
 * Generate the nvlist representing this vdev's config.
 */
nvlist_t *
vdev_config_generate(spa_t *spa, vdev_t *vd, boolean_t getstats,
    vdev_config_flag_t flags)
{
	nvlist_t *nv = NULL;
<<<<<<< HEAD

=======
>>>>>>> temp
	nv = fnvlist_alloc();

	fnvlist_add_string(nv, ZPOOL_CONFIG_TYPE, vd->vdev_ops->vdev_op_type);
	if (!(flags & (VDEV_CONFIG_SPARE | VDEV_CONFIG_L2CACHE)))
		fnvlist_add_uint64(nv, ZPOOL_CONFIG_ID, vd->vdev_id);
	fnvlist_add_uint64(nv, ZPOOL_CONFIG_GUID, vd->vdev_guid);

	if (vd->vdev_path != NULL)
		fnvlist_add_string(nv, ZPOOL_CONFIG_PATH, vd->vdev_path);

	if (vd->vdev_devid != NULL)
		fnvlist_add_string(nv, ZPOOL_CONFIG_DEVID, vd->vdev_devid);

	if (vd->vdev_physpath != NULL)
		fnvlist_add_string(nv, ZPOOL_CONFIG_PHYS_PATH,
		    vd->vdev_physpath);

<<<<<<< HEAD
=======
	if (vd->vdev_enc_sysfs_path != NULL)
		fnvlist_add_string(nv, ZPOOL_CONFIG_VDEV_ENC_SYSFS_PATH,
		    vd->vdev_enc_sysfs_path);

>>>>>>> temp
	if (vd->vdev_fru != NULL)
		fnvlist_add_string(nv, ZPOOL_CONFIG_FRU, vd->vdev_fru);

	if (vd->vdev_nparity != 0) {
		ASSERT(strcmp(vd->vdev_ops->vdev_op_type,
		    VDEV_TYPE_RAIDZ) == 0);

		/*
		 * Make sure someone hasn't managed to sneak a fancy new vdev
		 * into a crufty old storage pool.
		 */
		ASSERT(vd->vdev_nparity == 1 ||
		    (vd->vdev_nparity <= 2 &&
		    spa_version(spa) >= SPA_VERSION_RAIDZ2) ||
		    (vd->vdev_nparity <= 3 &&
		    spa_version(spa) >= SPA_VERSION_RAIDZ3));

		/*
		 * Note that we'll add the nparity tag even on storage pools
		 * that only support a single parity device -- older software
		 * will just ignore it.
		 */
		fnvlist_add_uint64(nv, ZPOOL_CONFIG_NPARITY, vd->vdev_nparity);
	}

	if (vd->vdev_wholedisk != -1ULL)
		fnvlist_add_uint64(nv, ZPOOL_CONFIG_WHOLE_DISK,
		    vd->vdev_wholedisk);

	if (vd->vdev_not_present)
		fnvlist_add_uint64(nv, ZPOOL_CONFIG_NOT_PRESENT, 1);

	if (vd->vdev_isspare)
		fnvlist_add_uint64(nv, ZPOOL_CONFIG_IS_SPARE, 1);

	if (!(flags & (VDEV_CONFIG_SPARE | VDEV_CONFIG_L2CACHE)) &&
	    vd == vd->vdev_top) {
		fnvlist_add_uint64(nv, ZPOOL_CONFIG_METASLAB_ARRAY,
		    vd->vdev_ms_array);
		fnvlist_add_uint64(nv, ZPOOL_CONFIG_METASLAB_SHIFT,
		    vd->vdev_ms_shift);
		fnvlist_add_uint64(nv, ZPOOL_CONFIG_ASHIFT, vd->vdev_ashift);
		fnvlist_add_uint64(nv, ZPOOL_CONFIG_ASIZE,
		    vd->vdev_asize);
		fnvlist_add_uint64(nv, ZPOOL_CONFIG_IS_LOG, vd->vdev_islog);
		if (vd->vdev_removing)
			fnvlist_add_uint64(nv, ZPOOL_CONFIG_REMOVING,
			    vd->vdev_removing);
	}

	if (vd->vdev_dtl_sm != NULL) {
		fnvlist_add_uint64(nv, ZPOOL_CONFIG_DTL,
		    space_map_object(vd->vdev_dtl_sm));
	}

	if (vd->vdev_crtxg)
		fnvlist_add_uint64(nv, ZPOOL_CONFIG_CREATE_TXG, vd->vdev_crtxg);

<<<<<<< HEAD
	if (getstats) {
		vdev_stat_t vs;
		pool_scan_stat_t ps;

		vdev_get_stats(vd, &vs);
		fnvlist_add_uint64_array(nv, ZPOOL_CONFIG_VDEV_STATS,
		    (uint64_t *)&vs, sizeof (vs) / sizeof (uint64_t));
=======
	if (flags & VDEV_CONFIG_MOS) {
		if (vd->vdev_leaf_zap != 0) {
			ASSERT(vd->vdev_ops->vdev_op_leaf);
			fnvlist_add_uint64(nv, ZPOOL_CONFIG_VDEV_LEAF_ZAP,
			    vd->vdev_leaf_zap);
		}

		if (vd->vdev_top_zap != 0) {
			ASSERT(vd == vd->vdev_top);
			fnvlist_add_uint64(nv, ZPOOL_CONFIG_VDEV_TOP_ZAP,
			    vd->vdev_top_zap);
		}
	}

	if (getstats) {
		pool_scan_stat_t ps;

		vdev_config_generate_stats(vd, nv);
>>>>>>> temp

		/* provide either current or previous scan information */
		if (spa_scan_get_stats(spa, &ps) == 0) {
			fnvlist_add_uint64_array(nv,
			    ZPOOL_CONFIG_SCAN_STATS, (uint64_t *)&ps,
			    sizeof (pool_scan_stat_t) / sizeof (uint64_t));
		}
	}

	if (!vd->vdev_ops->vdev_op_leaf) {
		nvlist_t **child;
		int c, idx;

		ASSERT(!vd->vdev_ishole);

		child = kmem_alloc(vd->vdev_children * sizeof (nvlist_t *),
		    KM_SLEEP);

		for (c = 0, idx = 0; c < vd->vdev_children; c++) {
			vdev_t *cvd = vd->vdev_child[c];

			/*
			 * If we're generating an nvlist of removing
			 * vdevs then skip over any device which is
			 * not being removed.
			 */
			if ((flags & VDEV_CONFIG_REMOVING) &&
			    !cvd->vdev_removing)
				continue;

			child[idx++] = vdev_config_generate(spa, cvd,
			    getstats, flags);
		}

		if (idx) {
			fnvlist_add_nvlist_array(nv, ZPOOL_CONFIG_CHILDREN,
			    child, idx);
		}

		for (c = 0; c < idx; c++)
			nvlist_free(child[c]);

		kmem_free(child, vd->vdev_children * sizeof (nvlist_t *));

	} else {
		const char *aux = NULL;

		if (vd->vdev_offline && !vd->vdev_tmpoffline)
			fnvlist_add_uint64(nv, ZPOOL_CONFIG_OFFLINE, B_TRUE);
		if (vd->vdev_resilver_txg != 0)
			fnvlist_add_uint64(nv, ZPOOL_CONFIG_RESILVER_TXG,
			    vd->vdev_resilver_txg);
		if (vd->vdev_faulted)
			fnvlist_add_uint64(nv, ZPOOL_CONFIG_FAULTED, B_TRUE);
		if (vd->vdev_degraded)
			fnvlist_add_uint64(nv, ZPOOL_CONFIG_DEGRADED, B_TRUE);
		if (vd->vdev_removed)
			fnvlist_add_uint64(nv, ZPOOL_CONFIG_REMOVED, B_TRUE);
		if (vd->vdev_unspare)
			fnvlist_add_uint64(nv, ZPOOL_CONFIG_UNSPARE, B_TRUE);
		if (vd->vdev_ishole)
			fnvlist_add_uint64(nv, ZPOOL_CONFIG_IS_HOLE, B_TRUE);

<<<<<<< HEAD
=======
		/* Set the reason why we're FAULTED/DEGRADED. */
>>>>>>> temp
		switch (vd->vdev_stat.vs_aux) {
		case VDEV_AUX_ERR_EXCEEDED:
			aux = "err_exceeded";
			break;

		case VDEV_AUX_EXTERNAL:
			aux = "external";
			break;
		}

<<<<<<< HEAD
		if (aux != NULL)
			fnvlist_add_string(nv, ZPOOL_CONFIG_AUX_STATE, aux);
=======
		if (aux != NULL && !vd->vdev_tmpoffline) {
			fnvlist_add_string(nv, ZPOOL_CONFIG_AUX_STATE, aux);
		} else {
			/*
			 * We're healthy - clear any previous AUX_STATE values.
			 */
			if (nvlist_exists(nv, ZPOOL_CONFIG_AUX_STATE))
				nvlist_remove_all(nv, ZPOOL_CONFIG_AUX_STATE);
		}
>>>>>>> temp

		if (vd->vdev_splitting && vd->vdev_orig_guid != 0LL) {
			fnvlist_add_uint64(nv, ZPOOL_CONFIG_ORIG_GUID,
			    vd->vdev_orig_guid);
		}
	}

	return (nv);
}

/*
 * Generate a view of the top-level vdevs.  If we currently have holes
 * in the namespace, then generate an array which contains a list of holey
 * vdevs.  Additionally, add the number of top-level children that currently
 * exist.
 */
void
vdev_top_config_generate(spa_t *spa, nvlist_t *config)
{
	vdev_t *rvd = spa->spa_root_vdev;
	uint64_t *array;
	uint_t c, idx;

	array = kmem_alloc(rvd->vdev_children * sizeof (uint64_t), KM_SLEEP);

	for (c = 0, idx = 0; c < rvd->vdev_children; c++) {
		vdev_t *tvd = rvd->vdev_child[c];

		if (tvd->vdev_ishole)
			array[idx++] = c;
	}

	if (idx) {
		VERIFY(nvlist_add_uint64_array(config, ZPOOL_CONFIG_HOLE_ARRAY,
		    array, idx) == 0);
	}

	VERIFY(nvlist_add_uint64(config, ZPOOL_CONFIG_VDEV_CHILDREN,
	    rvd->vdev_children) == 0);

	kmem_free(array, rvd->vdev_children * sizeof (uint64_t));
}

/*
 * Returns the configuration from the label of the given vdev. For vdevs
 * which don't have a txg value stored on their label (i.e. spares/cache)
 * or have not been completely initialized (txg = 0) just return
 * the configuration from the first valid label we find. Otherwise,
 * find the most up-to-date label that does not exceed the specified
 * 'txg' value.
 */
nvlist_t *
vdev_label_read_config(vdev_t *vd, uint64_t txg)
{
	spa_t *spa = vd->vdev_spa;
	nvlist_t *config = NULL;
	vdev_phys_t *vp;
<<<<<<< HEAD
=======
	abd_t *vp_abd;
>>>>>>> temp
	zio_t *zio;
	uint64_t best_txg = 0;
	int error = 0;
	int flags = ZIO_FLAG_CONFIG_WRITER | ZIO_FLAG_CANFAIL |
	    ZIO_FLAG_SPECULATIVE;
	int l;

	ASSERT(spa_config_held(spa, SCL_STATE_ALL, RW_WRITER) == SCL_STATE_ALL);

	if (!vdev_readable(vd))
		return (NULL);

<<<<<<< HEAD
	vp = zio_buf_alloc(sizeof (vdev_phys_t));
=======
	vp_abd = abd_alloc_linear(sizeof (vdev_phys_t), B_TRUE);
	vp = abd_to_buf(vp_abd);
>>>>>>> temp

retry:
	for (l = 0; l < VDEV_LABELS; l++) {
		nvlist_t *label = NULL;

		zio = zio_root(spa, NULL, NULL, flags);

<<<<<<< HEAD
		vdev_label_read(zio, vd, l, vp,
=======
		vdev_label_read(zio, vd, l, vp_abd,
>>>>>>> temp
		    offsetof(vdev_label_t, vl_vdev_phys),
		    sizeof (vdev_phys_t), NULL, NULL, flags);

		if (zio_wait(zio) == 0 &&
		    nvlist_unpack(vp->vp_nvlist, sizeof (vp->vp_nvlist),
		    &label, 0) == 0) {
			uint64_t label_txg = 0;

			/*
			 * Auxiliary vdevs won't have txg values in their
			 * labels and newly added vdevs may not have been
			 * completely initialized so just return the
			 * configuration from the first valid label we
			 * encounter.
			 */
			error = nvlist_lookup_uint64(label,
			    ZPOOL_CONFIG_POOL_TXG, &label_txg);
			if ((error || label_txg == 0) && !config) {
				config = label;
				break;
			} else if (label_txg <= txg && label_txg > best_txg) {
				best_txg = label_txg;
				nvlist_free(config);
				config = fnvlist_dup(label);
			}
		}

		if (label != NULL) {
			nvlist_free(label);
			label = NULL;
		}
	}

	if (config == NULL && !(flags & ZIO_FLAG_TRYHARD)) {
		flags |= ZIO_FLAG_TRYHARD;
		goto retry;
	}

<<<<<<< HEAD
	zio_buf_free(vp, sizeof (vdev_phys_t));
=======
	abd_free(vp_abd);
>>>>>>> temp

	return (config);
}

/*
 * Determine if a device is in use.  The 'spare_guid' parameter will be filled
 * in with the device guid if this spare is active elsewhere on the system.
 */
static boolean_t
vdev_inuse(vdev_t *vd, uint64_t crtxg, vdev_labeltype_t reason,
    uint64_t *spare_guid, uint64_t *l2cache_guid)
{
	spa_t *spa = vd->vdev_spa;
	uint64_t state, pool_guid, device_guid, txg, spare_pool;
	uint64_t vdtxg = 0;
	nvlist_t *label;

	if (spare_guid)
		*spare_guid = 0ULL;
	if (l2cache_guid)
		*l2cache_guid = 0ULL;

	/*
	 * Read the label, if any, and perform some basic sanity checks.
	 */
	if ((label = vdev_label_read_config(vd, -1ULL)) == NULL)
		return (B_FALSE);

	(void) nvlist_lookup_uint64(label, ZPOOL_CONFIG_CREATE_TXG,
	    &vdtxg);

	if (nvlist_lookup_uint64(label, ZPOOL_CONFIG_POOL_STATE,
	    &state) != 0 ||
	    nvlist_lookup_uint64(label, ZPOOL_CONFIG_GUID,
	    &device_guid) != 0) {
		nvlist_free(label);
		return (B_FALSE);
	}

	if (state != POOL_STATE_SPARE && state != POOL_STATE_L2CACHE &&
	    (nvlist_lookup_uint64(label, ZPOOL_CONFIG_POOL_GUID,
	    &pool_guid) != 0 ||
	    nvlist_lookup_uint64(label, ZPOOL_CONFIG_POOL_TXG,
	    &txg) != 0)) {
		nvlist_free(label);
		return (B_FALSE);
	}

	nvlist_free(label);

	/*
	 * Check to see if this device indeed belongs to the pool it claims to
	 * be a part of.  The only way this is allowed is if the device is a hot
	 * spare (which we check for later on).
	 */
	if (state != POOL_STATE_SPARE && state != POOL_STATE_L2CACHE &&
	    !spa_guid_exists(pool_guid, device_guid) &&
	    !spa_spare_exists(device_guid, NULL, NULL) &&
	    !spa_l2cache_exists(device_guid, NULL))
		return (B_FALSE);

	/*
	 * If the transaction group is zero, then this an initialized (but
	 * unused) label.  This is only an error if the create transaction
	 * on-disk is the same as the one we're using now, in which case the
	 * user has attempted to add the same vdev multiple times in the same
	 * transaction.
	 */
	if (state != POOL_STATE_SPARE && state != POOL_STATE_L2CACHE &&
	    txg == 0 && vdtxg == crtxg)
		return (B_TRUE);

	/*
	 * Check to see if this is a spare device.  We do an explicit check for
	 * spa_has_spare() here because it may be on our pending list of spares
	 * to add.  We also check if it is an l2cache device.
	 */
	if (spa_spare_exists(device_guid, &spare_pool, NULL) ||
	    spa_has_spare(spa, device_guid)) {
		if (spare_guid)
			*spare_guid = device_guid;

		switch (reason) {
		case VDEV_LABEL_CREATE:
		case VDEV_LABEL_L2CACHE:
			return (B_TRUE);

		case VDEV_LABEL_REPLACE:
			return (!spa_has_spare(spa, device_guid) ||
			    spare_pool != 0ULL);

		case VDEV_LABEL_SPARE:
			return (spa_has_spare(spa, device_guid));
		default:
			break;
		}
	}

	/*
	 * Check to see if this is an l2cache device.
	 */
	if (spa_l2cache_exists(device_guid, NULL))
		return (B_TRUE);

	/*
	 * We can't rely on a pool's state if it's been imported
	 * read-only.  Instead we look to see if the pools is marked
	 * read-only in the namespace and set the state to active.
	 */
	if (state != POOL_STATE_SPARE && state != POOL_STATE_L2CACHE &&
	    (spa = spa_by_guid(pool_guid, device_guid)) != NULL &&
	    spa_mode(spa) == FREAD)
		state = POOL_STATE_ACTIVE;

	/*
	 * If the device is marked ACTIVE, then this device is in use by another
	 * pool on the system.
	 */
	return (state == POOL_STATE_ACTIVE);
}

/*
 * Initialize a vdev label.  We check to make sure each leaf device is not in
 * use, and writable.  We put down an initial label which we will later
 * overwrite with a complete label.  Note that it's important to do this
 * sequentially, not in parallel, so that we catch cases of multiple use of the
 * same leaf vdev in the vdev we're creating -- e.g. mirroring a disk with
 * itself.
 */
int
vdev_label_init(vdev_t *vd, uint64_t crtxg, vdev_labeltype_t reason)
{
	spa_t *spa = vd->vdev_spa;
	nvlist_t *label;
	vdev_phys_t *vp;
<<<<<<< HEAD
	char *pad2;
	uberblock_t *ub;
=======
	abd_t *vp_abd;
	abd_t *pad2;
	uberblock_t *ub;
	abd_t *ub_abd;
>>>>>>> temp
	zio_t *zio;
	char *buf;
	size_t buflen;
	int error;
	uint64_t spare_guid = 0, l2cache_guid = 0;
	int flags = ZIO_FLAG_CONFIG_WRITER | ZIO_FLAG_CANFAIL;
	int c, l;
	vdev_t *pvd;

	ASSERT(spa_config_held(spa, SCL_ALL, RW_WRITER) == SCL_ALL);

	for (c = 0; c < vd->vdev_children; c++)
		if ((error = vdev_label_init(vd->vdev_child[c],
		    crtxg, reason)) != 0)
			return (error);

	/* Track the creation time for this vdev */
	vd->vdev_crtxg = crtxg;

	if (!vd->vdev_ops->vdev_op_leaf || !spa_writeable(spa))
		return (0);

	/*
	 * Dead vdevs cannot be initialized.
	 */
	if (vdev_is_dead(vd))
		return (SET_ERROR(EIO));

	/*
	 * Determine if the vdev is in use.
	 */
	if (reason != VDEV_LABEL_REMOVE && reason != VDEV_LABEL_SPLIT &&
	    vdev_inuse(vd, crtxg, reason, &spare_guid, &l2cache_guid))
		return (SET_ERROR(EBUSY));

	/*
	 * If this is a request to add or replace a spare or l2cache device
	 * that is in use elsewhere on the system, then we must update the
	 * guid (which was initialized to a random value) to reflect the
	 * actual GUID (which is shared between multiple pools).
	 */
	if (reason != VDEV_LABEL_REMOVE && reason != VDEV_LABEL_L2CACHE &&
	    spare_guid != 0ULL) {
		uint64_t guid_delta = spare_guid - vd->vdev_guid;

		vd->vdev_guid += guid_delta;

		for (pvd = vd; pvd != NULL; pvd = pvd->vdev_parent)
			pvd->vdev_guid_sum += guid_delta;

		/*
		 * If this is a replacement, then we want to fallthrough to the
		 * rest of the code.  If we're adding a spare, then it's already
		 * labeled appropriately and we can just return.
		 */
		if (reason == VDEV_LABEL_SPARE)
			return (0);
		ASSERT(reason == VDEV_LABEL_REPLACE ||
		    reason == VDEV_LABEL_SPLIT);
	}

	if (reason != VDEV_LABEL_REMOVE && reason != VDEV_LABEL_SPARE &&
	    l2cache_guid != 0ULL) {
		uint64_t guid_delta = l2cache_guid - vd->vdev_guid;

		vd->vdev_guid += guid_delta;

		for (pvd = vd; pvd != NULL; pvd = pvd->vdev_parent)
			pvd->vdev_guid_sum += guid_delta;

		/*
		 * If this is a replacement, then we want to fallthrough to the
		 * rest of the code.  If we're adding an l2cache, then it's
		 * already labeled appropriately and we can just return.
		 */
		if (reason == VDEV_LABEL_L2CACHE)
			return (0);
		ASSERT(reason == VDEV_LABEL_REPLACE);
	}

	/*
	 * Initialize its label.
	 */
<<<<<<< HEAD
	vp = zio_buf_alloc(sizeof (vdev_phys_t));
	bzero(vp, sizeof (vdev_phys_t));
=======
	vp_abd = abd_alloc_linear(sizeof (vdev_phys_t), B_TRUE);
	abd_zero(vp_abd, sizeof (vdev_phys_t));
	vp = abd_to_buf(vp_abd);
>>>>>>> temp

	/*
	 * Generate a label describing the pool and our top-level vdev.
	 * We mark it as being from txg 0 to indicate that it's not
	 * really part of an active pool just yet.  The labels will
	 * be written again with a meaningful txg by spa_sync().
	 */
	if (reason == VDEV_LABEL_SPARE ||
	    (reason == VDEV_LABEL_REMOVE && vd->vdev_isspare)) {
		/*
		 * For inactive hot spares, we generate a special label that
		 * identifies as a mutually shared hot spare.  We write the
		 * label if we are adding a hot spare, or if we are removing an
		 * active hot spare (in which case we want to revert the
		 * labels).
		 */
		VERIFY(nvlist_alloc(&label, NV_UNIQUE_NAME, KM_SLEEP) == 0);

		VERIFY(nvlist_add_uint64(label, ZPOOL_CONFIG_VERSION,
		    spa_version(spa)) == 0);
		VERIFY(nvlist_add_uint64(label, ZPOOL_CONFIG_POOL_STATE,
		    POOL_STATE_SPARE) == 0);
		VERIFY(nvlist_add_uint64(label, ZPOOL_CONFIG_GUID,
		    vd->vdev_guid) == 0);
	} else if (reason == VDEV_LABEL_L2CACHE ||
	    (reason == VDEV_LABEL_REMOVE && vd->vdev_isl2cache)) {
		/*
		 * For level 2 ARC devices, add a special label.
		 */
		VERIFY(nvlist_alloc(&label, NV_UNIQUE_NAME, KM_SLEEP) == 0);

		VERIFY(nvlist_add_uint64(label, ZPOOL_CONFIG_VERSION,
		    spa_version(spa)) == 0);
		VERIFY(nvlist_add_uint64(label, ZPOOL_CONFIG_POOL_STATE,
		    POOL_STATE_L2CACHE) == 0);
		VERIFY(nvlist_add_uint64(label, ZPOOL_CONFIG_GUID,
		    vd->vdev_guid) == 0);
	} else {
		uint64_t txg = 0ULL;

		if (reason == VDEV_LABEL_SPLIT)
			txg = spa->spa_uberblock.ub_txg;
		label = spa_config_generate(spa, vd, txg, B_FALSE);

		/*
		 * Add our creation time.  This allows us to detect multiple
		 * vdev uses as described above, and automatically expires if we
		 * fail.
		 */
		VERIFY(nvlist_add_uint64(label, ZPOOL_CONFIG_CREATE_TXG,
		    crtxg) == 0);
	}

	buf = vp->vp_nvlist;
	buflen = sizeof (vp->vp_nvlist);

	error = nvlist_pack(label, &buf, &buflen, NV_ENCODE_XDR, KM_SLEEP);
	if (error != 0) {
		nvlist_free(label);
<<<<<<< HEAD
		zio_buf_free(vp, sizeof (vdev_phys_t));
=======
		abd_free(vp_abd);
>>>>>>> temp
		/* EFAULT means nvlist_pack ran out of room */
		return (error == EFAULT ? ENAMETOOLONG : EINVAL);
	}

	/*
	 * Initialize uberblock template.
	 */
<<<<<<< HEAD
	ub = zio_buf_alloc(VDEV_UBERBLOCK_RING);
	bzero(ub, VDEV_UBERBLOCK_RING);
	*ub = spa->spa_uberblock;
	ub->ub_txg = 0;

	/* Initialize the 2nd padding area. */
	pad2 = zio_buf_alloc(VDEV_PAD_SIZE);
	bzero(pad2, VDEV_PAD_SIZE);
=======
	ub_abd = abd_alloc_linear(VDEV_UBERBLOCK_RING, B_TRUE);
	abd_zero(ub_abd, VDEV_UBERBLOCK_RING);
	abd_copy_from_buf(ub_abd, &spa->spa_uberblock, sizeof (uberblock_t));
	ub = abd_to_buf(ub_abd);
	ub->ub_txg = 0;

	/* Initialize the 2nd padding area. */
	pad2 = abd_alloc_for_io(VDEV_PAD_SIZE, B_TRUE);
	abd_zero(pad2, VDEV_PAD_SIZE);
>>>>>>> temp

	/*
	 * Write everything in parallel.
	 */
retry:
	zio = zio_root(spa, NULL, NULL, flags);

	for (l = 0; l < VDEV_LABELS; l++) {

<<<<<<< HEAD
		vdev_label_write(zio, vd, l, vp,
=======
		vdev_label_write(zio, vd, l, vp_abd,
>>>>>>> temp
		    offsetof(vdev_label_t, vl_vdev_phys),
		    sizeof (vdev_phys_t), NULL, NULL, flags);

		/*
		 * Skip the 1st padding area.
		 * Zero out the 2nd padding area where it might have
		 * left over data from previous filesystem format.
		 */
		vdev_label_write(zio, vd, l, pad2,
		    offsetof(vdev_label_t, vl_pad2),
		    VDEV_PAD_SIZE, NULL, NULL, flags);

<<<<<<< HEAD
		vdev_label_write(zio, vd, l, ub,
=======
		vdev_label_write(zio, vd, l, ub_abd,
>>>>>>> temp
		    offsetof(vdev_label_t, vl_uberblock),
		    VDEV_UBERBLOCK_RING, NULL, NULL, flags);
	}

	error = zio_wait(zio);

	if (error != 0 && !(flags & ZIO_FLAG_TRYHARD)) {
		flags |= ZIO_FLAG_TRYHARD;
		goto retry;
	}

	nvlist_free(label);
<<<<<<< HEAD
	zio_buf_free(pad2, VDEV_PAD_SIZE);
	zio_buf_free(ub, VDEV_UBERBLOCK_RING);
	zio_buf_free(vp, sizeof (vdev_phys_t));
=======
	abd_free(pad2);
	abd_free(ub_abd);
	abd_free(vp_abd);
>>>>>>> temp

	/*
	 * If this vdev hasn't been previously identified as a spare, then we
	 * mark it as such only if a) we are labeling it as a spare, or b) it
	 * exists as a spare elsewhere in the system.  Do the same for
	 * level 2 ARC devices.
	 */
	if (error == 0 && !vd->vdev_isspare &&
	    (reason == VDEV_LABEL_SPARE ||
	    spa_spare_exists(vd->vdev_guid, NULL, NULL)))
		spa_spare_add(vd);

	if (error == 0 && !vd->vdev_isl2cache &&
	    (reason == VDEV_LABEL_L2CACHE ||
	    spa_l2cache_exists(vd->vdev_guid, NULL)))
		spa_l2cache_add(vd);

	return (error);
}

/*
 * ==========================================================================
 * uberblock load/sync
 * ==========================================================================
 */

/*
 * Consider the following situation: txg is safely synced to disk.  We've
 * written the first uberblock for txg + 1, and then we lose power.  When we
 * come back up, we fail to see the uberblock for txg + 1 because, say,
 * it was on a mirrored device and the replica to which we wrote txg + 1
 * is now offline.  If we then make some changes and sync txg + 1, and then
 * the missing replica comes back, then for a few seconds we'll have two
 * conflicting uberblocks on disk with the same txg.  The solution is simple:
 * among uberblocks with equal txg, choose the one with the latest timestamp.
 */
static int
<<<<<<< HEAD
vdev_uberblock_compare(uberblock_t *ub1, uberblock_t *ub2)
{
	if (ub1->ub_txg < ub2->ub_txg)
		return (-1);
	if (ub1->ub_txg > ub2->ub_txg)
		return (1);

	if (ub1->ub_timestamp < ub2->ub_timestamp)
		return (-1);
	if (ub1->ub_timestamp > ub2->ub_timestamp)
		return (1);

	return (0);
=======
vdev_uberblock_compare(const uberblock_t *ub1, const uberblock_t *ub2)
{
	int cmp = AVL_CMP(ub1->ub_txg, ub2->ub_txg);
	if (likely(cmp))
		return (cmp);

	return (AVL_CMP(ub1->ub_timestamp, ub2->ub_timestamp));
>>>>>>> temp
}

struct ubl_cbdata {
	uberblock_t	*ubl_ubbest;	/* Best uberblock */
	vdev_t		*ubl_vd;	/* vdev associated with the above */
};

static void
vdev_uberblock_load_done(zio_t *zio)
{
	vdev_t *vd = zio->io_vd;
	spa_t *spa = zio->io_spa;
	zio_t *rio = zio->io_private;
<<<<<<< HEAD
	uberblock_t *ub = zio->io_data;
=======
	uberblock_t *ub = abd_to_buf(zio->io_abd);
>>>>>>> temp
	struct ubl_cbdata *cbp = rio->io_private;

	ASSERT3U(zio->io_size, ==, VDEV_UBERBLOCK_SIZE(vd));

	if (zio->io_error == 0 && uberblock_verify(ub) == 0) {
		mutex_enter(&rio->io_lock);
		if (ub->ub_txg <= spa->spa_load_max_txg &&
		    vdev_uberblock_compare(ub, cbp->ubl_ubbest) > 0) {
			/*
			 * Keep track of the vdev in which this uberblock
			 * was found. We will use this information later
			 * to obtain the config nvlist associated with
			 * this uberblock.
			 */
			*cbp->ubl_ubbest = *ub;
			cbp->ubl_vd = vd;
		}
		mutex_exit(&rio->io_lock);
	}

<<<<<<< HEAD
	zio_buf_free(zio->io_data, zio->io_size);
=======
	abd_free(zio->io_abd);
>>>>>>> temp
}

static void
vdev_uberblock_load_impl(zio_t *zio, vdev_t *vd, int flags,
    struct ubl_cbdata *cbp)
{
<<<<<<< HEAD
	int c, l, n;

	for (c = 0; c < vd->vdev_children; c++)
		vdev_uberblock_load_impl(zio, vd->vdev_child[c], flags, cbp);

	if (vd->vdev_ops->vdev_op_leaf && vdev_readable(vd)) {
		for (l = 0; l < VDEV_LABELS; l++) {
			for (n = 0; n < VDEV_UBERBLOCK_COUNT(vd); n++) {
				vdev_label_read(zio, vd, l,
				    zio_buf_alloc(VDEV_UBERBLOCK_SIZE(vd)),
				    VDEV_UBERBLOCK_OFFSET(vd, n),
=======
	for (int c = 0; c < vd->vdev_children; c++)
		vdev_uberblock_load_impl(zio, vd->vdev_child[c], flags, cbp);

	if (vd->vdev_ops->vdev_op_leaf && vdev_readable(vd)) {
		for (int l = 0; l < VDEV_LABELS; l++) {
			for (int n = 0; n < VDEV_UBERBLOCK_COUNT(vd); n++) {
				vdev_label_read(zio, vd, l,
				    abd_alloc_linear(VDEV_UBERBLOCK_SIZE(vd),
				    B_TRUE), VDEV_UBERBLOCK_OFFSET(vd, n),
>>>>>>> temp
				    VDEV_UBERBLOCK_SIZE(vd),
				    vdev_uberblock_load_done, zio, flags);
			}
		}
	}
}

/*
 * Reads the 'best' uberblock from disk along with its associated
 * configuration. First, we read the uberblock array of each label of each
 * vdev, keeping track of the uberblock with the highest txg in each array.
 * Then, we read the configuration from the same vdev as the best uberblock.
 */
void
vdev_uberblock_load(vdev_t *rvd, uberblock_t *ub, nvlist_t **config)
{
	zio_t *zio;
	spa_t *spa = rvd->vdev_spa;
	struct ubl_cbdata cb;
	int flags = ZIO_FLAG_CONFIG_WRITER | ZIO_FLAG_CANFAIL |
	    ZIO_FLAG_SPECULATIVE | ZIO_FLAG_TRYHARD;

	ASSERT(ub);
	ASSERT(config);

	bzero(ub, sizeof (uberblock_t));
	*config = NULL;

	cb.ubl_ubbest = ub;
	cb.ubl_vd = NULL;

	spa_config_enter(spa, SCL_ALL, FTAG, RW_WRITER);
	zio = zio_root(spa, NULL, &cb, flags);
	vdev_uberblock_load_impl(zio, rvd, flags, &cb);
	(void) zio_wait(zio);

	/*
	 * It's possible that the best uberblock was discovered on a label
	 * that has a configuration which was written in a future txg.
	 * Search all labels on this vdev to find the configuration that
	 * matches the txg for our uberblock.
	 */
	if (cb.ubl_vd != NULL)
		*config = vdev_label_read_config(cb.ubl_vd, ub->ub_txg);
	spa_config_exit(spa, SCL_ALL, FTAG);
}

/*
<<<<<<< HEAD
=======
 * For use when a leaf vdev is expanded.
 * The location of labels 2 and 3 changed, and at the new location the
 * uberblock rings are either empty or contain garbage.  The sync will write
 * new configs there because the vdev is dirty, but expansion also needs the
 * uberblock rings copied.  Read them from label 0 which did not move.
 *
 * Since the point is to populate labels {2,3} with valid uberblocks,
 * we zero uberblocks we fail to read or which are not valid.
 */

static void
vdev_copy_uberblocks(vdev_t *vd)
{
	abd_t *ub_abd;
	zio_t *write_zio;
	int locks = (SCL_L2ARC | SCL_ZIO);
	int flags = ZIO_FLAG_CONFIG_WRITER | ZIO_FLAG_CANFAIL |
	    ZIO_FLAG_SPECULATIVE;

	ASSERT(spa_config_held(vd->vdev_spa, SCL_STATE, RW_READER) ==
	    SCL_STATE);
	ASSERT(vd->vdev_ops->vdev_op_leaf);

	spa_config_enter(vd->vdev_spa, locks, FTAG, RW_READER);

	ub_abd = abd_alloc_linear(VDEV_UBERBLOCK_SIZE(vd), B_TRUE);

	write_zio = zio_root(vd->vdev_spa, NULL, NULL, flags);
	for (int n = 0; n < VDEV_UBERBLOCK_COUNT(vd); n++) {
		const int src_label = 0;
		zio_t *zio;

		zio = zio_root(vd->vdev_spa, NULL, NULL, flags);
		vdev_label_read(zio, vd, src_label, ub_abd,
		    VDEV_UBERBLOCK_OFFSET(vd, n), VDEV_UBERBLOCK_SIZE(vd),
		    NULL, NULL, flags);

		if (zio_wait(zio) || uberblock_verify(abd_to_buf(ub_abd)))
			abd_zero(ub_abd, VDEV_UBERBLOCK_SIZE(vd));

		for (int l = 2; l < VDEV_LABELS; l++)
			vdev_label_write(write_zio, vd, l, ub_abd,
			    VDEV_UBERBLOCK_OFFSET(vd, n),
			    VDEV_UBERBLOCK_SIZE(vd), NULL, NULL,
			    flags | ZIO_FLAG_DONT_PROPAGATE);
	}
	(void) zio_wait(write_zio);

	spa_config_exit(vd->vdev_spa, locks, FTAG);

	abd_free(ub_abd);
}

/*
>>>>>>> temp
 * On success, increment root zio's count of good writes.
 * We only get credit for writes to known-visible vdevs; see spa_vdev_add().
 */
static void
vdev_uberblock_sync_done(zio_t *zio)
{
	uint64_t *good_writes = zio->io_private;

	if (zio->io_error == 0 && zio->io_vd->vdev_top->vdev_ms_array != 0)
<<<<<<< HEAD
		atomic_add_64(good_writes, 1);
=======
		atomic_inc_64(good_writes);
>>>>>>> temp
}

/*
 * Write the uberblock to all labels of all leaves of the specified vdev.
 */
static void
vdev_uberblock_sync(zio_t *zio, uberblock_t *ub, vdev_t *vd, int flags)
{
<<<<<<< HEAD
	uberblock_t *ubbuf;
	int c, l, n;

	for (c = 0; c < vd->vdev_children; c++)
=======
	for (int c = 0; c < vd->vdev_children; c++)
>>>>>>> temp
		vdev_uberblock_sync(zio, ub, vd->vdev_child[c], flags);

	if (!vd->vdev_ops->vdev_op_leaf)
		return;

	if (!vdev_writeable(vd))
		return;

<<<<<<< HEAD
	n = ub->ub_txg & (VDEV_UBERBLOCK_COUNT(vd) - 1);

	ubbuf = zio_buf_alloc(VDEV_UBERBLOCK_SIZE(vd));
	bzero(ubbuf, VDEV_UBERBLOCK_SIZE(vd));
	*ubbuf = *ub;

	for (l = 0; l < VDEV_LABELS; l++)
		vdev_label_write(zio, vd, l, ubbuf,
=======
	/* If the vdev was expanded, need to copy uberblock rings. */
	if (vd->vdev_state == VDEV_STATE_HEALTHY &&
	    vd->vdev_copy_uberblocks == B_TRUE) {
		vdev_copy_uberblocks(vd);
		vd->vdev_copy_uberblocks = B_FALSE;
	}

	int m = spa_multihost(vd->vdev_spa) ? MMP_BLOCKS_PER_LABEL : 0;
	int n = ub->ub_txg % (VDEV_UBERBLOCK_COUNT(vd) - m);

	/* Copy the uberblock_t into the ABD */
	abd_t *ub_abd = abd_alloc_for_io(VDEV_UBERBLOCK_SIZE(vd), B_TRUE);
	abd_zero(ub_abd, VDEV_UBERBLOCK_SIZE(vd));
	abd_copy_from_buf(ub_abd, ub, sizeof (uberblock_t));

	for (int l = 0; l < VDEV_LABELS; l++)
		vdev_label_write(zio, vd, l, ub_abd,
>>>>>>> temp
		    VDEV_UBERBLOCK_OFFSET(vd, n), VDEV_UBERBLOCK_SIZE(vd),
		    vdev_uberblock_sync_done, zio->io_private,
		    flags | ZIO_FLAG_DONT_PROPAGATE);

<<<<<<< HEAD
	zio_buf_free(ubbuf, VDEV_UBERBLOCK_SIZE(vd));
=======
	abd_free(ub_abd);
>>>>>>> temp
}

/* Sync the uberblocks to all vdevs in svd[] */
int
vdev_uberblock_sync_list(vdev_t **svd, int svdcount, uberblock_t *ub, int flags)
{
	spa_t *spa = svd[0]->vdev_spa;
	zio_t *zio;
	uint64_t good_writes = 0;
	int v;

	zio = zio_root(spa, NULL, &good_writes, flags);

	for (v = 0; v < svdcount; v++)
		vdev_uberblock_sync(zio, ub, svd[v], flags);

	(void) zio_wait(zio);

	/*
	 * Flush the uberblocks to disk.  This ensures that the odd labels
	 * are no longer needed (because the new uberblocks and the even
	 * labels are safely on disk), so it is safe to overwrite them.
	 */
	zio = zio_root(spa, NULL, NULL, flags);

	for (v = 0; v < svdcount; v++)
		zio_flush(zio, svd[v]);

	(void) zio_wait(zio);

	return (good_writes >= 1 ? 0 : EIO);
}

/*
 * On success, increment the count of good writes for our top-level vdev.
 */
static void
vdev_label_sync_done(zio_t *zio)
{
	uint64_t *good_writes = zio->io_private;

	if (zio->io_error == 0)
<<<<<<< HEAD
		atomic_add_64(good_writes, 1);
=======
		atomic_inc_64(good_writes);
>>>>>>> temp
}

/*
 * If there weren't enough good writes, indicate failure to the parent.
 */
static void
vdev_label_sync_top_done(zio_t *zio)
{
	uint64_t *good_writes = zio->io_private;

	if (*good_writes == 0)
		zio->io_error = SET_ERROR(EIO);

	kmem_free(good_writes, sizeof (uint64_t));
}

/*
 * We ignore errors for log and cache devices, simply free the private data.
 */
static void
vdev_label_sync_ignore_done(zio_t *zio)
{
	kmem_free(zio->io_private, sizeof (uint64_t));
}

/*
 * Write all even or odd labels to all leaves of the specified vdev.
 */
static void
vdev_label_sync(zio_t *zio, vdev_t *vd, int l, uint64_t txg, int flags)
{
	nvlist_t *label;
	vdev_phys_t *vp;
<<<<<<< HEAD
=======
	abd_t *vp_abd;
>>>>>>> temp
	char *buf;
	size_t buflen;
	int c;

	for (c = 0; c < vd->vdev_children; c++)
		vdev_label_sync(zio, vd->vdev_child[c], l, txg, flags);

	if (!vd->vdev_ops->vdev_op_leaf)
		return;

	if (!vdev_writeable(vd))
		return;

	/*
	 * Generate a label describing the top-level config to which we belong.
	 */
	label = spa_config_generate(vd->vdev_spa, vd, txg, B_FALSE);

<<<<<<< HEAD
	vp = zio_buf_alloc(sizeof (vdev_phys_t));
	bzero(vp, sizeof (vdev_phys_t));
=======
	vp_abd = abd_alloc_linear(sizeof (vdev_phys_t), B_TRUE);
	abd_zero(vp_abd, sizeof (vdev_phys_t));
	vp = abd_to_buf(vp_abd);
>>>>>>> temp

	buf = vp->vp_nvlist;
	buflen = sizeof (vp->vp_nvlist);

	if (!nvlist_pack(label, &buf, &buflen, NV_ENCODE_XDR, KM_SLEEP)) {
		for (; l < VDEV_LABELS; l += 2) {
<<<<<<< HEAD
			vdev_label_write(zio, vd, l, vp,
=======
			vdev_label_write(zio, vd, l, vp_abd,
>>>>>>> temp
			    offsetof(vdev_label_t, vl_vdev_phys),
			    sizeof (vdev_phys_t),
			    vdev_label_sync_done, zio->io_private,
			    flags | ZIO_FLAG_DONT_PROPAGATE);
		}
	}

<<<<<<< HEAD
	zio_buf_free(vp, sizeof (vdev_phys_t));
=======
	abd_free(vp_abd);
>>>>>>> temp
	nvlist_free(label);
}

int
vdev_label_sync_list(spa_t *spa, int l, uint64_t txg, int flags)
{
	list_t *dl = &spa->spa_config_dirty_list;
	vdev_t *vd;
	zio_t *zio;
	int error;

	/*
	 * Write the new labels to disk.
	 */
	zio = zio_root(spa, NULL, NULL, flags);

	for (vd = list_head(dl); vd != NULL; vd = list_next(dl, vd)) {
		uint64_t *good_writes;
		zio_t *vio;

		ASSERT(!vd->vdev_ishole);

		good_writes = kmem_zalloc(sizeof (uint64_t), KM_SLEEP);
		vio = zio_null(zio, spa, NULL,
		    (vd->vdev_islog || vd->vdev_aux != NULL) ?
		    vdev_label_sync_ignore_done : vdev_label_sync_top_done,
		    good_writes, flags);
		vdev_label_sync(vio, vd, l, txg, flags);
		zio_nowait(vio);
	}

	error = zio_wait(zio);

	/*
	 * Flush the new labels to disk.
	 */
	zio = zio_root(spa, NULL, NULL, flags);

	for (vd = list_head(dl); vd != NULL; vd = list_next(dl, vd))
		zio_flush(zio, vd);

	(void) zio_wait(zio);

	return (error);
}

/*
 * Sync the uberblock and any changes to the vdev configuration.
 *
 * The order of operations is carefully crafted to ensure that
 * if the system panics or loses power at any time, the state on disk
 * is still transactionally consistent.  The in-line comments below
 * describe the failure semantics at each stage.
 *
 * Moreover, vdev_config_sync() is designed to be idempotent: if it fails
 * at any time, you can just call it again, and it will resume its work.
 */
int
<<<<<<< HEAD
vdev_config_sync(vdev_t **svd, int svdcount, uint64_t txg, boolean_t tryhard)
=======
vdev_config_sync(vdev_t **svd, int svdcount, uint64_t txg)
>>>>>>> temp
{
	spa_t *spa = svd[0]->vdev_spa;
	uberblock_t *ub = &spa->spa_uberblock;
	vdev_t *vd;
	zio_t *zio;
<<<<<<< HEAD
	int error;
	int flags = ZIO_FLAG_CONFIG_WRITER | ZIO_FLAG_CANFAIL;

=======
	int error = 0;
	int flags = ZIO_FLAG_CONFIG_WRITER | ZIO_FLAG_CANFAIL;

retry:
>>>>>>> temp
	/*
	 * Normally, we don't want to try too hard to write every label and
	 * uberblock.  If there is a flaky disk, we don't want the rest of the
	 * sync process to block while we retry.  But if we can't write a
	 * single label out, we should retry with ZIO_FLAG_TRYHARD before
	 * bailing out and declaring the pool faulted.
	 */
<<<<<<< HEAD
	if (tryhard)
		flags |= ZIO_FLAG_TRYHARD;
=======
	if (error != 0) {
		if ((flags & ZIO_FLAG_TRYHARD) != 0)
			return (error);
		flags |= ZIO_FLAG_TRYHARD;
	}
>>>>>>> temp

	ASSERT(ub->ub_txg <= txg);

	/*
	 * If this isn't a resync due to I/O errors,
	 * and nothing changed in this transaction group,
	 * and the vdev configuration hasn't changed,
	 * then there's nothing to do.
	 */
<<<<<<< HEAD
	if (ub->ub_txg < txg &&
	    uberblock_update(ub, spa->spa_root_vdev, txg) == B_FALSE &&
	    list_is_empty(&spa->spa_config_dirty_list))
		return (0);
=======
	if (ub->ub_txg < txg) {
		boolean_t changed = uberblock_update(ub, spa->spa_root_vdev,
		    txg, spa->spa_mmp.mmp_delay);

		if (!changed && list_is_empty(&spa->spa_config_dirty_list))
			return (0);
	}
>>>>>>> temp

	if (txg > spa_freeze_txg(spa))
		return (0);

	ASSERT(txg <= spa->spa_final_txg);

	/*
	 * Flush the write cache of every disk that's been written to
	 * in this transaction group.  This ensures that all blocks
	 * written in this txg will be committed to stable storage
	 * before any uberblock that references them.
	 */
	zio = zio_root(spa, NULL, NULL, flags);

	for (vd = txg_list_head(&spa->spa_vdev_txg_list, TXG_CLEAN(txg)); vd;
	    vd = txg_list_next(&spa->spa_vdev_txg_list, vd, TXG_CLEAN(txg)))
		zio_flush(zio, vd);

	(void) zio_wait(zio);

	/*
	 * Sync out the even labels (L0, L2) for every dirty vdev.  If the
	 * system dies in the middle of this process, that's OK: all of the
	 * even labels that made it to disk will be newer than any uberblock,
	 * and will therefore be considered invalid.  The odd labels (L1, L3),
	 * which have not yet been touched, will still be valid.  We flush
	 * the new labels to disk to ensure that all even-label updates
	 * are committed to stable storage before the uberblock update.
	 */
	if ((error = vdev_label_sync_list(spa, 0, txg, flags)) != 0)
<<<<<<< HEAD
		return (error);
=======
		goto retry;
>>>>>>> temp

	/*
	 * Sync the uberblocks to all vdevs in svd[].
	 * If the system dies in the middle of this step, there are two cases
	 * to consider, and the on-disk state is consistent either way:
	 *
	 * (1)	If none of the new uberblocks made it to disk, then the
	 *	previous uberblock will be the newest, and the odd labels
	 *	(which had not yet been touched) will be valid with respect
	 *	to that uberblock.
	 *
	 * (2)	If one or more new uberblocks made it to disk, then they
	 *	will be the newest, and the even labels (which had all
	 *	been successfully committed) will be valid with respect
	 *	to the new uberblocks.
	 */
	if ((error = vdev_uberblock_sync_list(svd, svdcount, ub, flags)) != 0)
<<<<<<< HEAD
		return (error);
=======
		goto retry;


	if (spa_multihost(spa))
		mmp_update_uberblock(spa, ub);
>>>>>>> temp

	/*
	 * Sync out odd labels for every dirty vdev.  If the system dies
	 * in the middle of this process, the even labels and the new
	 * uberblocks will suffice to open the pool.  The next time
	 * the pool is opened, the first thing we'll do -- before any
	 * user data is modified -- is mark every vdev dirty so that
	 * all labels will be brought up to date.  We flush the new labels
	 * to disk to ensure that all odd-label updates are committed to
	 * stable storage before the next transaction group begins.
	 */
<<<<<<< HEAD
	return (vdev_label_sync_list(spa, 1, txg, flags));
=======
	if ((error = vdev_label_sync_list(spa, 1, txg, flags)) != 0)
		goto retry;

	return (0);
>>>>>>> temp
}
