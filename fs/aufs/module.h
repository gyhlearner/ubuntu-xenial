/*
<<<<<<< HEAD
 * Copyright (C) 2005-2015 Junjiro R. Okajima
=======
 * Copyright (C) 2005-2017 Junjiro R. Okajima
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
 * module initialization and module-global
 */

#ifndef __AUFS_MODULE_H__
#define __AUFS_MODULE_H__

#ifdef __KERNEL__

#include <linux/slab.h>

struct path;
struct seq_file;

/* module parameters */
extern int sysaufs_brs;
<<<<<<< HEAD
=======
extern bool au_userns;
>>>>>>> temp

/* ---------------------------------------------------------------------- */

extern int au_dir_roflags;

<<<<<<< HEAD
enum {
	AuLcNonDir_FIINFO,
	AuLcNonDir_DIINFO,
	AuLcNonDir_IIINFO,

	AuLcDir_FIINFO,
	AuLcDir_DIINFO,
	AuLcDir_IIINFO,

	AuLcSymlink_DIINFO,
	AuLcSymlink_IIINFO,

	AuLcKey_Last
};
extern struct lock_class_key au_lc_key[AuLcKey_Last];

void *au_kzrealloc(void *p, unsigned int nused, unsigned int new_sz, gfp_t gfp);
=======
void *au_krealloc(void *p, unsigned int new_sz, gfp_t gfp, int may_shrink);
void *au_kzrealloc(void *p, unsigned int nused, unsigned int new_sz, gfp_t gfp,
		   int may_shrink);

static inline int au_kmidx_sub(size_t sz, size_t new_sz)
{
#ifndef CONFIG_SLOB
	return kmalloc_index(sz) - kmalloc_index(new_sz);
#else
	return -1; /* SLOB is untested */
#endif
}

>>>>>>> temp
int au_seq_path(struct seq_file *seq, struct path *path);

#ifdef CONFIG_PROC_FS
/* procfs.c */
int __init au_procfs_init(void);
void au_procfs_fin(void);
#else
AuStubInt0(au_procfs_init, void);
AuStubVoid(au_procfs_fin, void);
#endif

/* ---------------------------------------------------------------------- */

/* kmem cache */
enum {
	AuCache_DINFO,
	AuCache_ICNTNR,
	AuCache_FINFO,
	AuCache_VDIR,
	AuCache_DEHSTR,
	AuCache_HNOTIFY, /* must be last */
	AuCache_Last
};

<<<<<<< HEAD
=======
extern struct kmem_cache *au_cache[AuCache_Last];

>>>>>>> temp
#define AuCacheFlags		(SLAB_RECLAIM_ACCOUNT | SLAB_MEM_SPREAD)
#define AuCache(type)		KMEM_CACHE(type, AuCacheFlags)
#define AuCacheCtor(type, ctor)	\
	kmem_cache_create(#type, sizeof(struct type), \
			  __alignof__(struct type), AuCacheFlags, ctor)

<<<<<<< HEAD
extern struct kmem_cache *au_cachep[];

#define AuCacheFuncs(name, index) \
static inline struct au_##name *au_cache_alloc_##name(void) \
{ return kmem_cache_alloc(au_cachep[AuCache_##index], GFP_NOFS); } \
static inline void au_cache_free_##name(struct au_##name *p) \
{ kmem_cache_free(au_cachep[AuCache_##index], p); }
=======
#define AuCacheFuncs(name, index) \
static inline struct au_##name *au_cache_alloc_##name(void) \
{ return kmem_cache_alloc(au_cache[AuCache_##index], GFP_NOFS); } \
static inline void au_cache_free_##name(struct au_##name *p) \
{ kmem_cache_free(au_cache[AuCache_##index], p); }
>>>>>>> temp

AuCacheFuncs(dinfo, DINFO);
AuCacheFuncs(icntnr, ICNTNR);
AuCacheFuncs(finfo, FINFO);
AuCacheFuncs(vdir, VDIR);
AuCacheFuncs(vdir_dehstr, DEHSTR);
#ifdef CONFIG_AUFS_HNOTIFY
AuCacheFuncs(hnotify, HNOTIFY);
#endif

#endif /* __KERNEL__ */
#endif /* __AUFS_MODULE_H__ */
