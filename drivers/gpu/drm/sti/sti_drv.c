/*
 * Copyright (C) STMicroelectronics SA 2014
 * Author: Benjamin Gaignard <benjamin.gaignard@st.com> for STMicroelectronics.
 * License terms:  GNU General Public License (GPL), version 2
 */

#include <drm/drmP.h>

#include <linux/component.h>
#include <linux/debugfs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_platform.h>

#include <drm/drm_atomic.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_gem_cma_helper.h>
#include <drm/drm_gem_framebuffer_helper.h>
#include <drm/drm_fb_cma_helper.h>
#include <drm/drm_of.h>

#include "sti_crtc.h"
#include "sti_drv.h"
#include "sti_plane.h"

#define DRIVER_NAME	"sti"
#define DRIVER_DESC	"STMicroelectronics SoC DRM"
#define DRIVER_DATE	"20140601"
#define DRIVER_MAJOR	1
#define DRIVER_MINOR	0

#define STI_MAX_FB_HEIGHT	4096
#define STI_MAX_FB_WIDTH	4096

static int sti_drm_fps_get(void *data, u64 *val)
{
	struct drm_device *drm_dev = data;
	struct drm_plane *p;
	unsigned int i = 0;

	*val = 0;
	list_for_each_entry(p, &drm_dev->mode_config.plane_list, head) {
		struct sti_plane *plane = to_sti_plane(p);

		*val |= plane->fps_info.output << i;
		i++;
	}

	return 0;
}

