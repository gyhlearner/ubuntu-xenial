<<<<<<< HEAD
/*****************************************************************************\
=======
/*
>>>>>>> temp
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
<<<<<<< HEAD
\*****************************************************************************/

#ifndef _SPL_TASKQ_H
#define _SPL_TASKQ_H
=======
 */

#ifndef _SPL_TASKQ_H
#define	_SPL_TASKQ_H
>>>>>>> temp

#include <linux/module.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
<<<<<<< HEAD
#include <sys/types.h>
#include <sys/thread.h>

#define TASKQ_NAMELEN		31

#define TASKQ_PREPOPULATE	0x00000001
#define TASKQ_CPR_SAFE		0x00000002
#define TASKQ_DYNAMIC		0x00000004
#define TASKQ_THREADS_CPU_PCT	0x00000008
#define TASKQ_DC_BATCH		0x00000010
#define TASKQ_ACTIVE		0x80000000
=======
#include <linux/wait_compat.h>
#include <sys/types.h>
#include <sys/thread.h>
#include <sys/rwlock.h>

#define	TASKQ_NAMELEN		31

#define	TASKQ_PREPOPULATE	0x00000001
#define	TASKQ_CPR_SAFE		0x00000002
#define	TASKQ_DYNAMIC		0x00000004
#define	TASKQ_THREADS_CPU_PCT	0x00000008
#define	TASKQ_DC_BATCH		0x00000010
#define	TASKQ_ACTIVE		0x80000000
>>>>>>> temp

/*
 * Flags for taskq_dispatch. TQ_SLEEP/TQ_NOSLEEP should be same as
 * KM_SLEEP/KM_NOSLEEP.  TQ_NOQUEUE/TQ_NOALLOC are set particularly
 * large so as not to conflict with already used GFP_* defines.
 */
<<<<<<< HEAD
#define TQ_SLEEP		0x00000000
#define TQ_NOSLEEP		0x00000001
#define TQ_PUSHPAGE		0x00000002
#define TQ_NOQUEUE		0x01000000
#define TQ_NOALLOC		0x02000000
#define TQ_NEW			0x04000000
#define TQ_FRONT		0x08000000
=======
#define	TQ_SLEEP		0x00000000
#define	TQ_NOSLEEP		0x00000001
#define	TQ_PUSHPAGE		0x00000002
#define	TQ_NOQUEUE		0x01000000
#define	TQ_NOALLOC		0x02000000
#define	TQ_NEW			0x04000000
#define	TQ_FRONT		0x08000000

/*
 * Reserved taskqid values.
 */
#define	TASKQID_INVALID		((taskqid_t)0)
#define	TASKQID_INITIAL		((taskqid_t)1)

/*
 * spin_lock(lock) and spin_lock_nested(lock,0) are equivalent,
 * so TQ_LOCK_DYNAMIC must not evaluate to 0
 */
typedef enum tq_lock_role {
	TQ_LOCK_GENERAL =	0,
	TQ_LOCK_DYNAMIC =	1,
} tq_lock_role_t;
>>>>>>> temp

typedef unsigned long taskqid_t;
typedef void (task_func_t)(void *);

typedef struct taskq {
<<<<<<< HEAD
	spinlock_t		tq_lock;       /* protects taskq_t */
	unsigned long		tq_lock_flags; /* interrupt state */
	char			*tq_name;      /* taskq name */
	struct list_head	tq_thread_list;/* list of all threads */
	struct list_head	tq_active_list;/* list of active threads */
	int			tq_nactive;    /* # of active threads */
	int			tq_nthreads;   /* # of existing threads */
	int			tq_nspawn;     /* # of threads being spawned */
	int			tq_maxthreads; /* # of threads maximum */
	int			tq_pri;        /* priority */
	int			tq_minalloc;   /* min task_t pool size */
	int			tq_maxalloc;   /* max task_t pool size */
	int			tq_nalloc;     /* cur task_t pool size */
	uint_t			tq_flags;      /* flags */
	taskqid_t		tq_next_id;    /* next pend/work id */
	taskqid_t		tq_lowest_id;  /* lowest pend/work id */
	struct list_head	tq_free_list;  /* free task_t's */
	struct list_head	tq_pend_list;  /* pending task_t's */
	struct list_head	tq_prio_list;  /* priority pending task_t's */
	struct list_head	tq_delay_list; /* delayed task_t's */
	wait_queue_head_t	tq_work_waitq; /* new work waitq */
	wait_queue_head_t	tq_wait_waitq; /* wait waitq */
=======
	spinlock_t		tq_lock;	/* protects taskq_t */
	char			*tq_name;	/* taskq name */
	int			tq_instance;	/* instance of tq_name */
	struct list_head	tq_thread_list;	/* list of all threads */
	struct list_head	tq_active_list;	/* list of active threads */
	int			tq_nactive;	/* # of active threads */
	int			tq_nthreads;	/* # of existing threads */
	int			tq_nspawn;	/* # of threads being spawned */
	int			tq_maxthreads;	/* # of threads maximum */
	int			tq_pri;		/* priority */
	int			tq_minalloc;	/* min taskq_ent_t pool size */
	int			tq_maxalloc;	/* max taskq_ent_t pool size */
	int			tq_nalloc;	/* cur taskq_ent_t pool size */
	uint_t			tq_flags;	/* flags */
	taskqid_t		tq_next_id;	/* next pend/work id */
	taskqid_t		tq_lowest_id;	/* lowest pend/work id */
	struct list_head	tq_free_list;	/* free taskq_ent_t's */
	struct list_head	tq_pend_list;	/* pending taskq_ent_t's */
	struct list_head	tq_prio_list;	/* priority pending taskq_ent_t's */
	struct list_head	tq_delay_list;	/* delayed taskq_ent_t's */
	struct list_head	tq_taskqs;	/* all taskq_t's */
	spl_wait_queue_head_t	tq_work_waitq;	/* new work waitq */
	spl_wait_queue_head_t	tq_wait_waitq;	/* wait waitq */
	tq_lock_role_t		tq_lock_class;	/* class when taking tq_lock */
>>>>>>> temp
} taskq_t;

