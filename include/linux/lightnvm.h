/* SPDX-License-Identifier: GPL-2.0 */
#ifndef NVM_H
#define NVM_H

<<<<<<< HEAD
#include <linux/types.h>
=======
#include <linux/blkdev.h>
#include <linux/types.h>
#include <uapi/linux/lightnvm.h>
>>>>>>> temp

enum {
	NVM_IO_OK = 0,
	NVM_IO_REQUEUE = 1,
	NVM_IO_DONE = 2,
	NVM_IO_ERR = 3,

	NVM_IOTYPE_NONE = 0,
	NVM_IOTYPE_GC = 1,
};

#define NVM_BLK_BITS (16)
#define NVM_PG_BITS  (16)
#define NVM_SEC_BITS (8)
#define NVM_PL_BITS  (8)
#define NVM_LUN_BITS (8)
<<<<<<< HEAD
#define NVM_CH_BITS  (8)
=======
#define NVM_CH_BITS  (7)
>>>>>>> temp

struct ppa_addr {
	/* Generic structure for all addresses */
	union {
		struct {
			u64 blk		: NVM_BLK_BITS;
			u64 pg		: NVM_PG_BITS;
			u64 sec		: NVM_SEC_BITS;
			u64 pl		: NVM_PL_BITS;
			u64 lun		: NVM_LUN_BITS;
			u64 ch		: NVM_CH_BITS;
<<<<<<< HEAD
		} g;

=======
			u64 reserved	: 1;
		} g;

		struct {
			u64 line	: 63;
			u64 is_cached	: 1;
		} c;

>>>>>>> temp
		u64 ppa;
	};
};

struct nvm_rq;
struct nvm_id;
struct nvm_dev;
<<<<<<< HEAD

typedef int (nvm_l2p_update_fn)(u64, u32, __le64 *, void *);
typedef int (nvm_bb_update_fn)(struct ppa_addr, int, u8 *, void *);
typedef int (nvm_id_fn)(struct nvm_dev *, struct nvm_id *);
typedef int (nvm_get_l2p_tbl_fn)(struct nvm_dev *, u64, u32,
				nvm_l2p_update_fn *, void *);
typedef int (nvm_op_bb_tbl_fn)(struct nvm_dev *, struct ppa_addr, int,
				nvm_bb_update_fn *, void *);
typedef int (nvm_op_set_bb_fn)(struct nvm_dev *, struct nvm_rq *, int);
typedef int (nvm_submit_io_fn)(struct nvm_dev *, struct nvm_rq *);
typedef int (nvm_erase_blk_fn)(struct nvm_dev *, struct nvm_rq *);
=======
struct nvm_tgt_dev;

typedef int (nvm_l2p_update_fn)(u64, u32, __le64 *, void *);
typedef int (nvm_id_fn)(struct nvm_dev *, struct nvm_id *);
typedef int (nvm_get_l2p_tbl_fn)(struct nvm_dev *, u64, u32,
				nvm_l2p_update_fn *, void *);
typedef int (nvm_op_bb_tbl_fn)(struct nvm_dev *, struct ppa_addr, u8 *);
typedef int (nvm_op_set_bb_fn)(struct nvm_dev *, struct ppa_addr *, int, int);
typedef int (nvm_submit_io_fn)(struct nvm_dev *, struct nvm_rq *);
typedef int (nvm_submit_io_sync_fn)(struct nvm_dev *, struct nvm_rq *);
>>>>>>> temp
typedef void *(nvm_create_dma_pool_fn)(struct nvm_dev *, char *);
typedef void (nvm_destroy_dma_pool_fn)(void *);
typedef void *(nvm_dev_dma_alloc_fn)(struct nvm_dev *, void *, gfp_t,
								dma_addr_t *);
typedef void (nvm_dev_dma_free_fn)(void *, void*, dma_addr_t);

struct nvm_dev_ops {
	nvm_id_fn		*identity;
	nvm_get_l2p_tbl_fn	*get_l2p_tbl;
	nvm_op_bb_tbl_fn	*get_bb_tbl;
	nvm_op_set_bb_fn	*set_bb_tbl;

	nvm_submit_io_fn	*submit_io;
<<<<<<< HEAD
	nvm_erase_blk_fn	*erase_block;
=======
	nvm_submit_io_sync_fn	*submit_io_sync;
>>>>>>> temp

