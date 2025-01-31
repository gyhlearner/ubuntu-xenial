/*  $Id: vbox_drv.c $ */
<<<<<<< HEAD
/** @file
 * VirtualBox Additions Linux kernel video driver
 */

/*
 * Copyright (C) 2013 Oracle Corporation
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 * --------------------------------------------------------------------
 *
 * This code is based on
 * ast_drv.c
 * with the following copyright and permission notice:
 *
=======
/*
 * Copyright (C) 2013-2017 Oracle Corporation
 * This file is based on ast_drv.c
>>>>>>> temp
 * Copyright 2012 Red Hat Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
<<<<<<< HEAD
 */
/*
 * Authors: Dave Airlie <airlied@redhat.com>
 */
#include "vbox_drv.h"

#include <VBox/VBoxGuest.h>
=======
 * Authors: Dave Airlie <airlied@redhat.com>
 *          Michael Thayer <michael.thayer@oracle.com,
 *          Hans de Goede <hdegoede@redhat.com>
 */
#include "vbox_drv.h"

#include "version-generated.h"
#include "revision-generated.h"
>>>>>>> temp

#include <linux/module.h>
#include <linux/console.h>
#include <linux/vt_kern.h>

#include <drm/drmP.h>
#include <drm/drm_crtc_helper.h>
<<<<<<< HEAD
#include "version-generated.h"
=======
>>>>>>> temp

int vbox_modeset = -1;

MODULE_PARM_DESC(modeset, "Disable/Enable modesetting");
module_param_named(modeset, vbox_modeset, int, 0400);

static struct drm_driver driver;

<<<<<<< HEAD
static DEFINE_PCI_DEVICE_TABLE(pciidlist) =
{
    {0x80ee, 0xbeef, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
    {0, 0, 0},
};

=======
static const struct pci_device_id pciidlist[] = { { 0x80ee, 0xbeef, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ 0, 0, 0},
};
>>>>>>> temp
MODULE_DEVICE_TABLE(pci, pciidlist);

static int vbox_pci_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
<<<<<<< HEAD
    return drm_get_pci_dev(pdev, ent, &driver);
}


static void vbox_pci_remove(struct pci_dev *pdev)
{
    struct drm_device *dev = pci_get_drvdata(pdev);

    drm_put_dev(dev);
}



static int vbox_drm_freeze(struct drm_device *dev)
{
    drm_kms_helper_poll_disable(dev);

    pci_save_state(dev->pdev);

    console_lock();
    vbox_fbdev_set_suspend(dev, 1);
    console_unlock();
    return 0;
=======
	return drm_get_pci_dev(pdev, ent, &driver);
}

static void vbox_pci_remove(struct pci_dev *pdev)
{
	struct drm_device *dev = pci_get_drvdata(pdev);

	drm_put_dev(dev);
}

static int vbox_drm_freeze(struct drm_device *dev)
{
	drm_kms_helper_poll_disable(dev);

	pci_save_state(dev->pdev);

	console_lock();
	vbox_fbdev_set_suspend(dev, 1);
	console_unlock();

	return 0;
>>>>>>> temp
}

static int vbox_drm_thaw(struct drm_device *dev)
{
<<<<<<< HEAD
    int error = 0;

    drm_mode_config_reset(dev);
    drm_helper_resume_force_mode(dev);

    console_lock();
    vbox_fbdev_set_suspend(dev, 0);
    console_unlock();
    return error;
=======
	drm_mode_config_reset(dev);
	drm_helper_resume_force_mode(dev);

	console_lock();
	vbox_fbdev_set_suspend(dev, 0);
	console_unlock();

	return 0;
>>>>>>> temp
}

static int vbox_drm_resume(struct drm_device *dev)
{
<<<<<<< HEAD
    int ret;

    if (pci_enable_device(dev->pdev))
        return -EIO;

       ret = vbox_drm_thaw(dev);
    if (ret)
       return ret;

    drm_kms_helper_poll_enable(dev);
    return 0;
=======
	int ret;

	if (pci_enable_device(dev->pdev))
		return -EIO;

	ret = vbox_drm_thaw(dev);
	if (ret)
		return ret;

	drm_kms_helper_poll_enable(dev);

	return 0;
>>>>>>> temp
}

