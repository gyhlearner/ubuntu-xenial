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
 * Copyright (c) 2012, 2014 by Delphix. All rights reserved.
=======
 * Copyright (c) 2012, 2016 by Delphix. All rights reserved.
>>>>>>> temp
 * Copyright (c) 2013 by Saso Kiselkov. All rights reserved.
 */

#ifndef	_SYS_ARC_H
#define	_SYS_ARC_H

#include <sys/zfs_context.h>

#ifdef	__cplusplus
extern "C" {
#endif

#include <sys/zio.h>
#include <sys/dmu.h>
#include <sys/spa.h>
#include <sys/refcount.h>

/*
 * Used by arc_flush() to inform arc_evict_state() that it should evict
 * all available buffers from the arc state being passed in.
 */
#define	ARC_EVICT_ALL	-1ULL

<<<<<<< HEAD
=======
#define	HDR_SET_LSIZE(hdr, x) do { \
	ASSERT(IS_P2ALIGNED(x, 1U << SPA_MINBLOCKSHIFT)); \
	(hdr)->b_lsize = ((x) >> SPA_MINBLOCKSHIFT); \
_NOTE(CONSTCOND) } while (0)

#define	HDR_SET_PSIZE(hdr, x) do { \
	ASSERT(IS_P2ALIGNED((x), 1U << SPA_MINBLOCKSHIFT)); \
	(hdr)->b_psize = ((x) >> SPA_MINBLOCKSHIFT); \
_NOTE(CONSTCOND) } while (0)

#define	HDR_GET_LSIZE(hdr)	((hdr)->b_lsize << SPA_MINBLOCKSHIFT)
#define	HDR_GET_PSIZE(hdr)	((hdr)->b_psize << SPA_MINBLOCKSHIFT)

>>>>>>> temp
typedef struct arc_buf_hdr arc_buf_hdr_t;
typedef struct arc_buf arc_buf_t;
typedef struct arc_prune arc_prune_t;
typedef void arc_done_func_t(zio_t *zio, arc_buf_t *buf, void *private);
typedef void arc_prune_func_t(int64_t bytes, void *private);
<<<<<<< HEAD
typedef int arc_evict_func_t(void *private);
=======
>>>>>>> temp

/* Shared module parameters */
extern int zfs_arc_average_blocksize;

/* generic arc_done_func_t's which you can use */
arc_done_func_t arc_bcopy_func;
arc_done_func_t arc_getbuf_func;

/* generic arc_prune_func_t wrapper for callbacks */
struct arc_prune {
	arc_prune_func_t	*p_pfunc;
	void			*p_private;
	uint64_t		p_adjust;
	list_node_t		p_node;
	refcount_t		p_refcnt;
};

typedef enum arc_strategy {
	ARC_STRATEGY_META_ONLY		= 0, /* Evict only meta data buffers */
	ARC_STRATEGY_META_BALANCED	= 1, /* Evict data buffers if needed */
} arc_strategy_t;

