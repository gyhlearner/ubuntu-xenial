<<<<<<< HEAD
/* $Id: vbox_main.c $ */
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
 * ast_main.c
 * with the following copyright and permission notice:
 *
=======
/*
 * Copyright (C) 2013-2017 Oracle Corporation
 * This file is based on ast_main.c
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

#include <VBox/VBoxVideoGuest.h>
#include <VBox/VBoxVideo.h>
=======
 * Authors: Dave Airlie <airlied@redhat.com>,
 *          Michael Thayer <michael.thayer@oracle.com,
 *          Hans de Goede <hdegoede@redhat.com>
 */
#include "vbox_drv.h"

#include "vboxvideo_guest.h"
#include "vboxvideo_vbe.h"
>>>>>>> temp

#include <drm/drm_fb_helper.h>
#include <drm/drm_crtc_helper.h>

static void vbox_user_framebuffer_destroy(struct drm_framebuffer *fb)
{
<<<<<<< HEAD
    struct vbox_framebuffer *vbox_fb = to_vbox_framebuffer(fb);
    if (vbox_fb->obj)
        drm_gem_object_unreference_unlocked(vbox_fb->obj);

    LogFunc(("vboxvideo: %d: vbox_fb=%p, vbox_fb->obj=%p\n", __LINE__,
             vbox_fb, vbox_fb->obj));
    drm_framebuffer_cleanup(fb);
    kfree(fb);
=======
	struct vbox_framebuffer *vbox_fb = to_vbox_framebuffer(fb);

	if (vbox_fb->obj)
		drm_gem_object_unreference_unlocked(vbox_fb->obj);

	drm_framebuffer_cleanup(fb);
	kfree(fb);
>>>>>>> temp
}

void vbox_enable_accel(struct vbox_private *vbox)
{
<<<<<<< HEAD
    unsigned i;
    struct VBVABUFFER *vbva;

    AssertLogRelReturnVoid(vbox->vbva_info != NULL);
    for (i = 0; i < vbox->num_crtcs; ++i) {
        if (vbox->vbva_info[i].pVBVA == NULL) {
            LogFunc(("vboxvideo: enabling VBVA.\n"));
            vbva = (struct VBVABUFFER *) (  ((uint8_t *)vbox->vram)
                                           + vbox->vram_size
                                           + i * VBVA_MIN_BUFFER_SIZE);
            if (!VBoxVBVAEnable(&vbox->vbva_info[i], &vbox->submit_info, vbva, i))
                AssertReleaseMsgFailed(("VBoxVBVAEnable failed - heap allocation error, very old host or driver error.\n"));
        }
    }
=======
	unsigned int i;
	struct VBVABUFFER *vbva;

	if (!vbox->vbva_info || !vbox->vbva_buffers) {
		/* Should never happen... */
		DRM_ERROR("vboxvideo: failed to set up VBVA.\n");
		return;
	}

	for (i = 0; i < vbox->num_crtcs; ++i) {
		if (!vbox->vbva_info[i].vbva) {
			vbva = (struct VBVABUFFER *)
				((u8 *)vbox->vbva_buffers +
						     i * VBVA_MIN_BUFFER_SIZE);
			if (!vbva_enable(&vbox->vbva_info[i],
					    vbox->guest_pool, vbva, i)) {
				/* very old host or driver error. */
				DRM_ERROR("vboxvideo: vbva_enable failed - heap allocation error.\n");
				return;
			}
		}
	}
>>>>>>> temp
}

void vbox_disable_accel(struct vbox_private *vbox)
{
<<<<<<< HEAD
    unsigned i;

    for (i = 0; i < vbox->num_crtcs; ++i)
        VBoxVBVADisable(&vbox->vbva_info[i], &vbox->submit_info, i);
}

void vbox_enable_caps(struct vbox_private *vbox)
{
    uint32_t caps =   VBVACAPS_DISABLE_CURSOR_INTEGRATION
                     | VBVACAPS_IRQ
                     | VBVACAPS_USE_VBVA_ONLY;
    if (vbox->initial_mode_queried)
        caps |= VBVACAPS_VIDEO_MODE_HINTS;
    VBoxHGSMISendCapsInfo(&vbox->submit_info, caps);
}

/** Send information about dirty rectangles to VBVA.  If necessary we enable
 * VBVA first, as this is normally disabled after a change of master in case
 * the new master does not send dirty rectangle information (is this even
 * allowed?) */