static int vbox_pm_suspend(struct device *dev)
{
<<<<<<< HEAD
    struct pci_dev *pdev = to_pci_dev(dev);
    struct drm_device *ddev = pci_get_drvdata(pdev);
    int error;

    error = vbox_drm_freeze(ddev);
    if (error)
        return error;

    pci_disable_device(pdev);
    pci_set_power_state(pdev, PCI_D3hot);
    return 0;
=======
	struct pci_dev *pdev = to_pci_dev(dev);
	struct drm_device *ddev = pci_get_drvdata(pdev);
	int error;

	error = vbox_drm_freeze(ddev);
	if (error)
		return error;

	pci_disable_device(pdev);
	pci_set_power_state(pdev, PCI_D3hot);

	return 0;
>>>>>>> temp
}

static int vbox_pm_resume(struct device *dev)
{
<<<<<<< HEAD
    struct pci_dev *pdev = to_pci_dev(dev);
    struct drm_device *ddev = pci_get_drvdata(pdev);
    return vbox_drm_resume(ddev);
=======
	struct drm_device *ddev = pci_get_drvdata(to_pci_dev(dev));

	return vbox_drm_resume(ddev);
>>>>>>> temp
}

static int vbox_pm_freeze(struct device *dev)
{
<<<<<<< HEAD
    struct pci_dev *pdev = to_pci_dev(dev);
    struct drm_device *ddev = pci_get_drvdata(pdev);

    if (!ddev || !ddev->dev_private)
        return -ENODEV;
    return vbox_drm_freeze(ddev);

=======
	struct pci_dev *pdev = to_pci_dev(dev);
	struct drm_device *ddev = pci_get_drvdata(pdev);

	if (!ddev || !ddev->dev_private)
		return -ENODEV;

	return vbox_drm_freeze(ddev);
>>>>>>> temp
}

static int vbox_pm_thaw(struct device *dev)
{
<<<<<<< HEAD
    struct pci_dev *pdev = to_pci_dev(dev);
    struct drm_device *ddev = pci_get_drvdata(pdev);
    return vbox_drm_thaw(ddev);
=======
	struct drm_device *ddev = pci_get_drvdata(to_pci_dev(dev));

	return vbox_drm_thaw(ddev);
>>>>>>> temp
}

static int vbox_pm_poweroff(struct device *dev)
{
<<<<<<< HEAD
    struct pci_dev *pdev = to_pci_dev(dev);
    struct drm_device *ddev = pci_get_drvdata(pdev);

    return vbox_drm_freeze(ddev);
}

static const struct dev_pm_ops vbox_pm_ops = {
    .suspend = vbox_pm_suspend,
    .resume = vbox_pm_resume,
    .freeze = vbox_pm_freeze,
    .thaw = vbox_pm_thaw,
    .poweroff = vbox_pm_poweroff,
    .restore = vbox_pm_resume,
};

static struct pci_driver vbox_pci_driver =
{
    .name = DRIVER_NAME,
    .id_table = pciidlist,
    .probe = vbox_pci_probe,
    .remove = vbox_pci_remove,
    .driver.pm = &vbox_pm_ops,
};

static const struct file_operations vbox_fops =
{
    .owner = THIS_MODULE,
    .open = drm_open,
    .release = drm_release,
    .unlocked_ioctl = drm_ioctl,
    .mmap = vbox_mmap,
    .poll = drm_poll,
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 12, 0)
    .fasync = drm_fasync,
#endif
#ifdef CONFIG_COMPAT
    .compat_ioctl = drm_compat_ioctl,
#endif
    .read = drm_read,
};

static int vbox_master_set(struct drm_device *dev,
                           struct drm_file *file_priv,
                           bool from_open)
{
    struct vbox_private *vbox = dev->dev_private;
    vbox->initial_mode_queried = false;
    vbox_disable_accel(vbox);
    return 0;
}

static void vbox_master_drop(struct drm_device *dev,
                             struct drm_file *file_priv,
                             bool from_release)
{
    struct vbox_private *vbox = dev->dev_private;
    vbox->initial_mode_queried = false;
    vbox_disable_accel(vbox);
}