	nvm_create_dma_pool_fn	*create_dma_pool;
	nvm_destroy_dma_pool_fn	*destroy_dma_pool;
	nvm_dev_dma_alloc_fn	*dev_dma_alloc;
	nvm_dev_dma_free_fn	*dev_dma_free;

	unsigned int		max_phys_sect;
};

<<<<<<< HEAD


=======
>>>>>>> temp
#ifdef CONFIG_NVM

#include <linux/blkdev.h>
#include <linux/file.h>
#include <linux/dmapool.h>
#include <uapi/linux/lightnvm.h>

enum {
	/* HW Responsibilities */
	NVM_RSP_L2P	= 1 << 0,
	NVM_RSP_ECC	= 1 << 1,

	/* Physical Adressing Mode */
	NVM_ADDRMODE_LINEAR	= 0,
	NVM_ADDRMODE_CHANNEL	= 1,

	/* Plane programming mode for LUN */
	NVM_PLANE_SINGLE	= 1,
	NVM_PLANE_DOUBLE	= 2,
	NVM_PLANE_QUAD		= 4,

	/* Status codes */
	NVM_RSP_SUCCESS		= 0x0,
	NVM_RSP_NOT_CHANGEABLE	= 0x1,
	NVM_RSP_ERR_FAILWRITE	= 0x40ff,
	NVM_RSP_ERR_EMPTYPAGE	= 0x42ff,
	NVM_RSP_ERR_FAILECC	= 0x4281,
	NVM_RSP_ERR_FAILCRC	= 0x4004,
	NVM_RSP_WARN_HIGHECC	= 0x4700,

	/* Device opcodes */
	NVM_OP_HBREAD		= 0x02,
	NVM_OP_HBWRITE		= 0x81,
	NVM_OP_PWRITE		= 0x91,
	NVM_OP_PREAD		= 0x92,
	NVM_OP_ERASE		= 0x90,

	/* PPA Command Flags */
	NVM_IO_SNGL_ACCESS	= 0x0,
	NVM_IO_DUAL_ACCESS	= 0x1,
	NVM_IO_QUAD_ACCESS	= 0x2,

	/* NAND Access Modes */
	NVM_IO_SUSPEND		= 0x80,
	NVM_IO_SLC_MODE		= 0x100,
	NVM_IO_SCRAMBLE_ENABLE	= 0x200,

	/* Block Types */
	NVM_BLK_T_FREE		= 0x0,
	NVM_BLK_T_BAD		= 0x1,
	NVM_BLK_T_GRWN_BAD	= 0x2,
	NVM_BLK_T_DEV		= 0x4,
	NVM_BLK_T_HOST		= 0x8,

	/* Memory capabilities */
	NVM_ID_CAP_SLC		= 0x1,
	NVM_ID_CAP_CMD_SUSPEND	= 0x2,
	NVM_ID_CAP_SCRAMBLE	= 0x4,
	NVM_ID_CAP_ENCRYPT	= 0x8,

	/* Memory types */
	NVM_ID_FMTYPE_SLC	= 0,
	NVM_ID_FMTYPE_MLC	= 1,
<<<<<<< HEAD
=======

	/* Device capabilities */
	NVM_ID_DCAP_BBLKMGMT	= 0x1,
	NVM_UD_DCAP_ECC		= 0x2,
>>>>>>> temp
};

struct nvm_id_lp_mlc {
	u16	num_pairs;
	u8	pairs[886];
};

struct nvm_id_lp_tbl {
	__u8	id[8];
	struct nvm_id_lp_mlc mlc;
};

struct nvm_id_group {
	u8	mtype;
	u8	fmtype;
	u8	num_ch;
	u8	num_lun;
	u8	num_pln;
	u16	num_blk;
	u16	num_pg;
	u16	fpg_sz;
	u16	csecs;
	u16	sos;
	u32	trdt;
	u32	trdm;
	u32	tprt;
	u32	tprm;
	u32	tbet;
	u32	tbem;
	u32	mpos;
	u32	mccap;
	u16	cpar;

	struct nvm_id_lp_tbl lptbl;
};

struct nvm_addr_format {
	u8	ch_offset;
	u8	ch_len;
	u8	lun_offset;
	u8	lun_len;
	u8	pln_offset;
	u8	pln_len;
	u8	blk_offset;
	u8	blk_len;
	u8	pg_offset;
	u8	pg_len;
	u8	sect_offset;
	u8	sect_len;
};