void vbox_framebuffer_dirty_rectangles(struct drm_framebuffer *fb,
                                       struct drm_clip_rect *rects,
                                       unsigned num_rects)
{
    struct vbox_private *vbox = fb->dev->dev_private;
    unsigned i;

    LogFunc(("vboxvideo: %d: fb=%p, num_rects=%u, vbox=%p\n", __LINE__, fb,
             num_rects, vbox));
    vbox_enable_accel(vbox);
    mutex_lock(&vbox->hw_mutex);
    for (i = 0; i < num_rects; ++i)
    {
        struct drm_crtc *crtc;
        list_for_each_entry(crtc, &fb->dev->mode_config.crtc_list, head)
        {
            unsigned crtc_id = to_vbox_crtc(crtc)->crtc_id;
            VBVACMDHDR cmd_hdr;

            if (   CRTC_FB(crtc) != fb
                || rects[i].x1 >   crtc->x
                                  + crtc->hwmode.hdisplay
                || rects[i].y1 >   crtc->y
                                  + crtc->hwmode.vdisplay
                || rects[i].x2 < crtc->x
                || rects[i].y2 < crtc->y)
                continue;
            cmd_hdr.x = (int16_t)rects[i].x1;
            cmd_hdr.y = (int16_t)rects[i].y1;
            cmd_hdr.w = (uint16_t)rects[i].x2 - rects[i].x1;
            cmd_hdr.h = (uint16_t)rects[i].y2 - rects[i].y1;
            if (VBoxVBVABufferBeginUpdate(&vbox->vbva_info[crtc_id],
                                          &vbox->submit_info))
            {
                VBoxVBVAWrite(&vbox->vbva_info[crtc_id], &vbox->submit_info, &cmd_hdr,
                              sizeof(cmd_hdr));
                VBoxVBVABufferEndUpdate(&vbox->vbva_info[crtc_id]);
            }
        }
    }
    mutex_unlock(&vbox->hw_mutex);
    LogFunc(("vboxvideo: %d\n", __LINE__));
}

static int vbox_user_framebuffer_dirty(struct drm_framebuffer *fb,
                                       struct drm_file *file_priv,
                                       unsigned flags, unsigned color,
                                       struct drm_clip_rect *rects,
                                       unsigned num_rects)
{
    LogFunc(("vboxvideo: %d, flags=%u\n", __LINE__, flags));
    vbox_framebuffer_dirty_rectangles(fb, rects, num_rects);
    return 0;
}

static const struct drm_framebuffer_funcs vbox_fb_funcs = {
    .destroy = vbox_user_framebuffer_destroy,
    .dirty = vbox_user_framebuffer_dirty,
};


int vbox_framebuffer_init(struct drm_device *dev,
             struct vbox_framebuffer *vbox_fb,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)
             const
#endif
             struct DRM_MODE_FB_CMD *mode_cmd,
             struct drm_gem_object *obj)
{
    int ret;

    LogFunc(("vboxvideo: %d: dev=%p, vbox_fb=%p, obj=%p\n", __LINE__, dev,
             vbox_fb, obj));
    drm_helper_mode_fill_fb_struct(&vbox_fb->base, mode_cmd);
    vbox_fb->obj = obj;
    ret = drm_framebuffer_init(dev, &vbox_fb->base, &vbox_fb_funcs);
    if (ret) {
        DRM_ERROR("framebuffer init failed %d\n", ret);
        LogFunc(("vboxvideo: %d\n", __LINE__));
        return ret;
    }
    LogFunc(("vboxvideo: %d\n", __LINE__));
    return 0;
}

static struct drm_framebuffer *
vbox_user_framebuffer_create(struct drm_device *dev,
           struct drm_file *filp,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)
           const
#endif
           struct drm_mode_fb_cmd2 *mode_cmd)
{
    struct drm_gem_object *obj;
    struct vbox_framebuffer *vbox_fb;
    int ret;

    LogFunc(("vboxvideo: %d\n", __LINE__));
    obj = drm_gem_object_lookup(dev, filp, mode_cmd->handles[0]);
    if (obj == NULL)
        return ERR_PTR(-ENOENT);

    vbox_fb = kzalloc(sizeof(*vbox_fb), GFP_KERNEL);
    if (!vbox_fb) {
        drm_gem_object_unreference_unlocked(obj);
        return ERR_PTR(-ENOMEM);
    }

    ret = vbox_framebuffer_init(dev, vbox_fb, mode_cmd, obj);
    if (ret) {
        drm_gem_object_unreference_unlocked(obj);
        kfree(vbox_fb);
        return ERR_PTR(ret);
    }
    LogFunc(("vboxvideo: %d\n", __LINE__));
    return &vbox_fb->base;
}

static const struct drm_mode_config_funcs vbox_mode_funcs = {
    .fb_create = vbox_user_framebuffer_create,
=======
	unsigned int i;

	for (i = 0; i < vbox->num_crtcs; ++i)
		vbva_disable(&vbox->vbva_info[i], vbox->guest_pool, i);
}

