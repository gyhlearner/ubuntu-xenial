/*
 * Copyright 2014 IBM Corp.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/file.h>
#include <misc/cxl.h>
<<<<<<< HEAD
#include <linux/module.h>
#include <linux/mount.h>
=======
#include <linux/msi.h>
#include <linux/module.h>
#include <linux/mount.h>
#include <linux/sched/mm.h>
#include <linux/mmu_context.h>
>>>>>>> temp

#include "cxl.h"

/*
 * Since we want to track memory mappings to be able to force-unmap
 * when the AFU is no longer reachable, we need an inode. For devices
 * opened through the cxl user API, this is not a problem, but a
 * userland process can also get a cxl fd through the cxl_get_fd()
 * API, which is used by the cxlflash driver.
 *
 * Therefore we implement our own simple pseudo-filesystem and inode
 * allocator. We don't use the anonymous inode, as we need the
 * meta-data associated with it (address_space) and it is shared by
 * other drivers/processes, so it could lead to cxl unmapping VMAs
 * from random processes.
 */

#define CXL_PSEUDO_FS_MAGIC	0x1697697f

static int cxl_fs_cnt;
static struct vfsmount *cxl_vfs_mount;

static const struct dentry_operations cxl_fs_dops = {
	.d_dname	= simple_dname,
};

static struct dentry *cxl_fs_mount(struct file_system_type *fs_type, int flags,
				const char *dev_name, void *data)
{
	return mount_pseudo(fs_type, "cxl:", NULL, &cxl_fs_dops,
			CXL_PSEUDO_FS_MAGIC);
}

static struct file_system_type cxl_fs_type = {
	.name		= "cxl",
	.owner		= THIS_MODULE,
	.mount		= cxl_fs_mount,
	.kill_sb	= kill_anon_super,
};


void cxl_release_mapping(struct cxl_context *ctx)
{
	if (ctx->kernelapi && ctx->mapping)
		simple_release_fs(&cxl_vfs_mount, &cxl_fs_cnt);
}

static struct file *cxl_getfile(const char *name,
				const struct file_operations *fops,
				void *priv, int flags)
{
	struct qstr this;
	struct path path;
	struct file *file;
	struct inode *inode = NULL;
	int rc;

	/* strongly inspired by anon_inode_getfile() */

	if (fops->owner && !try_module_get(fops->owner))
		return ERR_PTR(-ENOENT);

	rc = simple_pin_fs(&cxl_fs_type, &cxl_vfs_mount, &cxl_fs_cnt);
	if (rc < 0) {
		pr_err("Cannot mount cxl pseudo filesystem: %d\n", rc);
		file = ERR_PTR(rc);
		goto err_module;
	}

	inode = alloc_anon_inode(cxl_vfs_mount->mnt_sb);
	if (IS_ERR(inode)) {
		file = ERR_CAST(inode);
		goto err_fs;
	}

	file = ERR_PTR(-ENOMEM);
	this.name = name;
	this.len = strlen(name);
	this.hash = 0;
	path.dentry = d_alloc_pseudo(cxl_vfs_mount->mnt_sb, &this);
	if (!path.dentry)
		goto err_inode;

	path.mnt = mntget(cxl_vfs_mount);
	d_instantiate(path.dentry, inode);

	file = alloc_file(&path, OPEN_FMODE(flags), fops);
<<<<<<< HEAD
	if (IS_ERR(file))
		goto err_dput;
=======
	if (IS_ERR(file)) {
		path_put(&path);
		goto err_fs;
	}
>>>>>>> temp
	file->f_flags = flags & (O_ACCMODE | O_NONBLOCK);
	file->private_data = priv;

	return file;

<<<<<<< HEAD
err_dput:
	path_put(&path);
=======
>>>>>>> temp
err_inode:
	iput(inode);
err_fs:
	simple_release_fs(&cxl_vfs_mount, &cxl_fs_cnt);
err_module:
	module_put(fops->owner);
	return file;
}

struct cxl_context *cxl_dev_context_init(struct pci_dev *dev)
{
	struct cxl_afu *afu;
	struct cxl_context  *ctx;
	int rc;

	afu = cxl_pci_to_afu(dev);
	if (IS_ERR(afu))
		return ERR_CAST(afu);

	ctx = cxl_context_alloc();
	if (!ctx)
		return ERR_PTR(-ENOMEM);

	ctx->kernelapi = true;

	/* Make it a slave context.  We can promote it later? */
	rc = cxl_context_init(ctx, afu, false);
	if (rc)
		goto err_ctx;

