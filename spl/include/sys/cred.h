/*****************************************************************************\
 *  Copyright (C) 2007-2010 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2007 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Brian Behlendorf <behlendorf1@llnl.gov>.
 *  UCRL-CODE-235197
 *
 *  This file is part of the SPL, Solaris Porting Layer.
 *  For details, see <http://zfsonlinux.org/>.
 *
 *  The SPL is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  The SPL is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with the SPL.  If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/

#ifndef _SPL_CRED_H
#define _SPL_CRED_H

#include <linux/module.h>
#include <sys/types.h>
#include <sys/vfs.h>

typedef struct cred cred_t;

#define	kcred		((cred_t *)(init_task.cred))
#define	CRED()		((cred_t *)current_cred())

<<<<<<< HEAD
#ifdef HAVE_KUIDGID_T

/*
 * Linux 3.8+ uses typedefs to redefine uid_t and gid_t. We have to rename the
 * typedefs to recover the original types. We then can use them provided that
 * we are careful about translating from k{g,u}id_t to the original versions
 * and vice versa.
 */
#define	uid_t		xuid_t
#define	gid_t		xgid_t
#include <linux/uidgid.h>
#undef uid_t
#undef gid_t
=======
/* Linux 4.9 API change, GROUP_AT was removed */
#ifndef GROUP_AT
#define	GROUP_AT(gi, i)	((gi)->gid[i])
#endif

#ifdef HAVE_KUIDGID_T
>>>>>>> temp

#define	KUID_TO_SUID(x)		(__kuid_val(x))
#define	KGID_TO_SGID(x)		(__kgid_val(x))
#define	SUID_TO_KUID(x)		(KUIDT_INIT(x))
#define	SGID_TO_KGID(x)		(KGIDT_INIT(x))
#define	KGIDP_TO_SGIDP(x)	(&(x)->val)

#else /* HAVE_KUIDGID_T */

#define	KUID_TO_SUID(x)		(x)
#define	KGID_TO_SGID(x)		(x)
#define	SUID_TO_KUID(x)		(x)
#define	SGID_TO_KGID(x)		(x)
#define	KGIDP_TO_SGIDP(x)	(x)

#endif /* HAVE_KUIDGID_T */

extern void crhold(cred_t *cr);
extern void crfree(cred_t *cr);
extern uid_t crgetuid(const cred_t *cr);
extern uid_t crgetruid(const cred_t *cr);
extern uid_t crgetsuid(const cred_t *cr);
extern uid_t crgetfsuid(const cred_t *cr);
extern gid_t crgetgid(const cred_t *cr);
extern gid_t crgetrgid(const cred_t *cr);
extern gid_t crgetsgid(const cred_t *cr);
extern gid_t crgetfsgid(const cred_t *cr);
extern int crgetngroups(const cred_t *cr);
extern gid_t * crgetgroups(const cred_t *cr);
extern int groupmember(gid_t gid, const cred_t *cr);

#endif  /* _SPL_CRED_H */