struct nvm_id {
	u8	ver_id;
	u8	vmnt;
	u32	cap;
	u32	dom;
	struct nvm_addr_format ppaf;
	struct nvm_id_group grp;
} __packed;

struct nvm_target {
	struct list_head list;
	struct nvm_tgt_dev *dev;
	struct nvm_tgt_type *type;
	struct gendisk *disk;
};

#define ADDR_EMPTY (~0ULL)

#define NVM_VERSION_MAJOR 1
#define NVM_VERSION_MINOR 0
#define NVM_VERSION_PATCH 0

struct nvm_rq;
typedef void (nvm_end_io_fn)(struct nvm_rq *);

struct nvm_rq {
	struct nvm_tgt_dev *dev;

	struct bio *bio;

	union {
		struct ppa_addr ppa_addr;
		dma_addr_t dma_ppa_list;
	};

	struct ppa_addr *ppa_list;

	void *meta_list;
	dma_addr_t dma_meta_list;

	struct completion *wait;
	nvm_end_io_fn *end_io;

	struct completion *wait;
	nvm_end_io_fn *end_io;

	uint8_t opcode;
	uint16_t nr_ppas;
	uint16_t flags;

<<<<<<< HEAD
	int error;
=======
	u64 ppa_status; /* ppa media status */
	int error;

	void *private;
>>>>>>> temp
};

static inline struct nvm_rq *nvm_rq_from_pdu(void *pdu)
{
	return pdu - sizeof(struct nvm_rq);
}

static inline void *nvm_rq_to_pdu(struct nvm_rq *rqdata)
{
	return rqdata + 1;
}

<<<<<<< HEAD
struct nvm_block;

struct nvm_lun {
	int id;

	int lun_id;
	int chnl_id;

	/* It is up to the target to mark blocks as closed. If the target does
	 * not do it, all blocks are marked as open, and nr_open_blocks
	 * represents the number of blocks in use
	 */
	unsigned int nr_open_blocks;	/* Number of used, writable blocks */
	unsigned int nr_closed_blocks;	/* Number of used, read-only blocks */
	unsigned int nr_free_blocks;	/* Number of unused blocks */
	unsigned int nr_bad_blocks;	/* Number of bad blocks */

	spinlock_t lock;

	struct nvm_block *blocks;
};

enum {
	NVM_BLK_ST_FREE =	0x1,	/* Free block */
	NVM_BLK_ST_OPEN =	0x2,	/* Open block - read-write */
	NVM_BLK_ST_CLOSED =	0x4,	/* Closed block - read-only */
	NVM_BLK_ST_BAD =	0x8,	/* Bad block */
};

struct nvm_block {
	struct list_head list;
	struct nvm_lun *lun;
	unsigned long id;

	void *priv;
	int state;
};

/* system block cpu representation */
struct nvm_sb_info {
	unsigned long		seqnr;
	unsigned long		erase_cnt;
	unsigned int		version;
	char			mmtype[NVM_MMTYPE_LEN];
	struct ppa_addr		fs_ppa;
};

struct nvm_dev {
	struct nvm_dev_ops *ops;

	struct list_head devices;
	struct list_head online_targets;

	/* Media manager */
	struct nvmm_type *mt;
	void *mp;

	/* System blocks */
	struct nvm_sb_info sb;

	/* Device information */
=======
enum {
	NVM_BLK_ST_FREE =	0x1,	/* Free block */
	NVM_BLK_ST_TGT =	0x2,	/* Block in use by target */
	NVM_BLK_ST_BAD =	0x8,	/* Bad block */
};

/* Device generic information */
struct nvm_geo {
>>>>>>> temp
	int nr_chnls;
	int nr_luns;
	int luns_per_chnl; /* -1 if channels are not symmetric */
	int nr_planes;
	int sec_per_pg; /* only sectors for a single page */
	int pgs_per_blk;
	int blks_per_lun;
	int fpg_size;
	int pfpg_size; /* size of buffer if all pages are to be read */
	int sec_size;
	int oob_size;
	int mccap;
	struct nvm_addr_format ppaf;

	/* Calculated/Cached values. These do not reflect the actual usable
	 * blocks at run-time.
	 */
	int max_rq_size;
	int plane_mode; /* drive device in single, double or quad mode */