void vbox_report_caps(struct vbox_private *vbox)
{
	u32 caps = VBVACAPS_DISABLE_CURSOR_INTEGRATION
	    | VBVACAPS_IRQ | VBVACAPS_USE_VBVA_ONLY;
	if (vbox->initial_mode_queried)
		caps |= VBVACAPS_VIDEO_MODE_HINTS;
	hgsmi_send_caps_info(vbox->guest_pool, caps);
}

/**
 * Send information about dirty rectangles to VBVA.  If necessary we enable
 * VBVA first, as this is normally disabled after a change of master in case
 * the new master does not send dirty rectangle information (is this even
 * allowed?)
 */
void vbox_framebuffer_dirty_rectangles(struct drm_framebuffer *fb,
				       struct drm_clip_rect *rects,
				       unsigned int num_rects)
{
	struct vbox_private *vbox = fb->dev->dev_private;
	struct drm_crtc *crtc;
	unsigned int i;

	mutex_lock(&vbox->hw_mutex);
	list_for_each_entry(crtc, &fb->dev->mode_config.crtc_list, head) {
		if (CRTC_FB(crtc) == fb) {
			vbox_enable_accel(vbox);
			for (i = 0; i < num_rects; ++i) {
				VBVACMDHDR cmd_hdr;
				unsigned int crtc_id =
				    to_vbox_crtc(crtc)->crtc_id;

				if ((rects[i].x1 >
					 crtc->x + crtc->hwmode.hdisplay) ||
				    (rects[i].y1 >
					 crtc->y + crtc->hwmode.vdisplay) ||
				    (rects[i].x2 < crtc->x) ||
				    (rects[i].y2 < crtc->y))
					continue;

				cmd_hdr.x = (s16)rects[i].x1;
				cmd_hdr.y = (s16)rects[i].y1;
				cmd_hdr.w = (u16)rects[i].x2 - rects[i].x1;
				cmd_hdr.h = (u16)rects[i].y2 - rects[i].y1;

				if (vbva_buffer_begin_update(
						&vbox->vbva_info[crtc_id],
						vbox->guest_pool)) {
					VBoxVBVAWrite(&vbox->vbva_info[crtc_id],
						      vbox->guest_pool,
						      &cmd_hdr,
						      sizeof(cmd_hdr));
					vbva_buffer_end_update(
						&vbox->vbva_info[crtc_id]);
				}
			}
		}
	}
	mutex_unlock(&vbox->hw_mutex);
}

static int vbox_user_framebuffer_dirty(struct drm_framebuffer *fb,
				       struct drm_file *file_priv,
				       unsigned int flags, unsigned int color,
				       struct drm_clip_rect *rects,
				       unsigned int num_rects)
{
	vbox_framebuffer_dirty_rectangles(fb, rects, num_rects);

	return 0;
}

static const struct drm_framebuffer_funcs vbox_fb_funcs = {
	.destroy = vbox_user_framebuffer_destroy,
	.dirty = vbox_user_framebuffer_dirty,
};

int vbox_framebuffer_init(struct drm_device *dev,
			  struct vbox_framebuffer *vbox_fb,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0) || defined(RHEL_73)
			  const
#endif
			  struct DRM_MODE_FB_CMD *mode_cmd,
			  struct drm_gem_object *obj)
{
	int ret;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
	drm_helper_mode_fill_fb_struct(dev, &vbox_fb->base, mode_cmd);
#else
	drm_helper_mode_fill_fb_struct(&vbox_fb->base, mode_cmd);
#endif
	vbox_fb->obj = obj;
	ret = drm_framebuffer_init(dev, &vbox_fb->base, &vbox_fb_funcs);
	if (ret) {
		DRM_ERROR("framebuffer init failed %d\n", ret);
		return ret;
	}

	return 0;
}

static struct drm_framebuffer *vbox_user_framebuffer_create(
		struct drm_device *dev,
		struct drm_file *filp,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0) || defined(RHEL_73)
		const struct drm_mode_fb_cmd2 *mode_cmd)
#else
		struct drm_mode_fb_cmd2 *mode_cmd)
#endif
{
	struct drm_gem_object *obj;
	struct vbox_framebuffer *vbox_fb;
	int ret;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0) || defined(RHEL_74)
	obj = drm_gem_object_lookup(filp, mode_cmd->handles[0]);
#else
	obj = drm_gem_object_lookup(dev, filp, mode_cmd->handles[0]);