	return ctx;

err_ctx:
	kfree(ctx);
<<<<<<< HEAD
err_dev:
=======
>>>>>>> temp
	return ERR_PTR(rc);
}
EXPORT_SYMBOL_GPL(cxl_dev_context_init);

struct cxl_context *cxl_get_context(struct pci_dev *dev)
{
	return dev->dev.archdata.cxl_ctx;
}
EXPORT_SYMBOL_GPL(cxl_get_context);

<<<<<<< HEAD
struct device *cxl_get_phys_dev(struct pci_dev *dev)
{
	struct cxl_afu *afu;

	afu = cxl_pci_to_afu(dev);

	return afu->adapter->dev.parent;
}

=======
>>>>>>> temp
int cxl_release_context(struct cxl_context *ctx)
{
	if (ctx->status >= STARTED)
		return -EBUSY;

	cxl_context_free(ctx);

	return 0;
}
EXPORT_SYMBOL_GPL(cxl_release_context);

static irq_hw_number_t cxl_find_afu_irq(struct cxl_context *ctx, int num)
{
	__u16 range;
	int r;

	for (r = 0; r < CXL_IRQ_RANGES; r++) {
		range = ctx->irqs.range[r];
		if (num < range) {
			return ctx->irqs.offset[r] + num;
		}
		num -= range;
	}
	return 0;
}

<<<<<<< HEAD
=======
int _cxl_next_msi_hwirq(struct pci_dev *pdev, struct cxl_context **ctx, int *afu_irq)
{
	if (*ctx == NULL || *afu_irq == 0) {
		*afu_irq = 1;
		*ctx = cxl_get_context(pdev);
	} else {
		(*afu_irq)++;
		if (*afu_irq > cxl_get_max_irqs_per_process(pdev)) {
			*ctx = list_next_entry(*ctx, extra_irq_contexts);
			*afu_irq = 1;
		}
	}
	return cxl_find_afu_irq(*ctx, *afu_irq);
}
/* Exported via cxl_base */

int cxl_set_priv(struct cxl_context *ctx, void *priv)
{
	if (!ctx)
		return -EINVAL;

	ctx->priv = priv;

	return 0;
}
EXPORT_SYMBOL_GPL(cxl_set_priv);

void *cxl_get_priv(struct cxl_context *ctx)
{
	if (!ctx)
		return ERR_PTR(-EINVAL);

	return ctx->priv;
}
EXPORT_SYMBOL_GPL(cxl_get_priv);