	int sec_per_pl; /* all sectors across planes */
	int sec_per_blk;
	int sec_per_lun;
};

<<<<<<< HEAD
	/* lower page table */
	int lps_per_blk;
	int *lptbl;

	unsigned long total_pages;
	unsigned long total_blocks;
	int nr_luns;
	unsigned max_pages_per_blk;
=======
/* sub-device structure */
struct nvm_tgt_dev {
	/* Device information */
	struct nvm_geo geo;

	/* Base ppas for target LUNs */
	struct ppa_addr *luns;
>>>>>>> temp

	sector_t total_secs;

	struct nvm_id identity;
	struct request_queue *q;

	struct nvm_dev *parent;
	void *map;
};

struct nvm_dev {
	struct nvm_dev_ops *ops;

	struct list_head devices;

	/* Device information */
	struct nvm_geo geo;

	  /* lower page table */
	int lps_per_blk;
	int *lptbl;

	unsigned long total_secs;

	unsigned long *lun_map;
	void *dma_pool;

	struct nvm_id identity;

	/* Backend device */
	struct request_queue *q;
	char name[DISK_NAME_LEN];
<<<<<<< HEAD

	struct mutex mlock;
=======
	void *private_data;

	void *rmap;

	struct mutex mlock;
	spinlock_t lock;

	/* target management */
	struct list_head area_list;
	struct list_head targets;
>>>>>>> temp
};

static inline struct ppa_addr linear_to_generic_addr(struct nvm_geo *geo,
						     u64 pba)
{
	struct ppa_addr l;
	int secs, pgs, blks, luns;
	sector_t ppa = pba;

	l.ppa = 0;

	div_u64_rem(ppa, geo->sec_per_pg, &secs);
	l.g.sec = secs;

	sector_div(ppa, geo->sec_per_pg);
	div_u64_rem(ppa, geo->pgs_per_blk, &pgs);
	l.g.pg = pgs;

	sector_div(ppa, geo->pgs_per_blk);
	div_u64_rem(ppa, geo->blks_per_lun, &blks);
	l.g.blk = blks;

	sector_div(ppa, geo->blks_per_lun);
	div_u64_rem(ppa, geo->luns_per_chnl, &luns);
	l.g.lun = luns;

	sector_div(ppa, geo->luns_per_chnl);
	l.g.ch = ppa;

	return l;
}

static inline struct ppa_addr generic_to_dev_addr(struct nvm_tgt_dev *tgt_dev,
						  struct ppa_addr r)
{
	struct nvm_geo *geo = &tgt_dev->geo;
	struct ppa_addr l;

<<<<<<< HEAD
=======
	l.ppa = ((u64)r.g.blk) << geo->ppaf.blk_offset;
	l.ppa |= ((u64)r.g.pg) << geo->ppaf.pg_offset;
	l.ppa |= ((u64)r.g.sec) << geo->ppaf.sect_offset;
	l.ppa |= ((u64)r.g.pl) << geo->ppaf.pln_offset;
	l.ppa |= ((u64)r.g.lun) << geo->ppaf.lun_offset;
	l.ppa |= ((u64)r.g.ch) << geo->ppaf.ch_offset;

	return l;
}

static inline struct ppa_addr dev_to_generic_addr(struct nvm_tgt_dev *tgt_dev,
						  struct ppa_addr r)
{
	struct nvm_geo *geo = &tgt_dev->geo;
	struct ppa_addr l;

>>>>>>> temp
	l.ppa = 0;
	/*
	 * (r.ppa << X offset) & X len bitmask. X eq. blk, pg, etc.
	 */
	l.g.blk = (r.ppa >> geo->ppaf.blk_offset) &
					(((1 << geo->ppaf.blk_len) - 1));
	l.g.pg |= (r.ppa >> geo->ppaf.pg_offset) &
					(((1 << geo->ppaf.pg_len) - 1));
	l.g.sec |= (r.ppa >> geo->ppaf.sect_offset) &
					(((1 << geo->ppaf.sect_len) - 1));
	l.g.pl |= (r.ppa >> geo->ppaf.pln_offset) &
					(((1 << geo->ppaf.pln_len) - 1));
	l.g.lun |= (r.ppa >> geo->ppaf.lun_offset) &
					(((1 << geo->ppaf.lun_len) - 1));
	l.g.ch |= (r.ppa >> geo->ppaf.ch_offset) &
					(((1 << geo->ppaf.ch_len) - 1));