#endif
	if (!obj)
		return ERR_PTR(-ENOENT);

	vbox_fb = kzalloc(sizeof(*vbox_fb), GFP_KERNEL);
	if (!vbox_fb) {
		drm_gem_object_unreference_unlocked(obj);
		return ERR_PTR(-ENOMEM);
	}

	ret = vbox_framebuffer_init(dev, vbox_fb, mode_cmd, obj);
	if (ret) {
		drm_gem_object_unreference_unlocked(obj);
		kfree(vbox_fb);
		return ERR_PTR(ret);
	}

	return &vbox_fb->base;
}

static const struct drm_mode_config_funcs vbox_mode_funcs = {
	.fb_create = vbox_user_framebuffer_create,
>>>>>>> temp
};

static void vbox_accel_fini(struct vbox_private *vbox)
{
<<<<<<< HEAD
    if (vbox->vbva_info)
    {
        vbox_disable_accel(vbox);
        kfree(vbox->vbva_info);
        vbox->vbva_info = NULL;
    }
}

static int vbox_accel_init(struct vbox_private *vbox)
{
    unsigned i;
    LogFunc(("vboxvideo: %d: vbox=%p, vbox->num_crtcs=%u, vbox->vbva_info=%p\n",
             __LINE__, vbox, (unsigned)vbox->num_crtcs, vbox->vbva_info));
    if (!vbox->vbva_info)
    {
        vbox->vbva_info = kzalloc(  sizeof(struct VBVABUFFERCONTEXT)
                                  * vbox->num_crtcs,
                                  GFP_KERNEL);
        if (!vbox->vbva_info)
            return -ENOMEM;
    }
    /* Take a command buffer for each screen from the end of usable VRAM. */
    vbox->vram_size -= vbox->num_crtcs * VBVA_MIN_BUFFER_SIZE;
    for (i = 0; i < vbox->num_crtcs; ++i)
        VBoxVBVASetupBufferContext(&vbox->vbva_info[i],
                                   vbox->vram_size + i * VBVA_MIN_BUFFER_SIZE,
                                   VBVA_MIN_BUFFER_SIZE);
    LogFunc(("vboxvideo: %d: vbox->vbva_info=%p, vbox->vram_size=%u\n",
             __LINE__, vbox->vbva_info, (unsigned)vbox->vram_size));
    return 0;
}

/** Allocation function for the HGSMI heap and data. */
static DECLCALLBACK(void *) alloc_hgsmi_environ(void *environ, HGSMISIZE size)
{
    NOREF(environ);
    return kmalloc(size, GFP_KERNEL);
}


/** Free function for the HGSMI heap and data. */
static DECLCALLBACK(void) free_hgsmi_environ(void *environ, void *ptr)
{
    NOREF(environ);
    kfree(ptr);
}


/** Pointers to the HGSMI heap and data manipulation functions. */
static HGSMIENV hgsmi_environ =
{
    NULL,
    alloc_hgsmi_environ,
    free_hgsmi_environ
};


/** Do we support the 4.3 plus mode hint reporting interface? */
static bool have_hgsmi_mode_hints(struct vbox_private *vbox)
{
    uint32_t have_hints, have_cursor;

    return    RT_SUCCESS(VBoxQueryConfHGSMI(&vbox->submit_info, VBOX_VBVA_CONF32_MODE_HINT_REPORTING, &have_hints))
           && RT_SUCCESS(VBoxQueryConfHGSMI(&vbox->submit_info, VBOX_VBVA_CONF32_GUEST_CURSOR_REPORTING, &have_cursor))
           && have_hints == VINF_SUCCESS
           && have_cursor == VINF_SUCCESS;
}


/** Set up our heaps and data exchange buffers in VRAM before handing the rest
 *  to the memory manager. */