static int sti_drm_fps_set(void *data, u64 val)
{
	struct drm_device *drm_dev = data;
	struct drm_plane *p;
	unsigned int i = 0;

	list_for_each_entry(p, &drm_dev->mode_config.plane_list, head) {
		struct sti_plane *plane = to_sti_plane(p);

		memset(&plane->fps_info, 0, sizeof(plane->fps_info));
		plane->fps_info.output = (val >> i) & 1;

		i++;
	}

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(sti_drm_fps_fops,
			sti_drm_fps_get, sti_drm_fps_set, "%llu\n");

static int sti_drm_fps_dbg_show(struct seq_file *s, void *data)
{
	struct drm_info_node *node = s->private;
	struct drm_device *dev = node->minor->dev;
	struct drm_plane *p;

	list_for_each_entry(p, &dev->mode_config.plane_list, head) {
		struct sti_plane *plane = to_sti_plane(p);

		seq_printf(s, "%s%s\n",
			   plane->fps_info.fps_str,
			   plane->fps_info.fips_str);
	}

	return 0;
}

static struct drm_info_list sti_drm_dbg_list[] = {
	{"fps_get", sti_drm_fps_dbg_show, 0},
};

static int sti_drm_dbg_init(struct drm_minor *minor)
{
	struct dentry *dentry;
	int ret;

	ret = drm_debugfs_create_files(sti_drm_dbg_list,
				       ARRAY_SIZE(sti_drm_dbg_list),
				       minor->debugfs_root, minor);
	if (ret)
		goto err;

	dentry = debugfs_create_file("fps_show", S_IRUGO | S_IWUSR,
				     minor->debugfs_root, minor->dev,
				     &sti_drm_fps_fops);
	if (!dentry) {
		ret = -ENOMEM;
		goto err;
	}

	DRM_INFO("%s: debugfs installed\n", DRIVER_NAME);
	return 0;
err:
	DRM_ERROR("%s: cannot install debugfs\n", DRIVER_NAME);
	return ret;
}

static int sti_atomic_check(struct drm_device *dev,
			    struct drm_atomic_state *state)
{
	int ret;

	ret = drm_atomic_helper_check_modeset(dev, state);
	if (ret)
		return ret;

	ret = drm_atomic_normalize_zpos(dev, state);
	if (ret)
		return ret;

	ret = drm_atomic_helper_check_planes(dev, state);
	if (ret)
		return ret;

	return ret;
}

static void sti_output_poll_changed(struct drm_device *ddev)
{
	struct sti_private *private = ddev->dev_private;

	drm_fbdev_cma_hotplug_event(private->fbdev);
}

static const struct drm_mode_config_funcs sti_mode_config_funcs = {
	.fb_create = drm_gem_fb_create,
	.output_poll_changed = sti_output_poll_changed,
	.atomic_check = sti_atomic_check,
	.atomic_commit = drm_atomic_helper_commit,
};

static void sti_mode_config_init(struct drm_device *dev)
{
	dev->mode_config.min_width = 0;
	dev->mode_config.min_height = 0;

	/*
	 * set max width and height as default value.
	 * this value would be used to check framebuffer size limitation
	 * at drm_mode_addfb().
	 */
	dev->mode_config.max_width = STI_MAX_FB_WIDTH;
	dev->mode_config.max_height = STI_MAX_FB_HEIGHT;

	dev->mode_config.funcs = &sti_mode_config_funcs;
}

<<<<<<< HEAD
static int sti_load(struct drm_device *dev, unsigned long flags)
{
	struct sti_private *private;
	int ret;

	private = kzalloc(sizeof(*private), GFP_KERNEL);
	if (!private) {
		DRM_ERROR("Failed to allocate private\n");
		return -ENOMEM;
	}
	dev->dev_private = (void *)private;
	private->drm_dev = dev;

	mutex_init(&private->commit.lock);
	INIT_WORK(&private->commit.work, sti_atomic_work);

	drm_mode_config_init(dev);
	drm_kms_helper_poll_init(dev);

	sti_mode_config_init(dev);

	ret = component_bind_all(dev->dev, dev);
	if (ret) {
		drm_kms_helper_poll_fini(dev);
		drm_mode_config_cleanup(dev);
		kfree(private);
		return ret;
	}

	drm_mode_config_reset(dev);

	drm_helper_disable_unused_functions(dev);
	drm_fbdev_cma_init(dev, 32,
			   dev->mode_config.num_crtc,
			   dev->mode_config.num_connector);

	return 0;
}

static const struct file_operations sti_driver_fops = {
	.owner = THIS_MODULE,
	.open = drm_open,
	.mmap = drm_gem_cma_mmap,
	.poll = drm_poll,
	.read = drm_read,
	.unlocked_ioctl = drm_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = drm_compat_ioctl,
#endif
	.release = drm_release,
};

static struct dma_buf *sti_gem_prime_export(struct drm_device *dev,
					    struct drm_gem_object *obj,
					    int flags)
{
	/* we want to be able to write in mmapped buffer */
	flags |= O_RDWR;
	return drm_gem_prime_export(dev, obj, flags);
}
=======
DEFINE_DRM_GEM_CMA_FOPS(sti_driver_fops);
>>>>>>> temp

static struct drm_driver sti_driver = {
	.driver_features = DRIVER_MODESET |
	    DRIVER_GEM | DRIVER_PRIME | DRIVER_ATOMIC,
	.gem_free_object_unlocked = drm_gem_cma_free_object,
	.gem_vm_ops = &drm_gem_cma_vm_ops,
	.dumb_create = drm_gem_cma_dumb_create,
	.fops = &sti_driver_fops,

	.enable_vblank = sti_crtc_enable_vblank,
	.disable_vblank = sti_crtc_disable_vblank,

	.prime_handle_to_fd = drm_gem_prime_handle_to_fd,
	.prime_fd_to_handle = drm_gem_prime_fd_to_handle,
	.gem_prime_export = drm_gem_prime_export,
	.gem_prime_import = drm_gem_prime_import,
	.gem_prime_get_sg_table = drm_gem_cma_prime_get_sg_table,
	.gem_prime_import_sg_table = drm_gem_cma_prime_import_sg_table,
	.gem_prime_vmap = drm_gem_cma_prime_vmap,
	.gem_prime_vunmap = drm_gem_cma_prime_vunmap,
	.gem_prime_mmap = drm_gem_cma_prime_mmap,

	.debugfs_init = sti_drm_dbg_init,

	.name = DRIVER_NAME,
	.desc = DRIVER_DESC,
	.date = DRIVER_DATE,
	.major = DRIVER_MAJOR,
	.minor = DRIVER_MINOR,
};

static int compare_of(struct device *dev, void *data)
{
	return dev->of_node == data;
}

static int sti_init(struct drm_device *ddev)
{
	struct sti_private *private;

	private = kzalloc(sizeof(*private), GFP_KERNEL);
	if (!private)
		return -ENOMEM;

	ddev->dev_private = (void *)private;
	dev_set_drvdata(ddev->dev, ddev);
	private->drm_dev = ddev;

	drm_mode_config_init(ddev);

	sti_mode_config_init(ddev);

	drm_kms_helper_poll_init(ddev);

	return 0;
}

static void sti_cleanup(struct drm_device *ddev)
{
	struct sti_private *private = ddev->dev_private;

	if (private->fbdev) {
		drm_fbdev_cma_fini(private->fbdev);
		private->fbdev = NULL;
	}

	drm_kms_helper_poll_fini(ddev);
	component_unbind_all(ddev->dev, ddev);
	kfree(private);
	ddev->dev_private = NULL;
}

static int sti_bind(struct device *dev)
{
	struct drm_device *ddev;
	struct sti_private *private;
	struct drm_fbdev_cma *fbdev;
	int ret;

	ddev = drm_dev_alloc(&sti_driver, dev);
	if (IS_ERR(ddev))
		return PTR_ERR(ddev);

	ret = sti_init(ddev);
	if (ret)
		goto err_drm_dev_unref;

	ret = component_bind_all(ddev->dev, ddev);
	if (ret)
		goto err_cleanup;

	ret = drm_dev_register(ddev, 0);
	if (ret)
		goto err_register;

	drm_mode_config_reset(ddev);

	private = ddev->dev_private;
	if (ddev->mode_config.num_connector) {
		fbdev = drm_fbdev_cma_init(ddev, 32,
					   ddev->mode_config.num_connector);
		if (IS_ERR(fbdev)) {
			DRM_DEBUG_DRIVER("Warning: fails to create fbdev\n");
			fbdev = NULL;
		}
		private->fbdev = fbdev;
	}

	return 0;

err_register:
	drm_mode_config_cleanup(ddev);
err_cleanup:
	sti_cleanup(ddev);
err_drm_dev_unref:
	drm_dev_unref(ddev);
	return ret;
}

static void sti_unbind(struct device *dev)
{
	struct drm_device *ddev = dev_get_drvdata(dev);

	drm_dev_unregister(ddev);
	sti_cleanup(ddev);
	drm_dev_unref(ddev);
}

static const struct component_master_ops sti_ops = {
	.bind = sti_bind,
	.unbind = sti_unbind,
};

static int sti_platform_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;
	struct device_node *child_np;
	struct component_match *match = NULL;

	dma_set_coherent_mask(dev, DMA_BIT_MASK(32));

	devm_of_platform_populate(dev);

	child_np = of_get_next_available_child(node, NULL);

	while (child_np) {
		drm_of_component_match_add(dev, &match, compare_of,
					   child_np);
		child_np = of_get_next_available_child(node, child_np);
	}

	return component_master_add_with_match(dev, &sti_ops, match);
}