	return l;
}

static inline int ppa_empty(struct ppa_addr ppa_addr)
{
	return (ppa_addr.ppa == ADDR_EMPTY);
}

static inline void ppa_set_empty(struct ppa_addr *ppa_addr)
{
	ppa_addr->ppa = ADDR_EMPTY;
}

static inline int ppa_cmp_blk(struct ppa_addr ppa1, struct ppa_addr ppa2)
{
	if (ppa_empty(ppa1) || ppa_empty(ppa2))
		return 0;

	return ((ppa1.g.ch == ppa2.g.ch) && (ppa1.g.lun == ppa2.g.lun) &&
					(ppa1.g.blk == ppa2.g.blk));
}

static inline int ppa_to_slc(struct nvm_dev *dev, int slc_pg)
{
	return dev->lptbl[slc_pg];
}

typedef blk_qc_t (nvm_tgt_make_rq_fn)(struct request_queue *, struct bio *);
typedef sector_t (nvm_tgt_capacity_fn)(void *);
<<<<<<< HEAD
typedef void *(nvm_tgt_init_fn)(struct nvm_dev *, struct gendisk *, int, int);
=======
typedef void *(nvm_tgt_init_fn)(struct nvm_tgt_dev *, struct gendisk *,
				int flags);
>>>>>>> temp
typedef void (nvm_tgt_exit_fn)(void *);
typedef int (nvm_tgt_sysfs_init_fn)(struct gendisk *);
typedef void (nvm_tgt_sysfs_exit_fn)(struct gendisk *);

struct nvm_tgt_type {
	const char *name;
	unsigned int version[3];

	/* target entry points */
	nvm_tgt_make_rq_fn *make_rq;
	nvm_tgt_capacity_fn *capacity;
<<<<<<< HEAD
	nvm_end_io_fn *end_io;
=======
>>>>>>> temp

	/* module-specific init/teardown */
	nvm_tgt_init_fn *init;
	nvm_tgt_exit_fn *exit;

	/* sysfs */
	nvm_tgt_sysfs_init_fn *sysfs_init;
	nvm_tgt_sysfs_exit_fn *sysfs_exit;

	/* For internal use */
	struct list_head list;
	struct module *owner;
};

extern int nvm_register_tgt_type(struct nvm_tgt_type *);
extern void nvm_unregister_tgt_type(struct nvm_tgt_type *);

extern void *nvm_dev_dma_alloc(struct nvm_dev *, gfp_t, dma_addr_t *);
extern void nvm_dev_dma_free(struct nvm_dev *, void *, dma_addr_t);

<<<<<<< HEAD
typedef int (nvmm_register_fn)(struct nvm_dev *);
typedef void (nvmm_unregister_fn)(struct nvm_dev *);
typedef struct nvm_block *(nvmm_get_blk_fn)(struct nvm_dev *,
					      struct nvm_lun *, unsigned long);
typedef void (nvmm_put_blk_fn)(struct nvm_dev *, struct nvm_block *);
typedef int (nvmm_open_blk_fn)(struct nvm_dev *, struct nvm_block *);
typedef int (nvmm_close_blk_fn)(struct nvm_dev *, struct nvm_block *);
typedef void (nvmm_flush_blk_fn)(struct nvm_dev *, struct nvm_block *);
typedef int (nvmm_submit_io_fn)(struct nvm_dev *, struct nvm_rq *);
typedef int (nvmm_erase_blk_fn)(struct nvm_dev *, struct nvm_block *,
								unsigned long);
typedef struct nvm_lun *(nvmm_get_lun_fn)(struct nvm_dev *, int);
typedef void (nvmm_lun_info_print_fn)(struct nvm_dev *);

struct nvmm_type {
	const char *name;
	unsigned int version[3];

	nvmm_register_fn *register_mgr;
	nvmm_unregister_fn *unregister_mgr;

	/* Block administration callbacks */
	nvmm_get_blk_fn *get_blk_unlocked;
	nvmm_put_blk_fn *put_blk_unlocked;
	nvmm_get_blk_fn *get_blk;
	nvmm_put_blk_fn *put_blk;
	nvmm_open_blk_fn *open_blk;
	nvmm_close_blk_fn *close_blk;
	nvmm_flush_blk_fn *flush_blk;

	nvmm_submit_io_fn *submit_io;
	nvmm_erase_blk_fn *erase_blk;