typedef struct taskq_ent {
	spinlock_t		tqent_lock;
<<<<<<< HEAD
	wait_queue_head_t	tqent_waitq;
=======
	spl_wait_queue_head_t	tqent_waitq;
>>>>>>> temp
	struct timer_list	tqent_timer;
	struct list_head	tqent_list;
	taskqid_t		tqent_id;
	task_func_t		*tqent_func;
	void			*tqent_arg;
	taskq_t			*tqent_taskq;
	uintptr_t		tqent_flags;
<<<<<<< HEAD
} taskq_ent_t;

#define TQENT_FLAG_PREALLOC     0x1
#define TQENT_FLAG_CANCEL       0x2
=======
	unsigned long		tqent_birth;
} taskq_ent_t;

#define	TQENT_FLAG_PREALLOC	0x1
#define	TQENT_FLAG_CANCEL	0x2
>>>>>>> temp

typedef struct taskq_thread {
	struct list_head	tqt_thread_list;
	struct list_head	tqt_active_list;
	struct task_struct	*tqt_thread;
	taskq_t			*tqt_tq;
	taskqid_t		tqt_id;
	taskq_ent_t		*tqt_task;
	uintptr_t		tqt_flags;
} taskq_thread_t;

/* Global system-wide dynamic task queue available for all consumers */
extern taskq_t *system_taskq;
<<<<<<< HEAD
=======
/* Global dynamic task queue for long delay */
extern taskq_t *system_delay_taskq;

/* List of all taskqs */
extern struct list_head tq_list;
extern struct rw_semaphore tq_list_sem;
>>>>>>> temp

extern taskqid_t taskq_dispatch(taskq_t *, task_func_t, void *, uint_t);
extern taskqid_t taskq_dispatch_delay(taskq_t *, task_func_t, void *,
    uint_t, clock_t);
extern void taskq_dispatch_ent(taskq_t *, task_func_t, void *, uint_t,
    taskq_ent_t *);
extern int taskq_empty_ent(taskq_ent_t *);
extern void taskq_init_ent(taskq_ent_t *);
extern taskq_t *taskq_create(const char *, int, pri_t, int, int, uint_t);
extern void taskq_destroy(taskq_t *);
extern void taskq_wait_id(taskq_t *, taskqid_t);
extern void taskq_wait_outstanding(taskq_t *, taskqid_t);
extern void taskq_wait(taskq_t *);
extern int taskq_cancel_id(taskq_t *, taskqid_t);
<<<<<<< HEAD
extern int taskq_member(taskq_t *, void *);

#define taskq_create_proc(name, nthreads, pri, min, max, proc, flags) \
    taskq_create(name, nthreads, pri, min, max, flags)
#define taskq_create_sysdc(name, nthreads, min, max, proc, dc, flags) \
=======
extern int taskq_member(taskq_t *, kthread_t *);

#define	taskq_create_proc(name, nthreads, pri, min, max, proc, flags) \
    taskq_create(name, nthreads, pri, min, max, flags)
#define	taskq_create_sysdc(name, nthreads, min, max, proc, dc, flags) \
>>>>>>> temp
    taskq_create(name, nthreads, maxclsyspri, min, max, flags)

int spl_taskq_init(void);
void spl_taskq_fini(void);

#endif  /* _SPL_TASKQ_H */