static struct drm_driver driver =
{
    .driver_features = DRIVER_MODESET | DRIVER_GEM | DRIVER_HAVE_IRQ | DRIVER_IRQ_SHARED,
    .dev_priv_size = 0,

    .load = vbox_driver_load,
    .unload = vbox_driver_unload,
    .lastclose = vbox_driver_lastclose,
    .master_set = vbox_master_set,
    .master_drop = vbox_master_drop,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0)
    .set_busid = drm_pci_set_busid,
#endif

    .fops = &vbox_fops,
    .irq_handler = vbox_irq_handler,
    .name = DRIVER_NAME,
    .desc = DRIVER_DESC,
    .date = DRIVER_DATE,
    .major = DRIVER_MAJOR,
    .minor = DRIVER_MINOR,
    .patchlevel = DRIVER_PATCHLEVEL,

    .gem_free_object = vbox_gem_free_object,
    .dumb_create = vbox_dumb_create,
    .dumb_map_offset = vbox_dumb_mmap_offset,
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 12, 0)
    .dumb_destroy = vbox_dumb_destroy,
#else
    .dumb_destroy = drm_gem_dumb_destroy,
#endif

=======
	struct drm_device *ddev = pci_get_drvdata(to_pci_dev(dev));

	return vbox_drm_freeze(ddev);
}

static const struct dev_pm_ops vbox_pm_ops = {
	.suspend = vbox_pm_suspend,
	.resume = vbox_pm_resume,
	.freeze = vbox_pm_freeze,
	.thaw = vbox_pm_thaw,
	.poweroff = vbox_pm_poweroff,
	.restore = vbox_pm_resume,
};

static struct pci_driver vbox_pci_driver = {
	.name = DRIVER_NAME,
	.id_table = pciidlist,
	.probe = vbox_pci_probe,
	.remove = vbox_pci_remove,
	.driver.pm = &vbox_pm_ops,
};

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 7, 0) && !defined(RHEL_74)
/* This works around a bug in X servers prior to 1.18.4, which sometimes
 * submit more dirty rectangles than the kernel is willing to handle and
 * then disable dirty rectangle handling altogether when they see the
 * EINVAL error.  I do not want the code to hang around forever, which is
 * why I am limiting it to certain kernel versions.  We can increase the
 * limit if some distributions uses old X servers with new kernels. */
long vbox_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long rc = drm_ioctl(filp, cmd, arg);

	if (cmd == DRM_IOCTL_MODE_DIRTYFB && rc == -EINVAL)
		return -EOVERFLOW;

	return rc;
}
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 7, 0) && !RHEL_74 */

static const struct file_operations vbox_fops = {
	.owner = THIS_MODULE,
	.open = drm_open,
	.release = drm_release,
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 7, 0) && !defined(RHEL_74)
	.unlocked_ioctl = vbox_ioctl,
#else
	.unlocked_ioctl = drm_ioctl,
#endif
	.mmap = vbox_mmap,
	.poll = drm_poll,
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 12, 0) && !defined(RHEL_73)
	.fasync = drm_fasync,
#endif
#ifdef CONFIG_COMPAT
	.compat_ioctl = drm_compat_ioctl,
#endif
	.read = drm_read,
};

static int vbox_master_set(struct drm_device *dev,
			   struct drm_file *file_priv, bool from_open)
{
	struct vbox_private *vbox = dev->dev_private;

	/*
	 * We do not yet know whether the new owner can handle hotplug, so we
	 * do not advertise dynamic modes on the first query and send a
	 * tentative hotplug notification after that to see if they query again.
	 */
	vbox->initial_mode_queried = false;

	mutex_lock(&vbox->hw_mutex);
	/*
	 * Disable VBVA when someone releases master in case the next person
	 * tries tries to do VESA.
	 */
	/** @todo work out if anyone is likely to and whether it will work. */
	/*
	 * Update: we also disable it because if the new master does not do
	 * dirty rectangle reporting (e.g. old versions of Plymouth) then at
	 * least the first screen will still be updated. We enable it as soon
	 * as we receive a dirty rectangle report.
	 */
	vbox_disable_accel(vbox);
	mutex_unlock(&vbox->hw_mutex);

	return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0) && !defined(RHEL_74)