	/* Configuration management */
	nvmm_get_lun_fn *get_lun;

	/* Statistics */
	nvmm_lun_info_print_fn *lun_info_print;
	struct list_head list;
};

extern int nvm_register_mgr(struct nvmm_type *);
extern void nvm_unregister_mgr(struct nvmm_type *);

extern struct nvm_block *nvm_get_blk_unlocked(struct nvm_dev *,
					struct nvm_lun *, unsigned long);
extern void nvm_put_blk_unlocked(struct nvm_dev *, struct nvm_block *);

extern struct nvm_block *nvm_get_blk(struct nvm_dev *, struct nvm_lun *,
								unsigned long);
extern void nvm_put_blk(struct nvm_dev *, struct nvm_block *);

extern int nvm_register(struct request_queue *, char *,
						struct nvm_dev_ops *);
extern void nvm_unregister(char *);

extern int nvm_submit_io(struct nvm_dev *, struct nvm_rq *);
extern void nvm_generic_to_addr_mode(struct nvm_dev *, struct nvm_rq *);
extern void nvm_addr_to_generic_mode(struct nvm_dev *, struct nvm_rq *);
extern int nvm_set_rqd_ppalist(struct nvm_dev *, struct nvm_rq *,
							struct ppa_addr *, int);
extern void nvm_free_rqd_ppalist(struct nvm_dev *, struct nvm_rq *);
extern int nvm_erase_ppa(struct nvm_dev *, struct ppa_addr *, int);
extern int nvm_erase_blk(struct nvm_dev *, struct nvm_block *);
extern void nvm_end_io(struct nvm_rq *, int);
extern int nvm_submit_ppa(struct nvm_dev *, struct ppa_addr *, int, int, int,
								void *, int);

/* sysblk.c */
#define NVM_SYSBLK_MAGIC 0x4E564D53 /* "NVMS" */

/* system block on disk representation */
struct nvm_system_block {
	__be32			magic;		/* magic signature */
	__be32			seqnr;		/* sequence number */
	__be32			erase_cnt;	/* erase count */
	__be16			version;	/* version number */
	u8			mmtype[NVM_MMTYPE_LEN]; /* media manager name */
	__be64			fs_ppa;		/* PPA for media manager
						 * superblock */
};

extern int nvm_get_sysblock(struct nvm_dev *, struct nvm_sb_info *);
extern int nvm_update_sysblock(struct nvm_dev *, struct nvm_sb_info *);
extern int nvm_init_sysblock(struct nvm_dev *, struct nvm_sb_info *);

extern int nvm_dev_factory(struct nvm_dev *, int flags);
=======
extern struct nvm_dev *nvm_alloc_dev(int);
extern int nvm_register(struct nvm_dev *);
extern void nvm_unregister(struct nvm_dev *);

extern int nvm_set_tgt_bb_tbl(struct nvm_tgt_dev *, struct ppa_addr *,
			      int, int);
extern int nvm_max_phys_sects(struct nvm_tgt_dev *);
extern int nvm_submit_io(struct nvm_tgt_dev *, struct nvm_rq *);
extern int nvm_submit_io_sync(struct nvm_tgt_dev *, struct nvm_rq *);
extern int nvm_erase_sync(struct nvm_tgt_dev *, struct ppa_addr *, int);
extern int nvm_get_l2p_tbl(struct nvm_tgt_dev *, u64, u32, nvm_l2p_update_fn *,
			   void *);
extern int nvm_get_area(struct nvm_tgt_dev *, sector_t *, sector_t);
extern void nvm_put_area(struct nvm_tgt_dev *, sector_t);
extern void nvm_end_io(struct nvm_rq *);
extern int nvm_bb_tbl_fold(struct nvm_dev *, u8 *, int);
extern int nvm_get_tgt_bb_tbl(struct nvm_tgt_dev *, struct ppa_addr, u8 *);

extern void nvm_part_to_tgt(struct nvm_dev *, sector_t *, int);

>>>>>>> temp
#else /* CONFIG_NVM */
struct nvm_dev_ops;

static inline struct nvm_dev *nvm_alloc_dev(int node)
{
	return ERR_PTR(-EINVAL);
}
static inline int nvm_register(struct nvm_dev *dev)
{
	return -EINVAL;
}
static inline void nvm_unregister(struct nvm_dev *dev) {}
#endif /* CONFIG_NVM */
#endif /* LIGHTNVM.H */
