/*
 * Internal Header for the Direct Rendering Manager
 *
 * Copyright 1999 Precision Insight, Inc., Cedar Park, Texas.
 * Copyright 2000 VA Linux Systems, Inc., Sunnyvale, California.
 * Copyright (c) 2009-2010, Code Aurora Forum.
 * All rights reserved.
 *
 * Author: Rickard E. (Rik) Faith <faith@valinux.com>
 * Author: Gareth Hughes <gareth@valinux.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _DRM_P_H_
#define _DRM_P_H_

#include <linux/agp_backend.h>
#include <linux/cdev.h>
#include <linux/dma-mapping.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/highmem.h>
#include <linux/idr.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/kref.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/poll.h>
#include <linux/ratelimit.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/workqueue.h>
#include <linux/dma-fence.h>
#include <linux/module.h>

#include <asm/mman.h>
#include <asm/pgalloc.h>
#include <linux/uaccess.h>

#include <uapi/drm/drm.h>
#include <uapi/drm/drm_mode.h>

#include <drm/drm_agpsupport.h>
#include <drm/drm_crtc.h>
#include <drm/drm_fourcc.h>
#include <drm/drm_global.h>
#include <drm/drm_hashtab.h>
#include <drm/drm_mm.h>
#include <drm/drm_os_linux.h>
#include <drm/drm_sarea.h>
#include <drm/drm_drv.h>
#include <drm/drm_prime.h>
#include <drm/drm_pci.h>
#include <drm/drm_file.h>
#include <drm/drm_debugfs.h>
#include <drm/drm_ioctl.h>
#include <drm/drm_sysfs.h>
#include <drm/drm_vblank.h>
#include <drm/drm_irq.h>
#include <drm/drm_device.h>

struct module;

struct device_node;
struct videomode;
struct reservation_object;
struct dma_buf_attachment;

struct pci_dev;
struct pci_controller;

/*
 * The following categories are defined:
 *
 * CORE: Used in the generic drm code: drm_ioctl.c, drm_mm.c, drm_memory.c, ...
 *	 This is the category used by the DRM_DEBUG() macro.
 *
 * DRIVER: Used in the vendor specific part of the driver: i915, radeon, ...
 *	   This is the category used by the DRM_DEBUG_DRIVER() macro.
 *
 * KMS: used in the modesetting code.
 *	This is the category used by the DRM_DEBUG_KMS() macro.
 *
 * PRIME: used in the prime code.
 *	  This is the category used by the DRM_DEBUG_PRIME() macro.
 *
 * ATOMIC: used in the atomic code.
 *	  This is the category used by the DRM_DEBUG_ATOMIC() macro.
 *
 * VBL: used for verbose debug message in the vblank code
 *	  This is the category used by the DRM_DEBUG_VBL() macro.
 *
 * Enabling verbose debug messages is done through the drm.debug parameter,
 * each category being enabled by a bit.
 *
 * drm.debug=0x1 will enable CORE messages
 * drm.debug=0x2 will enable DRIVER messages
 * drm.debug=0x3 will enable CORE and DRIVER messages
 * ...
 * drm.debug=0x3f will enable all messages
 *
 * An interesting feature is that it's possible to enable verbose logging at
 * run-time by echoing the debug value in its sysfs node:
 *   # echo 0xf > /sys/module/drm/parameters/debug
 */
#define DRM_UT_NONE		0x00
#define DRM_UT_CORE 		0x01
#define DRM_UT_DRIVER		0x02
#define DRM_UT_KMS		0x04
#define DRM_UT_PRIME		0x08
#define DRM_UT_ATOMIC		0x10
#define DRM_UT_VBL		0x20
#define DRM_UT_STATE		0x40
#define DRM_UT_LEASE		0x80

/***********************************************************************/
/** \name DRM template customization defaults */
/*@{*/

/***********************************************************************/
/** \name Macros to make printk easier */
/*@{*/

#define _DRM_PRINTK(once, level, fmt, ...)				\
	do {								\
		printk##once(KERN_##level "[" DRM_NAME "] " fmt,	\
			     ##__VA_ARGS__);				\
	} while (0)