static void vbox_master_drop(struct drm_device *dev,
			     struct drm_file *file_priv, bool from_release)
#else
static void vbox_master_drop(struct drm_device *dev, struct drm_file *file_priv)
#endif
{
	struct vbox_private *vbox = dev->dev_private;

	/* See vbox_master_set() */
	vbox->initial_mode_queried = false;

	mutex_lock(&vbox->hw_mutex);
	vbox_disable_accel(vbox);
	mutex_unlock(&vbox->hw_mutex);
}

static struct drm_driver driver = {
	.driver_features =
	    DRIVER_MODESET | DRIVER_GEM | DRIVER_HAVE_IRQ | DRIVER_IRQ_SHARED |
	    DRIVER_PRIME,
	.dev_priv_size = 0,

	.load = vbox_driver_load,
	.unload = vbox_driver_unload,
	.lastclose = vbox_driver_lastclose,
	.master_set = vbox_master_set,
	.master_drop = vbox_master_drop,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0) || defined(RHEL_73)
# if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
	.set_busid = drm_pci_set_busid,
# endif
#endif

	.fops = &vbox_fops,
	.irq_handler = vbox_irq_handler,
	.name = DRIVER_NAME,
	.desc = DRIVER_DESC,
	.date = DRIVER_DATE,
	.major = DRIVER_MAJOR,
	.minor = DRIVER_MINOR,
	.patchlevel = DRIVER_PATCHLEVEL,

	.gem_free_object = vbox_gem_free_object,
	.dumb_create = vbox_dumb_create,
	.dumb_map_offset = vbox_dumb_mmap_offset,
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 12, 0) && !defined(RHEL_73)
	.dumb_destroy = vbox_dumb_destroy,
#else
	.dumb_destroy = drm_gem_dumb_destroy,
#endif
	.prime_handle_to_fd = drm_gem_prime_handle_to_fd,
	.prime_fd_to_handle = drm_gem_prime_fd_to_handle,
	.gem_prime_export = drm_gem_prime_export,
	.gem_prime_import = drm_gem_prime_import,
	.gem_prime_pin = vbox_gem_prime_pin,
	.gem_prime_unpin = vbox_gem_prime_unpin,
	.gem_prime_get_sg_table = vbox_gem_prime_get_sg_table,
	.gem_prime_import_sg_table = vbox_gem_prime_import_sg_table,
	.gem_prime_vmap = vbox_gem_prime_vmap,
	.gem_prime_vunmap = vbox_gem_prime_vunmap,
	.gem_prime_mmap = vbox_gem_prime_mmap,
>>>>>>> temp
};

static int __init vbox_init(void)
{
<<<<<<< HEAD
    unsigned i;

#ifdef CONFIG_VGA_CONSOLE
    if (vgacon_text_force() && vbox_modeset == -1)
        return -EINVAL;
#endif

    if (vbox_modeset == 0)
        return -EINVAL;

    /* Do not load if any of the virtual consoles is in graphics mode to be
     * sure that we do not pick a fight with a user-mode driver or VESA. */
    for (i = 0; i < MAX_NR_CONSOLES - 1; ++i)
        if (vc_cons[i].d && vc_cons[i].d->vc_mode == KD_GRAPHICS)
            return -EINVAL;

    return drm_pci_init(&driver, &vbox_pci_driver);
}
static void __exit vbox_exit(void)
{
    drm_pci_exit(&driver, &vbox_pci_driver);
=======
#if defined(CONFIG_VGA_CONSOLE) || LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)
	if (vgacon_text_force() && vbox_modeset == -1)
		return -EINVAL;
#endif

	if (vbox_modeset == 0)
		return -EINVAL;

	return pci_register_driver(&vbox_pci_driver);
}

static void __exit vbox_exit(void)
{
	pci_unregister_driver(&vbox_pci_driver);
>>>>>>> temp
}

module_init(vbox_init);
module_exit(vbox_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL and additional rights");
#ifdef MODULE_VERSION
<<<<<<< HEAD
MODULE_VERSION(VBOX_VERSION_STRING);
#endif

=======
MODULE_VERSION(VBOX_VERSION_STRING " r" __stringify(VBOX_SVN_REV));
#endif
>>>>>>> temp