static int vbox_hw_init(struct vbox_private *vbox)
{
    uint32_t base_offset, guest_heap_offset, guest_heap_size, host_flags_offset;
    void *guest_heap;

    vbox->full_vram_size = VBoxVideoGetVRAMSize();
    vbox->any_pitch = VBoxVideoAnyWidthAllowed();
    DRM_INFO("VRAM %08x\n", vbox->full_vram_size);
    VBoxHGSMIGetBaseMappingInfo(vbox->full_vram_size, &base_offset, NULL,
                                &guest_heap_offset, &guest_heap_size, &host_flags_offset);
    guest_heap =   ((uint8_t *)vbox->vram) + base_offset + guest_heap_offset;
    vbox->host_flags_offset = base_offset + host_flags_offset;
    if (RT_FAILURE(VBoxHGSMISetupGuestContext(&vbox->submit_info, guest_heap,
                                              guest_heap_size,
                                              base_offset + guest_heap_offset,
                                              &hgsmi_environ)))
        return -ENOMEM;
    /* Reduce available VRAM size to reflect the guest heap. */
    vbox->vram_size = base_offset;
    /* Linux drm represents monitors as a 32-bit array. */
    vbox->num_crtcs = RT_MIN(VBoxHGSMIGetMonitorCount(&vbox->submit_info), 32);
    if (!have_hgsmi_mode_hints(vbox))
        return -ENOTSUPP;
    vbox->last_mode_hints = kzalloc(sizeof(VBVAMODEHINT) * vbox->num_crtcs, GFP_KERNEL);
    if (!vbox->last_mode_hints)
        return -ENOMEM;
    return vbox_accel_init(vbox);
=======
	if (vbox->vbva_info) {
		vbox_disable_accel(vbox);
		kfree(vbox->vbva_info);
		vbox->vbva_info = NULL;
	}
	if (vbox->vbva_buffers) {
		pci_iounmap(vbox->dev->pdev, vbox->vbva_buffers);
		vbox->vbva_buffers = NULL;
	}
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0) && !defined(RHEL_73)
#define pci_iomap_range(dev, bar, offset, maxlen) \
	ioremap(pci_resource_start(dev, bar) + (offset), maxlen)
#endif

static int vbox_accel_init(struct vbox_private *vbox)
{
	unsigned int i;

	vbox->vbva_info = kcalloc(vbox->num_crtcs, sizeof(*vbox->vbva_info),
				  GFP_KERNEL);
	if (!vbox->vbva_info)
		return -ENOMEM;

	/* Take a command buffer for each screen from the end of usable VRAM. */
	vbox->available_vram_size -= vbox->num_crtcs * VBVA_MIN_BUFFER_SIZE;

	vbox->vbva_buffers = pci_iomap_range(vbox->dev->pdev, 0,
					     vbox->available_vram_size,
					     vbox->num_crtcs *
					     VBVA_MIN_BUFFER_SIZE);
	if (!vbox->vbva_buffers)
		return -ENOMEM;

	for (i = 0; i < vbox->num_crtcs; ++i)
		VBoxVBVASetupBufferContext(&vbox->vbva_info[i],
					   vbox->available_vram_size +
					   i * VBVA_MIN_BUFFER_SIZE,
					   VBVA_MIN_BUFFER_SIZE);

	return 0;
}

/** Do we support the 4.3 plus mode hint reporting interface? */
static bool have_hgsmi_mode_hints(struct vbox_private *vbox)
{
	u32 have_hints, have_cursor;
	int ret;

	ret = hgsmi_query_conf(vbox->guest_pool,
				 VBOX_VBVA_CONF32_MODE_HINT_REPORTING,
				 &have_hints);
	if (RT_FAILURE(ret))
		return false;

	ret = hgsmi_query_conf(vbox->guest_pool,
				 VBOX_VBVA_CONF32_GUEST_CURSOR_REPORTING,
				 &have_cursor);
	if (RT_FAILURE(ret))
		return false;

	return have_hints == VINF_SUCCESS && have_cursor == VINF_SUCCESS;
}

/**
 * Set up our heaps and data exchange buffers in VRAM before handing the rest
 * to the memory manager.
 */
static int vbox_hw_init(struct vbox_private *vbox)
{
	int ret;

	vbox->full_vram_size = VBoxVideoGetVRAMSize();
	vbox->any_pitch = VBoxVideoAnyWidthAllowed();

	DRM_INFO("VRAM %08x\n", vbox->full_vram_size);

	/* Map guest-heap at end of vram */
	vbox->guest_heap =
	    pci_iomap_range(vbox->dev->pdev, 0, GUEST_HEAP_OFFSET(vbox),
			    GUEST_HEAP_SIZE);
	if (!vbox->guest_heap)
		return -ENOMEM;

	/* Create guest-heap mem-pool use 2^4 = 16 byte chunks */
	vbox->guest_pool = gen_pool_create(4, -1);
	if (!vbox->guest_pool)
		return -ENOMEM;

	ret = gen_pool_add_virt(vbox->guest_pool,
				(unsigned long)vbox->guest_heap,
				GUEST_HEAP_OFFSET(vbox),
				GUEST_HEAP_USABLE_SIZE, -1);
	if (ret)
		return ret;

	/* Reduce available VRAM size to reflect the guest heap. */
	vbox->available_vram_size = GUEST_HEAP_OFFSET(vbox);
	/* Linux drm represents monitors as a 32-bit array. */
	vbox->num_crtcs = min_t(u32, VBoxHGSMIGetMonitorCount(vbox->guest_pool),
				VBOX_MAX_SCREENS);

	if (!have_hgsmi_mode_hints(vbox))
		return -ENOTSUPP;

	vbox->last_mode_hints =
	    kcalloc(vbox->num_crtcs, sizeof(struct vbva_modehint), GFP_KERNEL);
	if (!vbox->last_mode_hints)
		return -ENOMEM;

	return vbox_accel_init(vbox);
>>>>>>> temp
}