>>>>>>> temp
int cxl_allocate_afu_irqs(struct cxl_context *ctx, int num)
{
	int res;
	irq_hw_number_t hwirq;

	if (num == 0)
		num = ctx->afu->pp_irqs;
	res = afu_allocate_irqs(ctx, num);
<<<<<<< HEAD
	if (!res && !cpu_has_feature(CPU_FTR_HVMODE)) {
=======
	if (res)
		return res;

	if (!cpu_has_feature(CPU_FTR_HVMODE)) {
>>>>>>> temp
		/* In a guest, the PSL interrupt is not multiplexed. It was
		 * allocated above, and we need to set its handler
		 */
		hwirq = cxl_find_afu_irq(ctx, 0);
		if (hwirq)
			cxl_map_irq(ctx->afu->adapter, hwirq, cxl_ops->psl_interrupt, ctx, "psl");
	}
<<<<<<< HEAD
=======

	if (ctx->status == STARTED) {
		if (cxl_ops->update_ivtes)
			cxl_ops->update_ivtes(ctx);
		else WARN(1, "BUG: cxl_allocate_afu_irqs must be called prior to starting the context on this platform\n");
	}

>>>>>>> temp
	return res;
}
EXPORT_SYMBOL_GPL(cxl_allocate_afu_irqs);

void cxl_free_afu_irqs(struct cxl_context *ctx)
{
	irq_hw_number_t hwirq;
	unsigned int virq;

	if (!cpu_has_feature(CPU_FTR_HVMODE)) {
		hwirq = cxl_find_afu_irq(ctx, 0);
		if (hwirq) {
			virq = irq_find_mapping(NULL, hwirq);
			if (virq)
				cxl_unmap_irq(virq, ctx);
		}
	}
	afu_irq_name_free(ctx);
	cxl_ops->release_irq_ranges(&ctx->irqs, ctx->afu->adapter);
}
EXPORT_SYMBOL_GPL(cxl_free_afu_irqs);

int cxl_map_afu_irq(struct cxl_context *ctx, int num,
		    irq_handler_t handler, void *cookie, char *name)
{
	irq_hw_number_t hwirq;

	/*
	 * Find interrupt we are to register.
	 */
	hwirq = cxl_find_afu_irq(ctx, num);
	if (!hwirq)
		return -ENOENT;

	return cxl_map_irq(ctx->afu->adapter, hwirq, handler, cookie, name);
}
EXPORT_SYMBOL_GPL(cxl_map_afu_irq);

void cxl_unmap_afu_irq(struct cxl_context *ctx, int num, void *cookie)
{
	irq_hw_number_t hwirq;
	unsigned int virq;

	hwirq = cxl_find_afu_irq(ctx, num);
	if (!hwirq)
		return;

	virq = irq_find_mapping(NULL, hwirq);
	if (virq)
		cxl_unmap_irq(virq, cookie);
}
EXPORT_SYMBOL_GPL(cxl_unmap_afu_irq);

/*
 * Start a context
 * Code here similar to afu_ioctl_start_work().
 */
int cxl_start_context(struct cxl_context *ctx, u64 wed,
		      struct task_struct *task)
{
	int rc = 0;
	bool kernel = true;

	pr_devel("%s: pe: %i\n", __func__, ctx->pe);

	mutex_lock(&ctx->status_mutex);
	if (ctx->status == STARTED)
		goto out; /* already started */

	/*
	 * Increment the mapped context count for adapter. This also checks
	 * if adapter_context_lock is taken.
	 */
	rc = cxl_adapter_context_get(ctx->afu->adapter);
	if (rc)
		goto out;

	if (task) {
		ctx->pid = get_task_pid(task, PIDTYPE_PID);
<<<<<<< HEAD
		ctx->glpid = get_task_pid(task->group_leader, PIDTYPE_PID);
=======
>>>>>>> temp
		kernel = false;
		ctx->real_mode = false;

		/* acquire a reference to the task's mm */
		ctx->mm = get_task_mm(current);

		/* ensure this mm_struct can't be freed */
		cxl_context_mm_count_get(ctx);

		if (ctx->mm) {
			/* decrement the use count from above */
			mmput(ctx->mm);
			/* make TLBIs for this context global */
			mm_context_add_copro(ctx->mm);
		}
	}

	/*
	 * Increment driver use count. Enables global TLBIs for hash
	 * and callbacks to handle the segment table
	 */
	cxl_ctx_get();

<<<<<<< HEAD
=======
	/* See the comment in afu_ioctl_start_work() */
	smp_mb();

>>>>>>> temp
	if ((rc = cxl_ops->attach_process(ctx, kernel, wed, 0))) {
		put_pid(ctx->pid);
		ctx->pid = NULL;
		cxl_adapter_context_put(ctx->afu->adapter);
		cxl_ctx_put();
		if (task) {
			cxl_context_mm_count_put(ctx);
			if (ctx->mm)
				mm_context_remove_copro(ctx->mm);
		}
		goto out;
	}

	ctx->status = STARTED;
out:
	mutex_unlock(&ctx->status_mutex);
	return rc;
}
EXPORT_SYMBOL_GPL(cxl_start_context);

int cxl_process_element(struct cxl_context *ctx)
{
	return ctx->external_pe;
}
EXPORT_SYMBOL_GPL(cxl_process_element);

/* Stop a context.  Returns 0 on success, otherwise -Errno */
int cxl_stop_context(struct cxl_context *ctx)
{
	return __detach_context(ctx);
}
EXPORT_SYMBOL_GPL(cxl_stop_context);

void cxl_set_master(struct cxl_context *ctx)
{
	ctx->master = true;
}
EXPORT_SYMBOL_GPL(cxl_set_master);

int cxl_set_translation_mode(struct cxl_context *ctx, bool real_mode)
{
	if (ctx->status == STARTED) {
		/*
		 * We could potentially update the PE and issue an update LLCMD
		 * to support this, but it doesn't seem to have a good use case
		 * since it's trivial to just create a second kernel context
		 * with different translation modes, so until someone convinces
		 * me otherwise:
		 */
		return -EBUSY;
	}

	ctx->real_mode = real_mode;
	return 0;
}
EXPORT_SYMBOL_GPL(cxl_set_translation_mode);

/* wrappers around afu_* file ops which are EXPORTED */
int cxl_fd_open(struct inode *inode, struct file *file)
{
	return afu_open(inode, file);
}
EXPORT_SYMBOL_GPL(cxl_fd_open);
int cxl_fd_release(struct inode *inode, struct file *file)
{
	return afu_release(inode, file);
}
EXPORT_SYMBOL_GPL(cxl_fd_release);
long cxl_fd_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return afu_ioctl(file, cmd, arg);
}
EXPORT_SYMBOL_GPL(cxl_fd_ioctl);
int cxl_fd_mmap(struct file *file, struct vm_area_struct *vm)
{
	return afu_mmap(file, vm);
}
EXPORT_SYMBOL_GPL(cxl_fd_mmap);
unsigned int cxl_fd_poll(struct file *file, struct poll_table_struct *poll)
{
	return afu_poll(file, poll);
}
EXPORT_SYMBOL_GPL(cxl_fd_poll);
ssize_t cxl_fd_read(struct file *file, char __user *buf, size_t count,
			loff_t *off)
{
	return afu_read(file, buf, count, off);
}
EXPORT_SYMBOL_GPL(cxl_fd_read);