#define DRM_INFO(fmt, ...)						\
	_DRM_PRINTK(, INFO, fmt, ##__VA_ARGS__)
#define DRM_NOTE(fmt, ...)						\
	_DRM_PRINTK(, NOTICE, fmt, ##__VA_ARGS__)
#define DRM_WARN(fmt, ...)						\
	_DRM_PRINTK(, WARNING, fmt, ##__VA_ARGS__)

#define DRM_INFO_ONCE(fmt, ...)						\
	_DRM_PRINTK(_once, INFO, fmt, ##__VA_ARGS__)
#define DRM_NOTE_ONCE(fmt, ...)						\
	_DRM_PRINTK(_once, NOTICE, fmt, ##__VA_ARGS__)
#define DRM_WARN_ONCE(fmt, ...)						\
	_DRM_PRINTK(_once, WARNING, fmt, ##__VA_ARGS__)

/**
 * Error output.
 *
 * \param fmt printf() like format string.
 * \param arg arguments
 */
#define DRM_DEV_ERROR(dev, fmt, ...)					\
	drm_dev_printk(dev, KERN_ERR, DRM_UT_NONE, __func__, " *ERROR*",\
		       fmt, ##__VA_ARGS__)
#define DRM_ERROR(fmt, ...)						\
	drm_printk(KERN_ERR, DRM_UT_NONE, fmt,	##__VA_ARGS__)

/**
 * Rate limited error output.  Like DRM_ERROR() but won't flood the log.
 *
 * \param fmt printf() like format string.
 * \param arg arguments
 */
#define DRM_DEV_ERROR_RATELIMITED(dev, fmt, ...)			\
({									\
	static DEFINE_RATELIMIT_STATE(_rs,				\
				      DEFAULT_RATELIMIT_INTERVAL,	\
				      DEFAULT_RATELIMIT_BURST);		\
									\
	if (__ratelimit(&_rs))						\
		DRM_DEV_ERROR(dev, fmt, ##__VA_ARGS__);			\
})
#define DRM_ERROR_RATELIMITED(fmt, ...)					\
	DRM_DEV_ERROR_RATELIMITED(NULL, fmt, ##__VA_ARGS__)

<<<<<<< HEAD
=======
#define DRM_DEV_INFO(dev, fmt, ...)					\
	drm_dev_printk(dev, KERN_INFO, DRM_UT_NONE, __func__, "", fmt,	\
		       ##__VA_ARGS__)

#define DRM_DEV_INFO_ONCE(dev, fmt, ...)				\
({									\
	static bool __print_once __read_mostly;				\
	if (!__print_once) {						\
		__print_once = true;					\
		DRM_DEV_INFO(dev, fmt, ##__VA_ARGS__);			\
	}								\
})

>>>>>>> temp
/**
 * Debug output.
 *
 * \param fmt printf() like format string.
 * \param arg arguments
 */
#define DRM_DEV_DEBUG(dev, fmt, args...)				\
	drm_dev_printk(dev, KERN_DEBUG, DRM_UT_CORE, __func__, "", fmt,	\
		       ##args)
#define DRM_DEBUG(fmt, ...)						\
	drm_printk(KERN_DEBUG, DRM_UT_CORE, fmt, ##__VA_ARGS__)

#define DRM_DEV_DEBUG_DRIVER(dev, fmt, args...)				\
	drm_dev_printk(dev, KERN_DEBUG, DRM_UT_DRIVER, __func__, "",	\
		       fmt, ##args)
#define DRM_DEBUG_DRIVER(fmt, ...)					\
	drm_printk(KERN_DEBUG, DRM_UT_DRIVER, fmt, ##__VA_ARGS__)

#define DRM_DEV_DEBUG_KMS(dev, fmt, args...)				\
	drm_dev_printk(dev, KERN_DEBUG, DRM_UT_KMS, __func__, "", fmt,	\
		       ##args)
#define DRM_DEBUG_KMS(fmt, ...)					\
	drm_printk(KERN_DEBUG, DRM_UT_KMS, fmt, ##__VA_ARGS__)

#define DRM_DEV_DEBUG_PRIME(dev, fmt, args...)				\
	drm_dev_printk(dev, KERN_DEBUG, DRM_UT_PRIME, __func__, "",	\
		       fmt, ##args)
#define DRM_DEBUG_PRIME(fmt, ...)					\
	drm_printk(KERN_DEBUG, DRM_UT_PRIME, fmt, ##__VA_ARGS__)

#define DRM_DEV_DEBUG_ATOMIC(dev, fmt, args...)				\
	drm_dev_printk(dev, KERN_DEBUG, DRM_UT_ATOMIC, __func__, "",	\
		       fmt, ##args)
#define DRM_DEBUG_ATOMIC(fmt, ...)					\
	drm_printk(KERN_DEBUG, DRM_UT_ATOMIC, fmt, ##__VA_ARGS__)

#define DRM_DEV_DEBUG_VBL(dev, fmt, args...)				\
	drm_dev_printk(dev, KERN_DEBUG, DRM_UT_VBL, __func__, "", fmt,	\
		       ##args)
#define DRM_DEBUG_VBL(fmt, ...)					\
	drm_printk(KERN_DEBUG, DRM_UT_VBL, fmt, ##__VA_ARGS__)

#define DRM_DEBUG_LEASE(fmt, ...)					\
	drm_printk(KERN_DEBUG, DRM_UT_LEASE, fmt, ##__VA_ARGS__)

#define _DRM_DEV_DEFINE_DEBUG_RATELIMITED(dev, level, fmt, args...)	\
({									\
	static DEFINE_RATELIMIT_STATE(_rs,				\
				      DEFAULT_RATELIMIT_INTERVAL,	\
				      DEFAULT_RATELIMIT_BURST);		\
	if (__ratelimit(&_rs))						\
		drm_dev_printk(dev, KERN_DEBUG, DRM_UT_ ## level,	\
			       __func__, "", fmt, ##args);		\
})

/**
 * Rate limited debug output. Like DRM_DEBUG() but won't flood the log.
 *
 * \param fmt printf() like format string.
 * \param arg arguments
 */
#define DRM_DEV_DEBUG_RATELIMITED(dev, fmt, args...)			\
	DEV__DRM_DEFINE_DEBUG_RATELIMITED(dev, CORE, fmt, ##args)
#define DRM_DEBUG_RATELIMITED(fmt, args...)				\
	DRM_DEV_DEBUG_RATELIMITED(NULL, fmt, ##args)
#define DRM_DEV_DEBUG_DRIVER_RATELIMITED(dev, fmt, args...)		\
	_DRM_DEV_DEFINE_DEBUG_RATELIMITED(dev, DRIVER, fmt, ##args)
#define DRM_DEBUG_DRIVER_RATELIMITED(fmt, args...)			\
	DRM_DEV_DEBUG_DRIVER_RATELIMITED(NULL, fmt, ##args)
#define DRM_DEV_DEBUG_KMS_RATELIMITED(dev, fmt, args...)		\
	_DRM_DEV_DEFINE_DEBUG_RATELIMITED(dev, KMS, fmt, ##args)
#define DRM_DEBUG_KMS_RATELIMITED(fmt, args...)				\
	DRM_DEV_DEBUG_KMS_RATELIMITED(NULL, fmt, ##args)
#define DRM_DEV_DEBUG_PRIME_RATELIMITED(dev, fmt, args...)		\
	_DRM_DEV_DEFINE_DEBUG_RATELIMITED(dev, PRIME, fmt, ##args)
#define DRM_DEBUG_PRIME_RATELIMITED(fmt, args...)			\
	DRM_DEV_DEBUG_PRIME_RATELIMITED(NULL, fmt, ##args)

/* Format strings and argument splitters to simplify printing
 * various "complex" objects
 */

/*@}*/

/***********************************************************************/
/** \name Internal types and structures */
/*@{*/

#define DRM_IF_VERSION(maj, min) (maj << 16 | min)


/**
 * drm_drv_uses_atomic_modeset - check if the driver implements
 * atomic_commit()
 * @dev: DRM device
 *
 * This check is useful if drivers do not have DRIVER_ATOMIC set but
 * have atomic modesetting internally implemented.
 */
static inline bool drm_drv_uses_atomic_modeset(struct drm_device *dev)
{
	return dev->mode_config.funcs->atomic_commit != NULL;
}

#define DRM_SWITCH_POWER_ON 0
#define DRM_SWITCH_POWER_OFF 1
#define DRM_SWITCH_POWER_CHANGING 2
#define DRM_SWITCH_POWER_DYNAMIC_OFF 3

static __inline__ int drm_core_check_feature(struct drm_device *dev,
					     int feature)
{
	return ((dev->driver->driver_features & feature) ? 1 : 0);
}

/******************************************************************/
/** \name Internal function definitions */
/*@{*/

				/* Driver support (drm_drv.h) */

/*
 * These are exported to drivers so that they can implement fencing using
 * DMA quiscent + idle. DMA quiescent usually requires the hardware lock.
 */

<<<<<<< HEAD
				/* IRQ support (drm_irq.h) */
extern int drm_irq_install(struct drm_device *dev, int irq);
extern int drm_irq_uninstall(struct drm_device *dev);

extern int drm_vblank_init(struct drm_device *dev, unsigned int num_crtcs);
extern int drm_wait_vblank(struct drm_device *dev, void *data,
			   struct drm_file *filp);
extern u32 drm_vblank_count(struct drm_device *dev, unsigned int pipe);
extern u32 drm_crtc_vblank_count(struct drm_crtc *crtc);
extern u32 drm_vblank_count_and_time(struct drm_device *dev, unsigned int pipe,
				     struct timeval *vblanktime);
extern u32 drm_crtc_vblank_count_and_time(struct drm_crtc *crtc,
					  struct timeval *vblanktime);
extern void drm_send_vblank_event(struct drm_device *dev, unsigned int pipe,
				  struct drm_pending_vblank_event *e);
extern void drm_crtc_send_vblank_event(struct drm_crtc *crtc,
				       struct drm_pending_vblank_event *e);
extern void drm_arm_vblank_event(struct drm_device *dev, unsigned int pipe,
				 struct drm_pending_vblank_event *e);
extern void drm_crtc_arm_vblank_event(struct drm_crtc *crtc,
				      struct drm_pending_vblank_event *e);
extern bool drm_handle_vblank(struct drm_device *dev, unsigned int pipe);
extern bool drm_crtc_handle_vblank(struct drm_crtc *crtc);
extern int drm_vblank_get(struct drm_device *dev, unsigned int pipe);
extern void drm_vblank_put(struct drm_device *dev, unsigned int pipe);
extern int drm_crtc_vblank_get(struct drm_crtc *crtc);
extern void drm_crtc_vblank_put(struct drm_crtc *crtc);
extern void drm_wait_one_vblank(struct drm_device *dev, unsigned int pipe);
extern void drm_crtc_wait_one_vblank(struct drm_crtc *crtc);
extern void drm_vblank_off(struct drm_device *dev, unsigned int pipe);
extern void drm_vblank_on(struct drm_device *dev, unsigned int pipe);
extern void drm_crtc_vblank_off(struct drm_crtc *crtc);
extern void drm_crtc_vblank_reset(struct drm_crtc *crtc);
extern void drm_crtc_vblank_on(struct drm_crtc *crtc);
extern void drm_vblank_cleanup(struct drm_device *dev);
extern u32 drm_vblank_no_hw_counter(struct drm_device *dev, unsigned int pipe);

extern int drm_calc_vbltimestamp_from_scanoutpos(struct drm_device *dev,
						 unsigned int pipe, int *max_error,
						 struct timeval *vblank_time,
						 unsigned flags,
						 const struct drm_display_mode *mode);
extern void drm_calc_timestamping_constants(struct drm_crtc *crtc,
					    const struct drm_display_mode *mode);

/**
 * drm_crtc_vblank_waitqueue - get vblank waitqueue for the CRTC
 * @crtc: which CRTC's vblank waitqueue to retrieve
 *
 * This function returns a pointer to the vblank waitqueue for the CRTC.
 * Drivers can use this to implement vblank waits using wait_event() & co.
 */
static inline wait_queue_head_t *drm_crtc_vblank_waitqueue(struct drm_crtc *crtc)
{
	return &crtc->dev->vblank[drm_crtc_index(crtc)].queue;
}

/* Modesetting support */
extern void drm_vblank_pre_modeset(struct drm_device *dev, unsigned int pipe);
extern void drm_vblank_post_modeset(struct drm_device *dev, unsigned int pipe);

				/* Stub support (drm_stub.h) */
extern struct drm_master *drm_master_get(struct drm_master *master);
extern void drm_master_put(struct drm_master **master);

extern void drm_put_dev(struct drm_device *dev);
extern void drm_unplug_dev(struct drm_device *dev);
extern unsigned int drm_debug;
extern bool drm_atomic;

				/* Debugfs support */
#if defined(CONFIG_DEBUG_FS)
extern int drm_debugfs_create_files(const struct drm_info_list *files,
				    int count, struct dentry *root,
				    struct drm_minor *minor);
extern int drm_debugfs_remove_files(const struct drm_info_list *files,
				    int count, struct drm_minor *minor);
#else
static inline int drm_debugfs_create_files(const struct drm_info_list *files,
					   int count, struct dentry *root,
					   struct drm_minor *minor)
{
	return 0;
}

static inline int drm_debugfs_remove_files(const struct drm_info_list *files,
					   int count, struct drm_minor *minor)
{
	return 0;
}
#endif

extern struct dma_buf *drm_gem_prime_export(struct drm_device *dev,
					    struct drm_gem_object *obj,
					    int flags);
extern int drm_gem_prime_handle_to_fd(struct drm_device *dev,
		struct drm_file *file_priv, uint32_t handle, uint32_t flags,
		int *prime_fd);
extern struct drm_gem_object *drm_gem_prime_import(struct drm_device *dev,
		struct dma_buf *dma_buf);
extern int drm_gem_prime_fd_to_handle(struct drm_device *dev,
		struct drm_file *file_priv, int prime_fd, uint32_t *handle);
extern void drm_gem_dmabuf_release(struct dma_buf *dma_buf);

extern int drm_prime_sg_to_page_addr_arrays(struct sg_table *sgt, struct page **pages,
					    dma_addr_t *addrs, int max_pages);
extern struct sg_table *drm_prime_pages_to_sg(struct page **pages, unsigned int nr_pages);
extern void drm_prime_gem_destroy(struct drm_gem_object *obj, struct sg_table *sg);


extern struct drm_dma_handle *drm_pci_alloc(struct drm_device *dev, size_t size,
					    size_t align);
extern void drm_pci_free(struct drm_device *dev, struct drm_dma_handle * dmah);

			       /* sysfs support (drm_sysfs.c) */
extern void drm_sysfs_hotplug_event(struct drm_device *dev);


struct drm_device *drm_dev_alloc(struct drm_driver *driver,
				 struct device *parent);
void drm_dev_ref(struct drm_device *dev);
void drm_dev_unref(struct drm_device *dev);
int drm_dev_register(struct drm_device *dev, unsigned long flags);
void drm_dev_unregister(struct drm_device *dev);
int drm_dev_set_unique(struct drm_device *dev, const char *fmt, ...);

struct drm_minor *drm_minor_acquire(unsigned int minor_id);
void drm_minor_release(struct drm_minor *minor);

/*@}*/

/* PCI section */
static __inline__ int drm_pci_device_is_agp(struct drm_device *dev)
{
	if (dev->driver->device_is_agp != NULL) {
		int err = (*dev->driver->device_is_agp) (dev);

		if (err != 2) {
			return err;
		}
	}

	return pci_find_capability(dev->pdev, PCI_CAP_ID_AGP);
}
void drm_pci_agp_destroy(struct drm_device *dev);

extern int drm_pci_init(struct drm_driver *driver, struct pci_driver *pdriver);
extern void drm_pci_exit(struct drm_driver *driver, struct pci_driver *pdriver);
#ifdef CONFIG_PCI
extern int drm_get_pci_dev(struct pci_dev *pdev,
			   const struct pci_device_id *ent,
			   struct drm_driver *driver);
extern int drm_pci_set_busid(struct drm_device *dev, struct drm_master *master);
#else
static inline int drm_get_pci_dev(struct pci_dev *pdev,
				  const struct pci_device_id *ent,
				  struct drm_driver *driver)
{
	return -ENOSYS;
}

static inline int drm_pci_set_busid(struct drm_device *dev,
				    struct drm_master *master)
{
	return -ENOSYS;
}
#endif

#define DRM_PCIE_SPEED_25 1
#define DRM_PCIE_SPEED_50 2
#define DRM_PCIE_SPEED_80 4

extern int drm_pcie_get_speed_cap_mask(struct drm_device *dev, u32 *speed_mask);
extern int drm_pcie_get_max_link_width(struct drm_device *dev, u32 *mlw);

/* platform section */
extern int drm_platform_init(struct drm_driver *driver, struct platform_device *platform_device);
extern int drm_platform_set_busid(struct drm_device *d, struct drm_master *m);

=======
/*@}*/

>>>>>>> temp
/* returns true if currently okay to sleep */
static __inline__ bool drm_can_sleep(void)
{
	if (in_atomic() || in_dbg_master() || irqs_disabled())
		return false;
	return true;
}

/* helper for handling conditionals in various for_each macros */
#define for_each_if(condition) if (!(condition)) {} else

#endif