typedef enum arc_flags
{
	/*
	 * Public flags that can be passed into the ARC by external consumers.
	 */
<<<<<<< HEAD
	ARC_FLAG_NONE			= 1 << 0,	/* No flags set */
	ARC_FLAG_WAIT			= 1 << 1,	/* perform sync I/O */
	ARC_FLAG_NOWAIT			= 1 << 2,	/* perform async I/O */
	ARC_FLAG_PREFETCH		= 1 << 3,	/* I/O is a prefetch */
	ARC_FLAG_CACHED			= 1 << 4,	/* I/O was in cache */
	ARC_FLAG_L2CACHE		= 1 << 5,	/* cache in L2ARC */
	ARC_FLAG_L2COMPRESS		= 1 << 6,	/* compress in L2ARC */
=======
	ARC_FLAG_WAIT			= 1 << 0,	/* perform sync I/O */
	ARC_FLAG_NOWAIT			= 1 << 1,	/* perform async I/O */
	ARC_FLAG_PREFETCH		= 1 << 2,	/* I/O is a prefetch */
	ARC_FLAG_CACHED			= 1 << 3,	/* I/O was in cache */
	ARC_FLAG_L2CACHE		= 1 << 4,	/* cache in L2ARC */
	ARC_FLAG_PREDICTIVE_PREFETCH	= 1 << 5,	/* I/O from zfetch */
	ARC_FLAG_PRESCIENT_PREFETCH     = 1 << 6,       /* long min lifespan */
>>>>>>> temp

	/*
	 * Private ARC flags.  These flags are private ARC only flags that
	 * will show up in b_flags in the arc_hdr_buf_t. These flags should
	 * only be set by ARC code.
	 */
	ARC_FLAG_IN_HASH_TABLE		= 1 << 7,	/* buffer is hashed */
	ARC_FLAG_IO_IN_PROGRESS		= 1 << 8,	/* I/O in progress */
	ARC_FLAG_IO_ERROR		= 1 << 9,	/* I/O failed for buf */
<<<<<<< HEAD
	ARC_FLAG_FREED_IN_READ		= 1 << 10,	/* freed during read */
	ARC_FLAG_BUF_AVAILABLE		= 1 << 11,	/* block not in use */
	ARC_FLAG_INDIRECT		= 1 << 12,	/* indirect block */
	ARC_FLAG_L2_WRITING		= 1 << 13,	/* write in progress */
	ARC_FLAG_L2_EVICTED		= 1 << 14,	/* evicted during I/O */
	ARC_FLAG_L2_WRITE_HEAD		= 1 << 15,	/* head of write list */
	/* indicates that the buffer contains metadata (otherwise, data) */
	ARC_FLAG_BUFC_METADATA		= 1 << 16,

	/* Flags specifying whether optional hdr struct fields are defined */
	ARC_FLAG_HAS_L1HDR		= 1 << 17,
	ARC_FLAG_HAS_L2HDR		= 1 << 18,
} arc_flags_t;

=======
	ARC_FLAG_INDIRECT		= 1 << 10,	/* indirect block */
	/* Indicates that block was read with ASYNC priority. */
	ARC_FLAG_PRIO_ASYNC_READ	= 1 << 11,
	ARC_FLAG_L2_WRITING		= 1 << 12,	/* write in progress */
	ARC_FLAG_L2_EVICTED		= 1 << 13,	/* evicted during I/O */
	ARC_FLAG_L2_WRITE_HEAD		= 1 << 14,	/* head of write list */
	/* indicates that the buffer contains metadata (otherwise, data) */
	ARC_FLAG_BUFC_METADATA		= 1 << 15,

	/* Flags specifying whether optional hdr struct fields are defined */
	ARC_FLAG_HAS_L1HDR		= 1 << 16,
	ARC_FLAG_HAS_L2HDR		= 1 << 17,

	/*
	 * Indicates the arc_buf_hdr_t's b_pdata matches the on-disk data.
	 * This allows the l2arc to use the blkptr's checksum to verify
	 * the data without having to store the checksum in the hdr.
	 */
	ARC_FLAG_COMPRESSED_ARC		= 1 << 18,
	ARC_FLAG_SHARED_DATA		= 1 << 19,

	/*
	 * The arc buffer's compression mode is stored in the top 7 bits of the
	 * flags field, so these dummy flags are included so that MDB can
	 * interpret the enum properly.
	 */
	ARC_FLAG_COMPRESS_0		= 1 << 24,
	ARC_FLAG_COMPRESS_1		= 1 << 25,
	ARC_FLAG_COMPRESS_2		= 1 << 26,
	ARC_FLAG_COMPRESS_3		= 1 << 27,
	ARC_FLAG_COMPRESS_4		= 1 << 28,
	ARC_FLAG_COMPRESS_5		= 1 << 29,
	ARC_FLAG_COMPRESS_6		= 1 << 30

} arc_flags_t;