#define PATCH_FOPS(NAME) if (!fops->NAME) fops->NAME = afu_fops.NAME

/* Get a struct file and fd for a context and attach the ops */
struct file *cxl_get_fd(struct cxl_context *ctx, struct file_operations *fops,
			int *fd)
{
	struct file *file;
	int rc, flags, fdtmp;
	char *name = NULL;

	/* only allow one per context */
	if (ctx->mapping)
		return ERR_PTR(-EEXIST);

	flags = O_RDWR | O_CLOEXEC;

	/* This code is similar to anon_inode_getfd() */
	rc = get_unused_fd_flags(flags);
	if (rc < 0)
		return ERR_PTR(rc);
	fdtmp = rc;

	/*
	 * Patch the file ops.  Needs to be careful that this is rentrant safe.
	 */
	if (fops) {
		PATCH_FOPS(open);
		PATCH_FOPS(poll);
		PATCH_FOPS(read);
		PATCH_FOPS(release);
		PATCH_FOPS(unlocked_ioctl);
		PATCH_FOPS(compat_ioctl);
		PATCH_FOPS(mmap);
	} else /* use default ops */
		fops = (struct file_operations *)&afu_fops;

	name = kasprintf(GFP_KERNEL, "cxl:%d", ctx->pe);
	file = cxl_getfile(name, fops, ctx, flags);
	kfree(name);
	if (IS_ERR(file))
		goto err_fd;

	cxl_context_set_mapping(ctx, file->f_mapping);
	*fd = fdtmp;
	return file;

err_fd:
	put_unused_fd(fdtmp);
	return NULL;
}
EXPORT_SYMBOL_GPL(cxl_get_fd);

struct cxl_context *cxl_fops_get_context(struct file *file)
{
	return file->private_data;
}
EXPORT_SYMBOL_GPL(cxl_fops_get_context);

void cxl_set_driver_ops(struct cxl_context *ctx,
			struct cxl_afu_driver_ops *ops)
{
	WARN_ON(!ops->fetch_event || !ops->event_delivered);
	atomic_set(&ctx->afu_driver_events, 0);
	ctx->afu_driver_ops = ops;
}
EXPORT_SYMBOL_GPL(cxl_set_driver_ops);

void cxl_context_events_pending(struct cxl_context *ctx,
				unsigned int new_events)
{
	atomic_add(new_events, &ctx->afu_driver_events);
	wake_up_all(&ctx->wq);
}
EXPORT_SYMBOL_GPL(cxl_context_events_pending);

int cxl_start_work(struct cxl_context *ctx,
		   struct cxl_ioctl_start_work *work)
{
	int rc;

	/* code taken from afu_ioctl_start_work */
	if (!(work->flags & CXL_START_WORK_NUM_IRQS))
		work->num_interrupts = ctx->afu->pp_irqs;
	else if ((work->num_interrupts < ctx->afu->pp_irqs) ||
		 (work->num_interrupts > ctx->afu->irqs_max)) {
		return -EINVAL;
	}

	rc = afu_register_irqs(ctx, work->num_interrupts);
	if (rc)
		return rc;

