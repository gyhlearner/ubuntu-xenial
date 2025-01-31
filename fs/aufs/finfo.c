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
 * file private data
 */

#include "aufs.h"

<<<<<<< HEAD
void au_hfput(struct au_hfile *hf, struct file *file)
{
	/* todo: direct access f_flags */
	if (vfsub_file_flags(file) & __FMODE_EXEC)
		allow_write_access(hf->hf_file);
	fput(hf->hf_file);
	hf->hf_file = NULL;
	atomic_dec(&hf->hf_br->br_count);
=======
void au_hfput(struct au_hfile *hf, int execed)
{
	if (execed)
		allow_write_access(hf->hf_file);
	fput(hf->hf_file);
	hf->hf_file = NULL;
	au_br_put(hf->hf_br);
>>>>>>> temp
	hf->hf_br = NULL;
}

void au_set_h_fptr(struct file *file, aufs_bindex_t bindex, struct file *val)
{
	struct au_finfo *finfo = au_fi(file);
	struct au_hfile *hf;
	struct au_fidir *fidir;

	fidir = finfo->fi_hdir;
	if (!fidir) {
		AuDebugOn(finfo->fi_btop != bindex);
		hf = &finfo->fi_htop;
	} else
		hf = fidir->fd_hfile + bindex;

	if (hf && hf->hf_file)
<<<<<<< HEAD
		au_hfput(hf, file);
=======
		au_hfput(hf, vfsub_file_execed(file));
>>>>>>> temp
	if (val) {
		FiMustWriteLock(file);
		AuDebugOn(IS_ERR_OR_NULL(file->f_path.dentry));
		hf->hf_file = val;
		hf->hf_br = au_sbr(file->f_path.dentry->d_sb, bindex);
	}
}

void au_update_figen(struct file *file)
{
	atomic_set(&au_fi(file)->fi_generation, au_digen(file->f_path.dentry));
	/* smp_mb(); */ /* atomic_set */
}

/* ---------------------------------------------------------------------- */

struct au_fidir *au_fidir_alloc(struct super_block *sb)
{
	struct au_fidir *fidir;
	int nbr;

<<<<<<< HEAD
	nbr = au_sbend(sb) + 1;
=======
	nbr = au_sbbot(sb) + 1;
>>>>>>> temp
	if (nbr < 2)
		nbr = 2; /* initial allocate for 2 branches */
	fidir = kzalloc(au_fidir_sz(nbr), GFP_NOFS);
	if (fidir) {
		fidir->fd_bbot = -1;
		fidir->fd_nent = nbr;
	}

	return fidir;
}

<<<<<<< HEAD
int au_fidir_realloc(struct au_finfo *finfo, int nbr)
=======
int au_fidir_realloc(struct au_finfo *finfo, int nbr, int may_shrink)
>>>>>>> temp
{
	int err;
	struct au_fidir *fidir, *p;

	AuRwMustWriteLock(&finfo->fi_rwsem);
	fidir = finfo->fi_hdir;
	AuDebugOn(!fidir);

	err = -ENOMEM;
	p = au_kzrealloc(fidir, au_fidir_sz(fidir->fd_nent), au_fidir_sz(nbr),
<<<<<<< HEAD
			 GFP_NOFS);
=======
			 GFP_NOFS, may_shrink);
>>>>>>> temp
	if (p) {
		p->fd_nent = nbr;
		finfo->fi_hdir = p;
		err = 0;
	}

	return err;
}

/* ---------------------------------------------------------------------- */

void au_finfo_fin(struct file *file)
{
	struct au_finfo *finfo;

	au_nfiles_dec(file->f_path.dentry->d_sb);

	finfo = au_fi(file);
	AuDebugOn(finfo->fi_hdir);
	AuRwDestroy(&finfo->fi_rwsem);
	au_cache_free_finfo(finfo);
}

void au_fi_init_once(void *_finfo)
{
	struct au_finfo *finfo = _finfo;
<<<<<<< HEAD
	static struct lock_class_key aufs_fi;

	au_rw_init(&finfo->fi_rwsem);
	au_rw_class(&finfo->fi_rwsem, &aufs_fi);
=======

	au_rw_init(&finfo->fi_rwsem);
>>>>>>> temp
}

int au_finfo_init(struct file *file, struct au_fidir *fidir)
{
	int err;
	struct au_finfo *finfo;
	struct dentry *dentry;

	err = -ENOMEM;
	dentry = file->f_path.dentry;
	finfo = au_cache_alloc_finfo();
	if (unlikely(!finfo))
		goto out;

	err = 0;
	au_nfiles_inc(dentry->d_sb);
<<<<<<< HEAD
	/* verbose coding for lock class name */
	if (!fidir)
		au_rw_class(&finfo->fi_rwsem, au_lc_key + AuLcNonDir_FIINFO);
	else
		au_rw_class(&finfo->fi_rwsem, au_lc_key + AuLcDir_FIINFO);
=======
>>>>>>> temp
	au_rw_write_lock(&finfo->fi_rwsem);
	finfo->fi_btop = -1;
	finfo->fi_hdir = fidir;
	atomic_set(&finfo->fi_generation, au_digen(dentry));
	/* smp_mb(); */ /* atomic_set */

	file->private_data = finfo;

out:
	return err;
}