typedef enum arc_buf_flags {
	ARC_BUF_FLAG_SHARED		= 1 << 0,
	ARC_BUF_FLAG_COMPRESSED		= 1 << 1
} arc_buf_flags_t;

>>>>>>> temp
struct arc_buf {
	arc_buf_hdr_t		*b_hdr;
	arc_buf_t		*b_next;
	kmutex_t		b_evict_lock;
	void			*b_data;
<<<<<<< HEAD
	arc_evict_func_t	*b_efunc;
	void			*b_private;
};

typedef enum arc_buf_contents {
=======
	arc_buf_flags_t		b_flags;
};

typedef enum arc_buf_contents {
	ARC_BUFC_INVALID,			/* invalid type */
>>>>>>> temp
	ARC_BUFC_DATA,				/* buffer contains data */
	ARC_BUFC_METADATA,			/* buffer contains metadata */
	ARC_BUFC_NUMTYPES
} arc_buf_contents_t;

/*
 * The following breakdows of arc_size exist for kstat only.
 */
typedef enum arc_space_type {
	ARC_SPACE_DATA,
	ARC_SPACE_META,
	ARC_SPACE_HDRS,
	ARC_SPACE_L2HDRS,
<<<<<<< HEAD
	ARC_SPACE_OTHER,
=======
	ARC_SPACE_DBUF,
	ARC_SPACE_DNODE,
	ARC_SPACE_BONUS,
>>>>>>> temp
	ARC_SPACE_NUMTYPES
} arc_space_type_t;

typedef enum arc_state_type {
	ARC_STATE_ANON,
	ARC_STATE_MRU,
	ARC_STATE_MRU_GHOST,
	ARC_STATE_MFU,
	ARC_STATE_MFU_GHOST,
	ARC_STATE_L2C_ONLY,
	ARC_STATE_NUMTYPES
} arc_state_type_t;

typedef struct arc_buf_info {
	arc_state_type_t	abi_state_type;
	arc_buf_contents_t	abi_state_contents;
	uint32_t		abi_flags;
<<<<<<< HEAD
	uint32_t		abi_datacnt;
=======
	uint32_t		abi_bufcnt;
>>>>>>> temp
	uint64_t		abi_size;
	uint64_t		abi_spa;
	uint64_t		abi_access;
	uint32_t		abi_mru_hits;
	uint32_t		abi_mru_ghost_hits;
	uint32_t		abi_mfu_hits;
	uint32_t		abi_mfu_ghost_hits;
	uint32_t		abi_l2arc_hits;
	uint32_t		abi_holds;
	uint64_t		abi_l2arc_dattr;
	uint64_t		abi_l2arc_asize;
	enum zio_compress	abi_l2arc_compress;
} arc_buf_info_t;

void arc_space_consume(uint64_t space, arc_space_type_t type);
void arc_space_return(uint64_t space, arc_space_type_t type);
<<<<<<< HEAD
arc_buf_t *arc_buf_alloc(spa_t *spa, uint64_t size, void *tag,
    arc_buf_contents_t type);
arc_buf_t *arc_loan_buf(spa_t *spa, uint64_t size);
void arc_return_buf(arc_buf_t *buf, void *tag);
void arc_loan_inuse_buf(arc_buf_t *buf, void *tag);
void arc_buf_add_ref(arc_buf_t *buf, void *tag);
boolean_t arc_buf_remove_ref(arc_buf_t *buf, void *tag);
void arc_buf_info(arc_buf_t *buf, arc_buf_info_t *abi, int state_index);
uint64_t arc_buf_size(arc_buf_t *buf);
=======
boolean_t arc_is_metadata(arc_buf_t *buf);
enum zio_compress arc_get_compression(arc_buf_t *buf);
int arc_decompress(arc_buf_t *buf);
arc_buf_t *arc_alloc_buf(spa_t *spa, void *tag, arc_buf_contents_t type,
    int32_t size);