	rc = cxl_start_context(ctx, work->work_element_descriptor, current);
	if (rc < 0) {
		afu_release_irqs(ctx, ctx);
		return rc;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(cxl_start_work);

void __iomem *cxl_psa_map(struct cxl_context *ctx)
{
	if (ctx->status != STARTED)
		return NULL;

	pr_devel("%s: psn_phys%llx size:%llx\n",
		__func__, ctx->psn_phys, ctx->psn_size);
	return ioremap(ctx->psn_phys, ctx->psn_size);
}
EXPORT_SYMBOL_GPL(cxl_psa_map);

void cxl_psa_unmap(void __iomem *addr)
{
	iounmap(addr);
}
EXPORT_SYMBOL_GPL(cxl_psa_unmap);

int cxl_afu_reset(struct cxl_context *ctx)
{
	struct cxl_afu *afu = ctx->afu;
	int rc;

	rc = cxl_ops->afu_reset(afu);
	if (rc)
		return rc;

	return cxl_ops->afu_check_and_enable(afu);
}
EXPORT_SYMBOL_GPL(cxl_afu_reset);

void cxl_perst_reloads_same_image(struct cxl_afu *afu,
				  bool perst_reloads_same_image)
{
	afu->adapter->perst_same_image = perst_reloads_same_image;
}
EXPORT_SYMBOL_GPL(cxl_perst_reloads_same_image);

ssize_t cxl_read_adapter_vpd(struct pci_dev *dev, void *buf, size_t count)
{
	struct cxl_afu *afu = cxl_pci_to_afu(dev);
<<<<<<< HEAD
=======
	if (IS_ERR(afu))
		return -ENODEV;
>>>>>>> temp

	return cxl_ops->read_adapter_vpd(afu->adapter, buf, count);
}
EXPORT_SYMBOL_GPL(cxl_read_adapter_vpd);
<<<<<<< HEAD
=======

int cxl_set_max_irqs_per_process(struct pci_dev *dev, int irqs)
{
	struct cxl_afu *afu = cxl_pci_to_afu(dev);
	if (IS_ERR(afu))
		return -ENODEV;

	if (irqs > afu->adapter->user_irqs)
		return -EINVAL;

	/* Limit user_irqs to prevent the user increasing this via sysfs */
	afu->adapter->user_irqs = irqs;
	afu->irqs_max = irqs;

	return 0;
}
EXPORT_SYMBOL_GPL(cxl_set_max_irqs_per_process);

int cxl_get_max_irqs_per_process(struct pci_dev *dev)
{
	struct cxl_afu *afu = cxl_pci_to_afu(dev);
	if (IS_ERR(afu))
		return -ENODEV;

	return afu->irqs_max;
}
EXPORT_SYMBOL_GPL(cxl_get_max_irqs_per_process);

/*
 * This is a special interrupt allocation routine called from the PHB's MSI
 * setup function. When capi interrupts are allocated in this manner they must
 * still be associated with a running context, but since the MSI APIs have no
 * way to specify this we use the default context associated with the device.
 *
 * The Mellanox CX4 has a hardware limitation that restricts the maximum AFU
 * interrupt number, so in order to overcome this their driver informs us of
 * the restriction by setting the maximum interrupts per context, and we
 * allocate additional contexts as necessary so that we can keep the AFU
 * interrupt number within the supported range.
 */
int _cxl_cx4_setup_msi_irqs(struct pci_dev *pdev, int nvec, int type)
{
	struct cxl_context *ctx, *new_ctx, *default_ctx;
	int remaining;
	int rc;

	ctx = default_ctx = cxl_get_context(pdev);
	if (WARN_ON(!default_ctx))
		return -ENODEV;

	remaining = nvec;
	while (remaining > 0) {
		rc = cxl_allocate_afu_irqs(ctx, min(remaining, ctx->afu->irqs_max));
		if (rc) {
			pr_warn("%s: Failed to find enough free MSIs\n", pci_name(pdev));
			return rc;
		}
		remaining -= ctx->afu->irqs_max;

		if (ctx != default_ctx && default_ctx->status == STARTED) {
			WARN_ON(cxl_start_context(ctx,
				be64_to_cpu(default_ctx->elem->common.wed),
				NULL));
		}

		if (remaining > 0) {
			new_ctx = cxl_dev_context_init(pdev);
			if (IS_ERR(new_ctx)) {
				pr_warn("%s: Failed to allocate enough contexts for MSIs\n", pci_name(pdev));
				return -ENOSPC;
			}
			list_add(&new_ctx->extra_irq_contexts, &ctx->extra_irq_contexts);
			ctx = new_ctx;
		}
	}

	return 0;
}
/* Exported via cxl_base */

void _cxl_cx4_teardown_msi_irqs(struct pci_dev *pdev)
{
	struct cxl_context *ctx, *pos, *tmp;

	ctx = cxl_get_context(pdev);
	if (WARN_ON(!ctx))
		return;

	cxl_free_afu_irqs(ctx);
	list_for_each_entry_safe(pos, tmp, &ctx->extra_irq_contexts, extra_irq_contexts) {
		cxl_stop_context(pos);
		cxl_free_afu_irqs(pos);
		list_del(&pos->extra_irq_contexts);
		cxl_release_context(pos);
	}
}
/* Exported via cxl_base */
>>>>>>> temp
