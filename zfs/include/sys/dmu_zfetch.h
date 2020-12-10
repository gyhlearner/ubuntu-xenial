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
 * Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

<<<<<<< HEAD
#ifndef	_DFETCH_H
#define	_DFETCH_H
=======
/*
 * Copyright (c) 2014 by Delphix. All rights reserved.
 */

#ifndef	_DMU_ZFETCH_H
#define	_DMU_ZFETCH_H
>>>>>>> temp

#include <sys/zfs_context.h>

#ifdef	__cplusplus
extern "C" {
#endif

extern unsigned long	zfetch_array_rd_sz;

struct dnode;				/* so we can reference dnode */

<<<<<<< HEAD
typedef enum zfetch_dirn {
	ZFETCH_FORWARD = 1,		/* prefetch increasing block numbers */
	ZFETCH_BACKWARD	= -1		/* prefetch decreasing block numbers */
} zfetch_dirn_t;

typedef struct zstream {
	uint64_t	zst_offset;	/* offset of starting block in range */
	uint64_t	zst_len;	/* length of range, in blocks */
	zfetch_dirn_t	zst_direction;	/* direction of prefetch */
	uint64_t	zst_stride;	/* length of stride, in blocks */
	uint64_t	zst_ph_offset;	/* prefetch offset, in blocks */
	uint64_t	zst_cap;	/* prefetch limit (cap), in blocks */
	kmutex_t	zst_lock;	/* protects stream */
	clock_t		zst_last;	/* lbolt of last prefetch */
	list_node_t	zst_node;	/* next zstream here */
=======
typedef struct zstream {
	uint64_t	zs_blkid;	/* expect next access at this blkid */
	uint64_t	zs_pf_blkid;	/* next block to prefetch */

	/*
	 * We will next prefetch the L1 indirect block of this level-0
	 * block id.
	 */
	uint64_t	zs_ipf_blkid;

	kmutex_t	zs_lock;	/* protects stream */
	hrtime_t	zs_atime;	/* time last prefetch issued */
	list_node_t	zs_node;	/* link for zf_stream */
>>>>>>> temp
} zstream_t;

typedef struct zfetch {
	krwlock_t	zf_rwlock;	/* protects zfetch structure */
<<<<<<< HEAD
	list_t		zf_stream;	/* AVL tree of zstream_t's */
	struct dnode	*zf_dnode;	/* dnode that owns this zfetch */
	uint32_t	zf_stream_cnt;	/* # of active streams */
	uint64_t	zf_alloc_fail;	/* # of failed attempts to alloc strm */
=======
	list_t		zf_stream;	/* list of zstream_t's */
	struct dnode	*zf_dnode;	/* dnode that owns this zfetch */
>>>>>>> temp
} zfetch_t;

void		zfetch_init(void);
void		zfetch_fini(void);

void		dmu_zfetch_init(zfetch_t *, struct dnode *);
<<<<<<< HEAD
void		dmu_zfetch_rele(zfetch_t *);
void		dmu_zfetch(zfetch_t *, uint64_t, uint64_t, int);
=======
void		dmu_zfetch_fini(zfetch_t *);
void		dmu_zfetch(zfetch_t *, uint64_t, uint64_t, boolean_t);
>>>>>>> temp


#ifdef	__cplusplus
}
#endif

<<<<<<< HEAD
#endif	/* _DFETCH_H */
=======
#endif	/* _DMU_ZFETCH_H */
>>>>>>> temp