static void vbox_hw_fini(struct vbox_private *vbox)
{
<<<<<<< HEAD
    vbox_accel_fini(vbox);
    if (vbox->last_mode_hints)
        kfree(vbox->last_mode_hints);
    vbox->last_mode_hints = NULL;
=======
	vbox_accel_fini(vbox);
	kfree(vbox->last_mode_hints);
	vbox->last_mode_hints = NULL;
>>>>>>> temp
}

int vbox_driver_load(struct drm_device *dev, unsigned long flags)
{
<<<<<<< HEAD
    struct vbox_private *vbox;
    int ret = 0;

    LogFunc(("vboxvideo: %d: dev=%p\n", __LINE__, dev));
    if (!VBoxHGSMIIsSupported())
        return -ENODEV;
    vbox = kzalloc(sizeof(struct vbox_private), GFP_KERNEL);
    if (!vbox)
        return -ENOMEM;

    dev->dev_private = vbox;
    vbox->dev = dev;

    mutex_init(&vbox->hw_mutex);
    /* I hope this won't interfere with the memory manager. */
    vbox->vram = pci_iomap(dev->pdev, 0, 0);
    if (!vbox->vram) {
        ret = -EIO;
        goto out_free;
    }

    ret = vbox_hw_init(vbox);
    if (ret)
        goto out_free;

    ret = vbox_mm_init(vbox);
    if (ret)
        goto out_free;

    drm_mode_config_init(dev);

    dev->mode_config.funcs = (void *)&vbox_mode_funcs;
    dev->mode_config.min_width = 64;
    dev->mode_config.min_height = 64;
    dev->mode_config.preferred_depth = 24;
    dev->mode_config.max_width = VBE_DISPI_MAX_XRES;
    dev->mode_config.max_height = VBE_DISPI_MAX_YRES;

    ret = vbox_mode_init(dev);
    if (ret)
        goto out_free;

    ret = vbox_irq_init(vbox);
    if (ret)
        goto out_free;

    ret = vbox_fbdev_init(dev);
    if (ret)
        goto out_free;
    LogFunc(("vboxvideo: %d: vbox=%p, vbox->vram=%p, vbox->full_vram_size=%u\n",
             __LINE__, vbox, vbox->vram, (unsigned)vbox->full_vram_size));
    return 0;
out_free:
    vbox_driver_unload(dev);
    LogFunc(("vboxvideo: %d: ret=%d\n", __LINE__, ret));
    return ret;
}

int vbox_driver_unload(struct drm_device *dev)
{
    struct vbox_private *vbox = dev->dev_private;

    LogFunc(("vboxvideo: %d\n", __LINE__));
    vbox_fbdev_fini(dev);
    vbox_irq_fini(vbox);
    vbox_mode_fini(dev);
    if (dev->mode_config.funcs)
        drm_mode_config_cleanup(dev);

    vbox_hw_fini(vbox);
    vbox_mm_fini(vbox);
    if (vbox->vram)
        pci_iounmap(dev->pdev, vbox->vram);
    kfree(vbox);
    dev->dev_private = NULL;
    LogFunc(("vboxvideo: %d\n", __LINE__));
    return 0;
}

/** @note this is described in the DRM framework documentation.  AST does not
 *        have it, but we get an oops on driver unload if it is not present. */
void vbox_driver_lastclose(struct drm_device *dev)
{
    struct vbox_private *vbox = dev->dev_private;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0)
    if (vbox->fbdev)
        drm_fb_helper_restore_fbdev_mode_unlocked(&vbox->fbdev->helper);
#else
    mutex_lock(&dev->mode_config.mutex);
    if (vbox->fbdev)
        drm_fb_helper_restore_fbdev_mode(&vbox->fbdev->helper);
    mutex_unlock(&dev->mode_config.mutex);