static int sti_platform_remove(struct platform_device *pdev)
{
	component_master_del(&pdev->dev, &sti_ops);

	return 0;
}

static const struct of_device_id sti_dt_ids[] = {
	{ .compatible = "st,sti-display-subsystem", },
	{ /* end node */ },
};
MODULE_DEVICE_TABLE(of, sti_dt_ids);

static struct platform_driver sti_platform_driver = {
	.probe = sti_platform_probe,
	.remove = sti_platform_remove,
	.driver = {
		.name = DRIVER_NAME,
		.of_match_table = sti_dt_ids,
	},
};

static struct platform_driver * const drivers[] = {
	&sti_tvout_driver,
	&sti_hqvdp_driver,
	&sti_hdmi_driver,
	&sti_hda_driver,
	&sti_dvo_driver,
	&sti_vtg_driver,
	&sti_compositor_driver,
	&sti_platform_driver,
};

static int sti_drm_init(void)
{
	return platform_register_drivers(drivers, ARRAY_SIZE(drivers));
}
module_init(sti_drm_init);

static void sti_drm_exit(void)
{
	platform_unregister_drivers(drivers, ARRAY_SIZE(drivers));
}
module_exit(sti_drm_exit);

MODULE_AUTHOR("Benjamin Gaignard <benjamin.gaignard@st.com>");
MODULE_DESCRIPTION("STMicroelectronics SoC DRM driver");
MODULE_LICENSE("GPL");