arc_buf_t *arc_alloc_compressed_buf(spa_t *spa, void *tag,
    uint64_t psize, uint64_t lsize, enum zio_compress compression_type);
arc_buf_t *arc_loan_buf(spa_t *spa, boolean_t is_metadata, int size);
arc_buf_t *arc_loan_compressed_buf(spa_t *spa, uint64_t psize, uint64_t lsize,
    enum zio_compress compression_type);
void arc_return_buf(arc_buf_t *buf, void *tag);
void arc_loan_inuse_buf(arc_buf_t *buf, void *tag);
void arc_buf_destroy(arc_buf_t *buf, void *tag);
void arc_buf_info(arc_buf_t *buf, arc_buf_info_t *abi, int state_index);
uint64_t arc_buf_size(arc_buf_t *buf);
uint64_t arc_buf_lsize(arc_buf_t *buf);
void arc_buf_access(arc_buf_t *buf);
>>>>>>> temp
void arc_release(arc_buf_t *buf, void *tag);
int arc_released(arc_buf_t *buf);
void arc_buf_sigsegv(int sig, siginfo_t *si, void *unused);
void arc_buf_freeze(arc_buf_t *buf);
void arc_buf_thaw(arc_buf_t *buf);
<<<<<<< HEAD
boolean_t arc_buf_eviction_needed(arc_buf_t *buf);
=======
>>>>>>> temp
#ifdef ZFS_DEBUG
int arc_referenced(arc_buf_t *buf);
#endif

int arc_read(zio_t *pio, spa_t *spa, const blkptr_t *bp,
    arc_done_func_t *done, void *private, zio_priority_t priority, int flags,
    arc_flags_t *arc_flags, const zbookmark_phys_t *zb);
zio_t *arc_write(zio_t *pio, spa_t *spa, uint64_t txg,
<<<<<<< HEAD
    blkptr_t *bp, arc_buf_t *buf, boolean_t l2arc, boolean_t l2arc_compress,
    const zio_prop_t *zp, arc_done_func_t *ready, arc_done_func_t *physdone,
    arc_done_func_t *done, void *private, zio_priority_t priority,
    int zio_flags, const zbookmark_phys_t *zb);
=======
    blkptr_t *bp, arc_buf_t *buf, boolean_t l2arc, const zio_prop_t *zp,
    arc_done_func_t *ready, arc_done_func_t *child_ready,
    arc_done_func_t *physdone, arc_done_func_t *done,
    void *private, zio_priority_t priority, int zio_flags,
    const zbookmark_phys_t *zb);
>>>>>>> temp

arc_prune_t *arc_add_prune_callback(arc_prune_func_t *func, void *private);
void arc_remove_prune_callback(arc_prune_t *p);
void arc_freed(spa_t *spa, const blkptr_t *bp);

<<<<<<< HEAD
void arc_set_callback(arc_buf_t *buf, arc_evict_func_t *func, void *private);
boolean_t arc_clear_callback(arc_buf_t *buf);

=======
>>>>>>> temp
void arc_flush(spa_t *spa, boolean_t retry);
void arc_tempreserve_clear(uint64_t reserve);
int arc_tempreserve_space(uint64_t reserve, uint64_t txg);

<<<<<<< HEAD
=======
uint64_t arc_target_bytes(void);
>>>>>>> temp
void arc_init(void);
void arc_fini(void);

/*
 * Level 2 ARC
 */

void l2arc_add_vdev(spa_t *spa, vdev_t *vd);
void l2arc_remove_vdev(vdev_t *vd);
boolean_t l2arc_vdev_present(vdev_t *vd);
void l2arc_init(void);
void l2arc_fini(void);
void l2arc_start(void);
void l2arc_stop(void);

#ifndef _KERNEL
extern boolean_t arc_watch;
#endif

#ifdef	__cplusplus
}
#endif

#endif /* _SYS_ARC_H */