=======
	struct vbox_private *vbox;
	int ret = 0;

	if (!VBoxHGSMIIsSupported())
		return -ENODEV;

	vbox = kzalloc(sizeof(*vbox), GFP_KERNEL);
	if (!vbox)
		return -ENOMEM;

	dev->dev_private = vbox;
	vbox->dev = dev;

	mutex_init(&vbox->hw_mutex);

	ret = vbox_hw_init(vbox);
	if (ret)
		goto out_free;

	ret = vbox_mm_init(vbox);
	if (ret)
		goto out_free;

	drm_mode_config_init(dev);

	dev->mode_config.funcs = (void *)&vbox_mode_funcs;
	dev->mode_config.min_width = 64;
	dev->mode_config.min_height = 64;
	dev->mode_config.preferred_depth = 24;
	dev->mode_config.max_width = VBE_DISPI_MAX_XRES;
	dev->mode_config.max_height = VBE_DISPI_MAX_YRES;

	ret = vbox_mode_init(dev);
	if (ret)
		goto out_free;

	ret = vbox_irq_init(vbox);
	if (ret)
		goto out_free;

	ret = vbox_fbdev_init(dev);
	if (ret)
		goto out_free;

	return 0;

out_free:
	vbox_driver_unload(dev);
	return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
void vbox_driver_unload(struct drm_device *dev)
#else
int vbox_driver_unload(struct drm_device *dev)
#endif
{
	struct vbox_private *vbox = dev->dev_private;

	vbox_fbdev_fini(dev);
	vbox_irq_fini(vbox);
	vbox_mode_fini(dev);
	if (dev->mode_config.funcs)
		drm_mode_config_cleanup(dev);

	vbox_hw_fini(vbox);
	vbox_mm_fini(vbox);
	if (vbox->guest_pool)
		gen_pool_destroy(vbox->guest_pool);
	if (vbox->guest_heap)
		pci_iounmap(dev->pdev, vbox->guest_heap);
	kfree(vbox);
	dev->dev_private = NULL;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0)
	return 0;
#endif
}

/**
 * @note this is described in the DRM framework documentation.  AST does not
 * have it, but we get an oops on driver unload if it is not present.
 */
void vbox_driver_lastclose(struct drm_device *dev)
{
	struct vbox_private *vbox = dev->dev_private;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0) || defined(RHEL_73)
	if (vbox->fbdev)
		drm_fb_helper_restore_fbdev_mode_unlocked(&vbox->fbdev->helper);
#else
	drm_modeset_lock_all(dev);
	if (vbox->fbdev)
		drm_fb_helper_restore_fbdev_mode(&vbox->fbdev->helper);
	drm_modeset_unlock_all(dev);
>>>>>>> temp
#endif
}

int vbox_gem_create(struct drm_device *dev,
<<<<<<< HEAD
           u32 size, bool iskernel,
           struct drm_gem_object **obj)
{
    struct vbox_bo *vboxbo;
    int ret;

    LogFunc(("vboxvideo: %d: dev=%p, size=%u, iskernel=%u\n", __LINE__,
             dev, (unsigned)size, (unsigned)iskernel));
    *obj = NULL;

    size = roundup(size, PAGE_SIZE);
    if (size == 0)
        return -EINVAL;

    ret = vbox_bo_create(dev, size, 0, 0, &vboxbo);
    if (ret) {
        if (ret != -ERESTARTSYS)
            DRM_ERROR("failed to allocate GEM object\n");
        return ret;
    }
    *obj = &vboxbo->gem;
    LogFunc(("vboxvideo: %d: obj=%p\n", __LINE__, obj));
    return 0;
}

int vbox_dumb_create(struct drm_file *file,
            struct drm_device *dev,
            struct drm_mode_create_dumb *args)
{
    int ret;
    struct drm_gem_object *gobj;
    u32 handle;

    LogFunc(("vboxvideo: %d: args->width=%u, args->height=%u, args->bpp=%u\n",
             __LINE__, (unsigned)args->width, (unsigned)args->height,
             (unsigned)args->bpp));
    args->pitch = args->width * ((args->bpp + 7) / 8);
    args->size = args->pitch * args->height;

    ret = vbox_gem_create(dev, args->size, false,
                 &gobj);
    if (ret)
        return ret;

    ret = drm_gem_handle_create(file, gobj, &handle);
    drm_gem_object_unreference_unlocked(gobj);
    if (ret)
        return ret;

    args->handle = handle;
    LogFunc(("vboxvideo: %d: args->handle=%u\n", __LINE__,
             (unsigned)args->handle));
    return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 12, 0)
int vbox_dumb_destroy(struct drm_file *file,
             struct drm_device *dev,
             uint32_t handle)
{
    LogFunc(("vboxvideo: %d: dev=%p, handle=%u\n", __LINE__, dev,
             (unsigned)handle));
    return drm_gem_handle_delete(file, handle);
=======
		    u32 size, bool iskernel, struct drm_gem_object **obj)
{
	struct vbox_bo *vboxbo;
	int ret;

	*obj = NULL;

	size = roundup(size, PAGE_SIZE);
	if (size == 0)
		return -EINVAL;

	ret = vbox_bo_create(dev, size, 0, 0, &vboxbo);
	if (ret) {
		if (ret != -ERESTARTSYS)
			DRM_ERROR("failed to allocate GEM object\n");
		return ret;
	}

	*obj = &vboxbo->gem;

	return 0;
}

int vbox_dumb_create(struct drm_file *file,
		     struct drm_device *dev, struct drm_mode_create_dumb *args)
{
	int ret;
	struct drm_gem_object *gobj;
	u32 handle;

	args->pitch = args->width * ((args->bpp + 7) / 8);
	args->size = args->pitch * args->height;

	ret = vbox_gem_create(dev, args->size, false, &gobj);
	if (ret)
		return ret;

	ret = drm_gem_handle_create(file, gobj, &handle);
	drm_gem_object_unreference_unlocked(gobj);
	if (ret)
		return ret;

	args->handle = handle;

	return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 12, 0) && !defined(RHEL_73)
int vbox_dumb_destroy(struct drm_file *file,
		      struct drm_device *dev, u32 handle)
{
	return drm_gem_handle_delete(file, handle);
>>>>>>> temp
}
#endif

static void vbox_bo_unref(struct vbox_bo **bo)
{
<<<<<<< HEAD
    struct ttm_buffer_object *tbo;

    if ((*bo) == NULL)
        return;

    LogFunc(("vboxvideo: %d: bo=%p\n", __LINE__, bo));
    tbo = &((*bo)->bo);
    ttm_bo_unref(&tbo);
    if (tbo == NULL)
        *bo = NULL;

}
void vbox_gem_free_object(struct drm_gem_object *obj)
{
    struct vbox_bo *vbox_bo = gem_to_vbox_bo(obj);

    LogFunc(("vboxvideo: %d: vbox_bo=%p\n", __LINE__, vbox_bo));
    vbox_bo_unref(&vbox_bo);
}


static inline u64 vbox_bo_mmap_offset(struct vbox_bo *bo)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 12, 0)
    return bo->bo.addr_space_offset;
#else
    return drm_vma_node_offset_addr(&bo->bo.vma_node);
#endif
}
int
vbox_dumb_mmap_offset(struct drm_file *file,
             struct drm_device *dev,
             uint32_t handle,
             uint64_t *offset)
{
    struct drm_gem_object *obj;
    int ret;
    struct vbox_bo *bo;

    LogFunc(("vboxvideo: %d: dev=%p, handle=%u\n", __LINE__,
             dev, (unsigned)handle));
    mutex_lock(&dev->struct_mutex);
    obj = drm_gem_object_lookup(dev, file, handle);
    if (obj == NULL) {
        ret = -ENOENT;
        goto out_unlock;
    }

    bo = gem_to_vbox_bo(obj);
    *offset = vbox_bo_mmap_offset(bo);

    drm_gem_object_unreference(obj);
    ret = 0;
    LogFunc(("vboxvideo: %d: bo=%p, *offset=%llu\n", __LINE__,
             bo, (unsigned long long)*offset));
out_unlock:
    mutex_unlock(&dev->struct_mutex);
    return ret;

=======
	struct ttm_buffer_object *tbo;

	if ((*bo) == NULL)
		return;

	tbo = &((*bo)->bo);
	ttm_bo_unref(&tbo);
	if (!tbo)
		*bo = NULL;
}

void vbox_gem_free_object(struct drm_gem_object *obj)
{
	struct vbox_bo *vbox_bo = gem_to_vbox_bo(obj);

	vbox_bo_unref(&vbox_bo);
}

static inline u64 vbox_bo_mmap_offset(struct vbox_bo *bo)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 12, 0) && !defined(RHEL_73)
	return bo->bo.addr_space_offset;
#else
	return drm_vma_node_offset_addr(&bo->bo.vma_node);
#endif
}

int
vbox_dumb_mmap_offset(struct drm_file *file,
		      struct drm_device *dev,
		      u32 handle, u64 *offset)
{
	struct drm_gem_object *obj;
	int ret;
	struct vbox_bo *bo;

	mutex_lock(&dev->struct_mutex);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0) || defined(RHEL_74)
	obj = drm_gem_object_lookup(file, handle);
#else
	obj = drm_gem_object_lookup(dev, file, handle);
#endif
	if (!obj) {
		ret = -ENOENT;
		goto out_unlock;
	}

	bo = gem_to_vbox_bo(obj);
	*offset = vbox_bo_mmap_offset(bo);

	drm_gem_object_unreference(obj);
	ret = 0;

out_unlock:
	mutex_unlock(&dev->struct_mutex);
	return ret;
>>>>>>> temp
}
