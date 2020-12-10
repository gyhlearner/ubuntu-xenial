/*
 * Copyright 2015 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */
<<<<<<< HEAD
=======
#include "pp_debug.h"
>>>>>>> temp
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include "amd_shared.h"
#include "amd_powerplay.h"
#include "pp_instance.h"
#include "power_state.h"
<<<<<<< HEAD
#include "eventmanager.h"

#define PP_CHECK(handle)						\
	do {								\
		if ((handle) == NULL || (handle)->pp_valid != PP_VALID)	\
			return -EINVAL;					\
	} while (0)

static int pp_early_init(void *handle)
{
	return 0;
}

static int pp_sw_init(void *handle)
{
	struct pp_instance *pp_handle;
	struct pp_hwmgr  *hwmgr;
	int ret = 0;

	if (handle == NULL)
		return -EINVAL;

	pp_handle = (struct pp_instance *)handle;
	hwmgr = pp_handle->hwmgr;

	if (hwmgr == NULL || hwmgr->pptable_func == NULL ||
	    hwmgr->hwmgr_func == NULL ||
	    hwmgr->pptable_func->pptable_init == NULL ||
	    hwmgr->hwmgr_func->backend_init == NULL)
		return -EINVAL;

	ret = hwmgr->pptable_func->pptable_init(hwmgr);

	if (ret == 0)
		ret = hwmgr->hwmgr_func->backend_init(hwmgr);

	if (ret)
		printk("amdgpu: powerplay initialization failed\n");
	else
		printk("amdgpu: powerplay initialized\n");

	return ret;
}

static int pp_sw_fini(void *handle)
{
	struct pp_instance *pp_handle;
	struct pp_hwmgr  *hwmgr;
	int ret = 0;

	if (handle == NULL)
		return -EINVAL;

	pp_handle = (struct pp_instance *)handle;
	hwmgr = pp_handle->hwmgr;

	if (hwmgr != NULL || hwmgr->hwmgr_func != NULL ||
	    hwmgr->hwmgr_func->backend_fini != NULL)
		ret = hwmgr->hwmgr_func->backend_fini(hwmgr);

	return ret;
}

static int pp_hw_init(void *handle)
{
	struct pp_instance *pp_handle;
	struct pp_smumgr *smumgr;
	struct pp_eventmgr *eventmgr;
	int ret = 0;

	if (handle == NULL)
		return -EINVAL;

	pp_handle = (struct pp_instance *)handle;
	smumgr = pp_handle->smu_mgr;

	if (smumgr == NULL || smumgr->smumgr_funcs == NULL ||
		smumgr->smumgr_funcs->smu_init == NULL ||
		smumgr->smumgr_funcs->start_smu == NULL)
		return -EINVAL;

	ret = smumgr->smumgr_funcs->smu_init(smumgr);
	if (ret) {
		printk(KERN_ERR "[ powerplay ] smc initialization failed\n");
		return ret;
	}

	ret = smumgr->smumgr_funcs->start_smu(smumgr);
	if (ret) {
		printk(KERN_ERR "[ powerplay ] smc start failed\n");
		smumgr->smumgr_funcs->smu_fini(smumgr);
		return ret;
	}

	hw_init_power_state_table(pp_handle->hwmgr);
	eventmgr = pp_handle->eventmgr;

	if (eventmgr == NULL || eventmgr->pp_eventmgr_init == NULL)
		return -EINVAL;

	ret = eventmgr->pp_eventmgr_init(eventmgr);
	return 0;
=======

#define PP_DPM_DISABLED 0xCCCC

static int pp_dpm_dispatch_tasks(void *handle, enum amd_pp_task task_id,
		void *input, void *output);

static inline int pp_check(struct pp_instance *handle)
{
	if (handle == NULL)
		return -EINVAL;

	if (handle->hwmgr == NULL || handle->hwmgr->smumgr_funcs == NULL)
		return -EINVAL;

	if (handle->pm_en == 0)
		return PP_DPM_DISABLED;

	if (handle->hwmgr->hwmgr_func == NULL)
		return PP_DPM_DISABLED;

	return 0;
}

static int amd_powerplay_create(struct amd_pp_init *pp_init,
				void **handle)
{
	struct pp_instance *instance;

	if (pp_init == NULL || handle == NULL)
		return -EINVAL;

	instance = kzalloc(sizeof(struct pp_instance), GFP_KERNEL);
	if (instance == NULL)
		return -ENOMEM;

	instance->chip_family = pp_init->chip_family;
	instance->chip_id = pp_init->chip_id;
	instance->pm_en = pp_init->pm_en;
	instance->feature_mask = pp_init->feature_mask;
	instance->device = pp_init->device;
	mutex_init(&instance->pp_lock);
	*handle = instance;
	return 0;
}

static int amd_powerplay_destroy(void *handle)
{
	struct pp_instance *instance = (struct pp_instance *)handle;

	kfree(instance->hwmgr->hardcode_pp_table);
	instance->hwmgr->hardcode_pp_table = NULL;

	kfree(instance->hwmgr);
	instance->hwmgr = NULL;

	kfree(instance);
	instance = NULL;
	return 0;
}

static int pp_early_init(void *handle)
{
	int ret;
	struct pp_instance *pp_handle = NULL;

	pp_handle = cgs_register_pp_handle(handle, amd_powerplay_create);

	if (!pp_handle)
		return -EINVAL;

	ret = hwmgr_early_init(pp_handle);
	if (ret)
		return -EINVAL;

	return 0;
}

static int pp_sw_init(void *handle)
{
	struct pp_hwmgr *hwmgr;
	int ret = 0;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;

	ret = pp_check(pp_handle);

	if (ret >= 0) {
		hwmgr = pp_handle->hwmgr;

		if (hwmgr->smumgr_funcs->smu_init == NULL)
			return -EINVAL;

		ret = hwmgr->smumgr_funcs->smu_init(hwmgr);

		pr_info("amdgpu: powerplay sw initialized\n");
	}
	return ret;
}

static int pp_sw_fini(void *handle)
{
	struct pp_hwmgr *hwmgr;
	int ret = 0;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;

	ret = pp_check(pp_handle);
	if (ret >= 0) {
		hwmgr = pp_handle->hwmgr;

		if (hwmgr->smumgr_funcs->smu_fini == NULL)
			return -EINVAL;

		ret = hwmgr->smumgr_funcs->smu_fini(pp_handle->hwmgr);
	}
	return ret;
}

static int pp_hw_init(void *handle)
{
	int ret = 0;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	struct pp_hwmgr *hwmgr;

	ret = pp_check(pp_handle);

	if (ret >= 0) {
		hwmgr = pp_handle->hwmgr;

		if (hwmgr->smumgr_funcs->start_smu == NULL)
			return -EINVAL;

		if(hwmgr->smumgr_funcs->start_smu(pp_handle->hwmgr)) {
			pr_err("smc start failed\n");
			hwmgr->smumgr_funcs->smu_fini(pp_handle->hwmgr);
			return -EINVAL;;
		}
		if (ret == PP_DPM_DISABLED)
			goto exit;
		ret = hwmgr_hw_init(pp_handle);
		if (ret)
			goto exit;
	}
	return ret;
exit:
	pp_handle->pm_en = 0;
	cgs_notify_dpm_enabled(hwmgr->device, false);
	return 0;

>>>>>>> temp
}

static int pp_hw_fini(void *handle)
{
<<<<<<< HEAD
	struct pp_instance *pp_handle;
	struct pp_smumgr *smumgr;
	struct pp_eventmgr *eventmgr;

	if (handle == NULL)
		return -EINVAL;

	pp_handle = (struct pp_instance *)handle;
	eventmgr = pp_handle->eventmgr;

	if (eventmgr != NULL || eventmgr->pp_eventmgr_fini != NULL)
		eventmgr->pp_eventmgr_fini(eventmgr);

	smumgr = pp_handle->smu_mgr;

	if (smumgr != NULL || smumgr->smumgr_funcs != NULL ||
		smumgr->smumgr_funcs->smu_fini != NULL)
		smumgr->smumgr_funcs->smu_fini(smumgr);
=======
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);
	if (ret == 0)
		hwmgr_hw_fini(pp_handle);

	return 0;
}

static int pp_late_init(void *handle)
{
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);
	if (ret == 0)
		pp_dpm_dispatch_tasks(pp_handle,
					AMD_PP_TASK_COMPLETE_INIT, NULL, NULL);
>>>>>>> temp

	return 0;
}

<<<<<<< HEAD
static bool pp_is_idle(void *handle)
{
	return 0;
=======
static void pp_late_fini(void *handle)
{
	amd_powerplay_destroy(handle);
}


static bool pp_is_idle(void *handle)
{
	return false;
>>>>>>> temp
}

static int pp_wait_for_idle(void *handle)
{
	return 0;
}

static int pp_sw_reset(void *handle)
{
	return 0;
}

<<<<<<< HEAD
static void pp_print_status(void *handle)
{

}

static int pp_set_clockgating_state(void *handle,
				    enum amd_clockgating_state state)
{
	return 0;
}

static int pp_set_powergating_state(void *handle,
				    enum amd_powergating_state state)
{
	return 0;
=======
static int pp_set_powergating_state(void *handle,
				    enum amd_powergating_state state)
{
	struct pp_hwmgr  *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);

	if (ret)
		return ret;

	hwmgr = pp_handle->hwmgr;

	if (hwmgr->hwmgr_func->enable_per_cu_power_gating == NULL) {
		pr_info("%s was not implemented.\n", __func__);
		return 0;
	}

	/* Enable/disable GFX per cu powergating through SMU */
	return hwmgr->hwmgr_func->enable_per_cu_power_gating(hwmgr,
			state == AMD_PG_STATE_GATE);
>>>>>>> temp
}

static int pp_suspend(void *handle)
{
<<<<<<< HEAD
	struct pp_instance *pp_handle;
	struct pp_eventmgr *eventmgr;
	struct pem_event_data event_data = { {0} };

	if (handle == NULL)
		return -EINVAL;

	pp_handle = (struct pp_instance *)handle;
	eventmgr = pp_handle->eventmgr;
	pem_handle_event(eventmgr, AMD_PP_EVENT_SUSPEND, &event_data);
=======
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);
	if (ret == 0)
		hwmgr_hw_suspend(pp_handle);
>>>>>>> temp
	return 0;
}

static int pp_resume(void *handle)
{
<<<<<<< HEAD
	struct pp_instance *pp_handle;
	struct pp_eventmgr *eventmgr;
	struct pem_event_data event_data = { {0} };
	struct pp_smumgr *smumgr;
	int ret;

	if (handle == NULL)
		return -EINVAL;

	pp_handle = (struct pp_instance *)handle;
	smumgr = pp_handle->smu_mgr;

	if (smumgr == NULL || smumgr->smumgr_funcs == NULL ||
		smumgr->smumgr_funcs->start_smu == NULL)
		return -EINVAL;

	ret = smumgr->smumgr_funcs->start_smu(smumgr);
	if (ret) {
		printk(KERN_ERR "[ powerplay ] smc start failed\n");
		smumgr->smumgr_funcs->smu_fini(smumgr);
		return ret;
	}

	eventmgr = pp_handle->eventmgr;
	pem_handle_event(eventmgr, AMD_PP_EVENT_RESUME, &event_data);

	return 0;
}

const struct amd_ip_funcs pp_ip_funcs = {
	.early_init = pp_early_init,
	.late_init = NULL,
=======
	struct pp_hwmgr  *hwmgr;
	int ret;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;

	ret = pp_check(pp_handle);

	if (ret < 0)
		return ret;

	hwmgr = pp_handle->hwmgr;

	if (hwmgr->smumgr_funcs->start_smu == NULL)
		return -EINVAL;

	if (hwmgr->smumgr_funcs->start_smu(pp_handle->hwmgr)) {
		pr_err("smc start failed\n");
		hwmgr->smumgr_funcs->smu_fini(pp_handle->hwmgr);
		return -EINVAL;
	}

	if (ret == PP_DPM_DISABLED)
		return 0;

	return hwmgr_hw_resume(pp_handle);
}

const struct amd_ip_funcs pp_ip_funcs = {
	.name = "powerplay",
	.early_init = pp_early_init,
	.late_init = pp_late_init,
>>>>>>> temp
	.sw_init = pp_sw_init,
	.sw_fini = pp_sw_fini,
	.hw_init = pp_hw_init,
	.hw_fini = pp_hw_fini,
<<<<<<< HEAD
=======
	.late_fini = pp_late_fini,
>>>>>>> temp
	.suspend = pp_suspend,
	.resume = pp_resume,
	.is_idle = pp_is_idle,
	.wait_for_idle = pp_wait_for_idle,
	.soft_reset = pp_sw_reset,
<<<<<<< HEAD
	.print_status = pp_print_status,
	.set_clockgating_state = pp_set_clockgating_state,
=======
	.set_clockgating_state = NULL,
>>>>>>> temp
	.set_powergating_state = pp_set_powergating_state,
};

static int pp_dpm_load_fw(void *handle)
{
	return 0;
}

static int pp_dpm_fw_loading_complete(void *handle)
{
	return 0;
}

<<<<<<< HEAD
static int pp_dpm_force_performance_level(void *handle,
					enum amd_dpm_forced_level level)
{
	struct pp_instance *pp_handle;
	struct pp_hwmgr  *hwmgr;

	if (handle == NULL)
		return -EINVAL;

	pp_handle = (struct pp_instance *)handle;

	hwmgr = pp_handle->hwmgr;

	if (hwmgr == NULL || hwmgr->hwmgr_func == NULL ||
	    hwmgr->hwmgr_func->force_dpm_level == NULL)
		return -EINVAL;

	hwmgr->hwmgr_func->force_dpm_level(hwmgr, level);

=======
static int pp_set_clockgating_by_smu(void *handle, uint32_t msg_id)
{
	struct pp_hwmgr  *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);

	if (ret)
		return ret;

	hwmgr = pp_handle->hwmgr;

	if (hwmgr->hwmgr_func->update_clock_gatings == NULL) {
		pr_info("%s was not implemented.\n", __func__);
		return 0;
	}

	return hwmgr->hwmgr_func->update_clock_gatings(hwmgr, &msg_id);
}

static void pp_dpm_en_umd_pstate(struct pp_hwmgr  *hwmgr,
						enum amd_dpm_forced_level *level)
{
	uint32_t profile_mode_mask = AMD_DPM_FORCED_LEVEL_PROFILE_STANDARD |
					AMD_DPM_FORCED_LEVEL_PROFILE_MIN_SCLK |
					AMD_DPM_FORCED_LEVEL_PROFILE_MIN_MCLK |
					AMD_DPM_FORCED_LEVEL_PROFILE_PEAK;

	if (!(hwmgr->dpm_level & profile_mode_mask)) {
		/* enter umd pstate, save current level, disable gfx cg*/
		if (*level & profile_mode_mask) {
			hwmgr->saved_dpm_level = hwmgr->dpm_level;
			hwmgr->en_umd_pstate = true;
			cgs_set_clockgating_state(hwmgr->device,
						AMD_IP_BLOCK_TYPE_GFX,
						AMD_CG_STATE_UNGATE);
			cgs_set_powergating_state(hwmgr->device,
					AMD_IP_BLOCK_TYPE_GFX,
					AMD_PG_STATE_UNGATE);
		}
	} else {
		/* exit umd pstate, restore level, enable gfx cg*/
		if (!(*level & profile_mode_mask)) {
			if (*level == AMD_DPM_FORCED_LEVEL_PROFILE_EXIT)
				*level = hwmgr->saved_dpm_level;
			hwmgr->en_umd_pstate = false;
			cgs_set_clockgating_state(hwmgr->device,
					AMD_IP_BLOCK_TYPE_GFX,
					AMD_CG_STATE_GATE);
			cgs_set_powergating_state(hwmgr->device,
					AMD_IP_BLOCK_TYPE_GFX,
					AMD_PG_STATE_GATE);
		}
	}
}

static int pp_dpm_force_performance_level(void *handle,
					enum amd_dpm_forced_level level)
{
	struct pp_hwmgr  *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);

	if (ret)
		return ret;

	hwmgr = pp_handle->hwmgr;

	if (level == hwmgr->dpm_level)
		return 0;

	if (hwmgr->hwmgr_func->force_dpm_level == NULL) {
		pr_info("%s was not implemented.\n", __func__);
		return 0;
	}

	mutex_lock(&pp_handle->pp_lock);
	pp_dpm_en_umd_pstate(hwmgr, &level);
	hwmgr->request_dpm_level = level;
	hwmgr_handle_task(pp_handle, AMD_PP_TASK_READJUST_POWER_STATE, NULL, NULL);
	ret = hwmgr->hwmgr_func->force_dpm_level(hwmgr, level);
	if (!ret)
		hwmgr->dpm_level = hwmgr->request_dpm_level;

	mutex_unlock(&pp_handle->pp_lock);
>>>>>>> temp
	return 0;
}

static enum amd_dpm_forced_level pp_dpm_get_performance_level(
								void *handle)
{
	struct pp_hwmgr  *hwmgr;
<<<<<<< HEAD

	if (handle == NULL)
		return -EINVAL;

	hwmgr = ((struct pp_instance *)handle)->hwmgr;

	if (hwmgr == NULL)
		return -EINVAL;

	return (((struct pp_instance *)handle)->hwmgr->dpm_level);
}

static int pp_dpm_get_sclk(void *handle, bool low)
{
	struct pp_hwmgr  *hwmgr;

	if (handle == NULL)
		return -EINVAL;

	hwmgr = ((struct pp_instance *)handle)->hwmgr;

	if (hwmgr == NULL || hwmgr->hwmgr_func == NULL ||
	    hwmgr->hwmgr_func->get_sclk == NULL)
		return -EINVAL;

	return hwmgr->hwmgr_func->get_sclk(hwmgr, low);
}

static int pp_dpm_get_mclk(void *handle, bool low)
{
	struct pp_hwmgr  *hwmgr;

	if (handle == NULL)
		return -EINVAL;

	hwmgr = ((struct pp_instance *)handle)->hwmgr;

	if (hwmgr == NULL || hwmgr->hwmgr_func == NULL ||
	    hwmgr->hwmgr_func->get_mclk == NULL)
		return -EINVAL;

	return hwmgr->hwmgr_func->get_mclk(hwmgr, low);
}

static int pp_dpm_powergate_vce(void *handle, bool gate)
{
	struct pp_hwmgr  *hwmgr;

	if (handle == NULL)
		return -EINVAL;

	hwmgr = ((struct pp_instance *)handle)->hwmgr;

	if (hwmgr == NULL || hwmgr->hwmgr_func == NULL ||
	    hwmgr->hwmgr_func->powergate_vce == NULL)
		return -EINVAL;

	return hwmgr->hwmgr_func->powergate_vce(hwmgr, gate);
}

static int pp_dpm_powergate_uvd(void *handle, bool gate)
{
	struct pp_hwmgr  *hwmgr;

	if (handle == NULL)
		return -EINVAL;

	hwmgr = ((struct pp_instance *)handle)->hwmgr;

	if (hwmgr == NULL || hwmgr->hwmgr_func == NULL ||
	    hwmgr->hwmgr_func->powergate_uvd == NULL)
		return -EINVAL;

	return hwmgr->hwmgr_func->powergate_uvd(hwmgr, gate);
}

static enum PP_StateUILabel power_state_convert(enum amd_pm_state_type  state)
{
	switch (state) {
	case POWER_STATE_TYPE_BATTERY:
		return PP_StateUILabel_Battery;
	case POWER_STATE_TYPE_BALANCED:
		return PP_StateUILabel_Balanced;
	case POWER_STATE_TYPE_PERFORMANCE:
		return PP_StateUILabel_Performance;
	default:
		return PP_StateUILabel_None;
	}
}

int pp_dpm_dispatch_tasks(void *handle, enum amd_pp_event event_id, void *input, void *output)
{
	int ret = 0;
	struct pp_instance *pp_handle;
	struct pem_event_data data = { {0} };

	pp_handle = (struct pp_instance *)handle;

	if (pp_handle == NULL)
		return -EINVAL;

	switch (event_id) {
	case AMD_PP_EVENT_DISPLAY_CONFIG_CHANGE:
		ret = pem_handle_event(pp_handle->eventmgr, event_id, &data);
		break;
	case AMD_PP_EVENT_ENABLE_USER_STATE:
	{
		enum amd_pm_state_type  ps;

		if (input == NULL)
			return -EINVAL;
		ps = *(unsigned long *)input;

		data.requested_ui_label = power_state_convert(ps);
		ret = pem_handle_event(pp_handle->eventmgr, event_id, &data);
	}
	break;
	default:
		break;
	}
	return ret;
}

enum amd_pm_state_type pp_dpm_get_current_power_state(void *handle)
{
	struct pp_hwmgr *hwmgr;
	struct pp_power_state *state;

	if (handle == NULL)
		return -EINVAL;

	hwmgr = ((struct pp_instance *)handle)->hwmgr;

	if (hwmgr == NULL || hwmgr->current_ps == NULL)
		return -EINVAL;

=======
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;
	enum amd_dpm_forced_level level;

	ret = pp_check(pp_handle);

	if (ret)
		return ret;

	hwmgr = pp_handle->hwmgr;
	mutex_lock(&pp_handle->pp_lock);
	level = hwmgr->dpm_level;
	mutex_unlock(&pp_handle->pp_lock);
	return level;
}

static uint32_t pp_dpm_get_sclk(void *handle, bool low)
{
	struct pp_hwmgr  *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;
	uint32_t clk = 0;

	ret = pp_check(pp_handle);

	if (ret)
		return ret;

	hwmgr = pp_handle->hwmgr;

	if (hwmgr->hwmgr_func->get_sclk == NULL) {
		pr_info("%s was not implemented.\n", __func__);
		return 0;
	}
	mutex_lock(&pp_handle->pp_lock);
	clk = hwmgr->hwmgr_func->get_sclk(hwmgr, low);
	mutex_unlock(&pp_handle->pp_lock);
	return clk;
}

static uint32_t pp_dpm_get_mclk(void *handle, bool low)
{
	struct pp_hwmgr  *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;
	uint32_t clk = 0;

	ret = pp_check(pp_handle);

	if (ret)
		return ret;

	hwmgr = pp_handle->hwmgr;

	if (hwmgr->hwmgr_func->get_mclk == NULL) {
		pr_info("%s was not implemented.\n", __func__);
		return 0;
	}
	mutex_lock(&pp_handle->pp_lock);
	clk = hwmgr->hwmgr_func->get_mclk(hwmgr, low);
	mutex_unlock(&pp_handle->pp_lock);
	return clk;
}

static void pp_dpm_powergate_vce(void *handle, bool gate)
{
	struct pp_hwmgr  *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);

	if (ret)
		return;

	hwmgr = pp_handle->hwmgr;

	if (hwmgr->hwmgr_func->powergate_vce == NULL) {
		pr_info("%s was not implemented.\n", __func__);
		return;
	}
	mutex_lock(&pp_handle->pp_lock);
	hwmgr->hwmgr_func->powergate_vce(hwmgr, gate);
	mutex_unlock(&pp_handle->pp_lock);
}

static void pp_dpm_powergate_uvd(void *handle, bool gate)
{
	struct pp_hwmgr  *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);

	if (ret)
		return;

	hwmgr = pp_handle->hwmgr;

	if (hwmgr->hwmgr_func->powergate_uvd == NULL) {
		pr_info("%s was not implemented.\n", __func__);
		return;
	}
	mutex_lock(&pp_handle->pp_lock);
	hwmgr->hwmgr_func->powergate_uvd(hwmgr, gate);
	mutex_unlock(&pp_handle->pp_lock);
}

static int pp_dpm_dispatch_tasks(void *handle, enum amd_pp_task task_id,
		void *input, void *output)
{
	int ret = 0;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;

	ret = pp_check(pp_handle);

	if (ret)
		return ret;

	mutex_lock(&pp_handle->pp_lock);
	ret = hwmgr_handle_task(pp_handle, task_id, input, output);
	mutex_unlock(&pp_handle->pp_lock);

	return ret;
}

static enum amd_pm_state_type pp_dpm_get_current_power_state(void *handle)
{
	struct pp_hwmgr *hwmgr;
	struct pp_power_state *state;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;
	enum amd_pm_state_type pm_type;

	ret = pp_check(pp_handle);

	if (ret)
		return ret;

	hwmgr = pp_handle->hwmgr;

	if (hwmgr->current_ps == NULL)
		return -EINVAL;

	mutex_lock(&pp_handle->pp_lock);

>>>>>>> temp
	state = hwmgr->current_ps;

	switch (state->classification.ui_label) {
	case PP_StateUILabel_Battery:
<<<<<<< HEAD
		return POWER_STATE_TYPE_BATTERY;
	case PP_StateUILabel_Balanced:
		return POWER_STATE_TYPE_BALANCED;
	case PP_StateUILabel_Performance:
		return POWER_STATE_TYPE_PERFORMANCE;
	default:
		return POWER_STATE_TYPE_DEFAULT;
	}
}

static void
pp_debugfs_print_current_performance_level(void *handle,
					       struct seq_file *m)
{
	struct pp_hwmgr  *hwmgr;

	if (handle == NULL)
		return;

	hwmgr = ((struct pp_instance *)handle)->hwmgr;

	if (hwmgr == NULL || hwmgr->hwmgr_func == NULL ||
	  hwmgr->hwmgr_func->print_current_perforce_level == NULL)
		return;

	hwmgr->hwmgr_func->print_current_perforce_level(hwmgr, m);
}

static int pp_dpm_set_fan_control_mode(void *handle, uint32_t mode)
{
	struct pp_hwmgr  *hwmgr;

	if (handle == NULL)
		return -EINVAL;

	hwmgr = ((struct pp_instance *)handle)->hwmgr;

	if (hwmgr == NULL || hwmgr->hwmgr_func == NULL ||
	  hwmgr->hwmgr_func->set_fan_control_mode == NULL)
		return -EINVAL;

	return hwmgr->hwmgr_func->set_fan_control_mode(hwmgr, mode);
}

static int pp_dpm_get_fan_control_mode(void *handle)
{
	struct pp_hwmgr  *hwmgr;

	if (handle == NULL)
		return -EINVAL;

	hwmgr = ((struct pp_instance *)handle)->hwmgr;

	if (hwmgr == NULL || hwmgr->hwmgr_func == NULL ||
	  hwmgr->hwmgr_func->get_fan_control_mode == NULL)
		return -EINVAL;

	return hwmgr->hwmgr_func->get_fan_control_mode(hwmgr);
}

static int pp_dpm_set_fan_speed_percent(void *handle, uint32_t percent)
{
	struct pp_hwmgr  *hwmgr;

	if (handle == NULL)
		return -EINVAL;

	hwmgr = ((struct pp_instance *)handle)->hwmgr;

	if (hwmgr == NULL || hwmgr->hwmgr_func == NULL ||
	  hwmgr->hwmgr_func->set_fan_speed_percent == NULL)
		return -EINVAL;

	return hwmgr->hwmgr_func->set_fan_speed_percent(hwmgr, percent);
}

static int pp_dpm_get_fan_speed_percent(void *handle, uint32_t *speed)
{
	struct pp_hwmgr  *hwmgr;

	if (handle == NULL)
		return -EINVAL;

	hwmgr = ((struct pp_instance *)handle)->hwmgr;

	if (hwmgr == NULL || hwmgr->hwmgr_func == NULL ||
	  hwmgr->hwmgr_func->get_fan_speed_percent == NULL)
		return -EINVAL;

	return hwmgr->hwmgr_func->get_fan_speed_percent(hwmgr, speed);
}

static int pp_dpm_get_temperature(void *handle)
{
	struct pp_hwmgr  *hwmgr;

	if (handle == NULL)
		return -EINVAL;

	hwmgr = ((struct pp_instance *)handle)->hwmgr;

	if (hwmgr == NULL || hwmgr->hwmgr_func == NULL ||
	  hwmgr->hwmgr_func->get_temperature == NULL)
		return -EINVAL;

	return hwmgr->hwmgr_func->get_temperature(hwmgr);
}

const struct amd_powerplay_funcs pp_dpm_funcs = {
=======
		pm_type = POWER_STATE_TYPE_BATTERY;
		break;
	case PP_StateUILabel_Balanced:
		pm_type = POWER_STATE_TYPE_BALANCED;
		break;
	case PP_StateUILabel_Performance:
		pm_type = POWER_STATE_TYPE_PERFORMANCE;
		break;
	default:
		if (state->classification.flags & PP_StateClassificationFlag_Boot)
			pm_type = POWER_STATE_TYPE_INTERNAL_BOOT;
		else
			pm_type = POWER_STATE_TYPE_DEFAULT;
		break;
	}
	mutex_unlock(&pp_handle->pp_lock);

	return pm_type;
}

static void pp_dpm_set_fan_control_mode(void *handle, uint32_t mode)
{
	struct pp_hwmgr  *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);

	if (ret)
		return;

	hwmgr = pp_handle->hwmgr;

	if (hwmgr->hwmgr_func->set_fan_control_mode == NULL) {
		pr_info("%s was not implemented.\n", __func__);
		return;
	}
	mutex_lock(&pp_handle->pp_lock);
	hwmgr->hwmgr_func->set_fan_control_mode(hwmgr, mode);
	mutex_unlock(&pp_handle->pp_lock);
}

static uint32_t pp_dpm_get_fan_control_mode(void *handle)
{
	struct pp_hwmgr  *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;
	uint32_t mode = 0;

	ret = pp_check(pp_handle);

	if (ret)
		return ret;

	hwmgr = pp_handle->hwmgr;

	if (hwmgr->hwmgr_func->get_fan_control_mode == NULL) {
		pr_info("%s was not implemented.\n", __func__);
		return 0;
	}
	mutex_lock(&pp_handle->pp_lock);
	mode = hwmgr->hwmgr_func->get_fan_control_mode(hwmgr);
	mutex_unlock(&pp_handle->pp_lock);
	return mode;
}

static int pp_dpm_set_fan_speed_percent(void *handle, uint32_t percent)
{
	struct pp_hwmgr  *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);

	if (ret)
		return ret;

	hwmgr = pp_handle->hwmgr;

	if (hwmgr->hwmgr_func->set_fan_speed_percent == NULL) {
		pr_info("%s was not implemented.\n", __func__);
		return 0;
	}
	mutex_lock(&pp_handle->pp_lock);
	ret = hwmgr->hwmgr_func->set_fan_speed_percent(hwmgr, percent);
	mutex_unlock(&pp_handle->pp_lock);
	return ret;
}

static int pp_dpm_get_fan_speed_percent(void *handle, uint32_t *speed)
{
	struct pp_hwmgr  *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);

	if (ret)
		return ret;

	hwmgr = pp_handle->hwmgr;

	if (hwmgr->hwmgr_func->get_fan_speed_percent == NULL) {
		pr_info("%s was not implemented.\n", __func__);
		return 0;
	}

	mutex_lock(&pp_handle->pp_lock);
	ret = hwmgr->hwmgr_func->get_fan_speed_percent(hwmgr, speed);
	mutex_unlock(&pp_handle->pp_lock);
	return ret;
}

static int pp_dpm_get_fan_speed_rpm(void *handle, uint32_t *rpm)
{
	struct pp_hwmgr *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);

	if (ret)
		return ret;

	hwmgr = pp_handle->hwmgr;

	if (hwmgr->hwmgr_func->get_fan_speed_rpm == NULL)
		return -EINVAL;

	mutex_lock(&pp_handle->pp_lock);
	ret = hwmgr->hwmgr_func->get_fan_speed_rpm(hwmgr, rpm);
	mutex_unlock(&pp_handle->pp_lock);
	return ret;
}

static int pp_dpm_get_temperature(void *handle)
{
	struct pp_hwmgr  *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);

	if (ret)
		return ret;

	hwmgr = pp_handle->hwmgr;

	if (hwmgr->hwmgr_func->get_temperature == NULL) {
		pr_info("%s was not implemented.\n", __func__);
		return 0;
	}
	mutex_lock(&pp_handle->pp_lock);
	ret = hwmgr->hwmgr_func->get_temperature(hwmgr);
	mutex_unlock(&pp_handle->pp_lock);
	return ret;
}

static int pp_dpm_get_pp_num_states(void *handle,
		struct pp_states_info *data)
{
	struct pp_hwmgr *hwmgr;
	int i;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);

	if (ret)
		return ret;

	hwmgr = pp_handle->hwmgr;

	if (hwmgr->ps == NULL)
		return -EINVAL;

	mutex_lock(&pp_handle->pp_lock);

	data->nums = hwmgr->num_ps;

	for (i = 0; i < hwmgr->num_ps; i++) {
		struct pp_power_state *state = (struct pp_power_state *)
				((unsigned long)hwmgr->ps + i * hwmgr->ps_size);
		switch (state->classification.ui_label) {
		case PP_StateUILabel_Battery:
			data->states[i] = POWER_STATE_TYPE_BATTERY;
			break;
		case PP_StateUILabel_Balanced:
			data->states[i] = POWER_STATE_TYPE_BALANCED;
			break;
		case PP_StateUILabel_Performance:
			data->states[i] = POWER_STATE_TYPE_PERFORMANCE;
			break;
		default:
			if (state->classification.flags & PP_StateClassificationFlag_Boot)
				data->states[i] = POWER_STATE_TYPE_INTERNAL_BOOT;
			else
				data->states[i] = POWER_STATE_TYPE_DEFAULT;
		}
	}
	mutex_unlock(&pp_handle->pp_lock);
	return 0;
}

static int pp_dpm_get_pp_table(void *handle, char **table)
{
	struct pp_hwmgr *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;
	int size = 0;

	ret = pp_check(pp_handle);

	if (ret)
		return ret;

	hwmgr = pp_handle->hwmgr;

	if (!hwmgr->soft_pp_table)
		return -EINVAL;

	mutex_lock(&pp_handle->pp_lock);
	*table = (char *)hwmgr->soft_pp_table;
	size = hwmgr->soft_pp_table_size;
	mutex_unlock(&pp_handle->pp_lock);
	return size;
}

static int pp_dpm_set_pp_table(void *handle, const char *buf, size_t size)
{
	struct pp_hwmgr *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);

	if (ret)
		return ret;

	hwmgr = pp_handle->hwmgr;
	mutex_lock(&pp_handle->pp_lock);
	if (!hwmgr->hardcode_pp_table) {
		hwmgr->hardcode_pp_table = kmemdup(hwmgr->soft_pp_table,
						   hwmgr->soft_pp_table_size,
						   GFP_KERNEL);
		if (!hwmgr->hardcode_pp_table) {
			mutex_unlock(&pp_handle->pp_lock);
			return -ENOMEM;
		}
	}

	memcpy(hwmgr->hardcode_pp_table, buf, size);

	hwmgr->soft_pp_table = hwmgr->hardcode_pp_table;
	mutex_unlock(&pp_handle->pp_lock);

	ret = amd_powerplay_reset(handle);
	if (ret)
		return ret;

	if (hwmgr->hwmgr_func->avfs_control) {
		ret = hwmgr->hwmgr_func->avfs_control(hwmgr, false);
		if (ret)
			return ret;
	}

	return 0;
}

static int pp_dpm_force_clock_level(void *handle,
		enum pp_clock_type type, uint32_t mask)
{
	struct pp_hwmgr *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);

	if (ret)
		return ret;

	hwmgr = pp_handle->hwmgr;

	if (hwmgr->hwmgr_func->force_clock_level == NULL) {
		pr_info("%s was not implemented.\n", __func__);
		return 0;
	}
	mutex_lock(&pp_handle->pp_lock);
	hwmgr->hwmgr_func->force_clock_level(hwmgr, type, mask);
	mutex_unlock(&pp_handle->pp_lock);
	return ret;
}

static int pp_dpm_print_clock_levels(void *handle,
		enum pp_clock_type type, char *buf)
{
	struct pp_hwmgr *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);

	if (ret)
		return ret;

	hwmgr = pp_handle->hwmgr;

	if (hwmgr->hwmgr_func->print_clock_levels == NULL) {
		pr_info("%s was not implemented.\n", __func__);
		return 0;
	}
	mutex_lock(&pp_handle->pp_lock);
	ret = hwmgr->hwmgr_func->print_clock_levels(hwmgr, type, buf);
	mutex_unlock(&pp_handle->pp_lock);
	return ret;
}

static int pp_dpm_get_sclk_od(void *handle)
{
	struct pp_hwmgr *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);

	if (ret)
		return ret;

	hwmgr = pp_handle->hwmgr;

	if (hwmgr->hwmgr_func->get_sclk_od == NULL) {
		pr_info("%s was not implemented.\n", __func__);
		return 0;
	}
	mutex_lock(&pp_handle->pp_lock);
	ret = hwmgr->hwmgr_func->get_sclk_od(hwmgr);
	mutex_unlock(&pp_handle->pp_lock);
	return ret;
}

static int pp_dpm_set_sclk_od(void *handle, uint32_t value)
{
	struct pp_hwmgr *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);

	if (ret)
		return ret;

	hwmgr = pp_handle->hwmgr;

	if (hwmgr->hwmgr_func->set_sclk_od == NULL) {
		pr_info("%s was not implemented.\n", __func__);
		return 0;
	}

	mutex_lock(&pp_handle->pp_lock);
	ret = hwmgr->hwmgr_func->set_sclk_od(hwmgr, value);
	mutex_unlock(&pp_handle->pp_lock);
	return ret;
}

static int pp_dpm_get_mclk_od(void *handle)
{
	struct pp_hwmgr *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);

	if (ret)
		return ret;

	hwmgr = pp_handle->hwmgr;

	if (hwmgr->hwmgr_func->get_mclk_od == NULL) {
		pr_info("%s was not implemented.\n", __func__);
		return 0;
	}
	mutex_lock(&pp_handle->pp_lock);
	ret = hwmgr->hwmgr_func->get_mclk_od(hwmgr);
	mutex_unlock(&pp_handle->pp_lock);
	return ret;
}

static int pp_dpm_set_mclk_od(void *handle, uint32_t value)
{
	struct pp_hwmgr *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);

	if (ret)
		return ret;

	hwmgr = pp_handle->hwmgr;

	if (hwmgr->hwmgr_func->set_mclk_od == NULL) {
		pr_info("%s was not implemented.\n", __func__);
		return 0;
	}
	mutex_lock(&pp_handle->pp_lock);
	ret = hwmgr->hwmgr_func->set_mclk_od(hwmgr, value);
	mutex_unlock(&pp_handle->pp_lock);
	return ret;
}

static int pp_dpm_read_sensor(void *handle, int idx,
			      void *value, int *size)
{
	struct pp_hwmgr *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);

	if (ret)
		return ret;

	hwmgr = pp_handle->hwmgr;

	if (hwmgr->hwmgr_func->read_sensor == NULL) {
		pr_info("%s was not implemented.\n", __func__);
		return 0;
	}

	mutex_lock(&pp_handle->pp_lock);
	ret = hwmgr->hwmgr_func->read_sensor(hwmgr, idx, value, size);
	mutex_unlock(&pp_handle->pp_lock);

	return ret;
}

static struct amd_vce_state*
pp_dpm_get_vce_clock_state(void *handle, unsigned idx)
{
	struct pp_hwmgr *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);

	if (ret)
		return NULL;

	hwmgr = pp_handle->hwmgr;

	if (hwmgr && idx < hwmgr->num_vce_state_tables)
		return &hwmgr->vce_states[idx];
	return NULL;
}

static int pp_dpm_reset_power_profile_state(void *handle,
		struct amd_pp_profile *request)
{
	struct pp_hwmgr *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;

	if (!request || pp_check(pp_handle))
		return -EINVAL;

	hwmgr = pp_handle->hwmgr;

	if (hwmgr->hwmgr_func->set_power_profile_state == NULL) {
		pr_info("%s was not implemented.\n", __func__);
		return 0;
	}

	if (request->type == AMD_PP_GFX_PROFILE) {
		hwmgr->gfx_power_profile = hwmgr->default_gfx_power_profile;
		return hwmgr->hwmgr_func->set_power_profile_state(hwmgr,
				&hwmgr->gfx_power_profile);
	} else if (request->type == AMD_PP_COMPUTE_PROFILE) {
		hwmgr->compute_power_profile =
				hwmgr->default_compute_power_profile;
		return hwmgr->hwmgr_func->set_power_profile_state(hwmgr,
				&hwmgr->compute_power_profile);
	} else
		return -EINVAL;
}

static int pp_dpm_get_power_profile_state(void *handle,
		struct amd_pp_profile *query)
{
	struct pp_hwmgr *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;

	if (!query || pp_check(pp_handle))
		return -EINVAL;

	hwmgr = pp_handle->hwmgr;

	if (query->type == AMD_PP_GFX_PROFILE)
		memcpy(query, &hwmgr->gfx_power_profile,
				sizeof(struct amd_pp_profile));
	else if (query->type == AMD_PP_COMPUTE_PROFILE)
		memcpy(query, &hwmgr->compute_power_profile,
				sizeof(struct amd_pp_profile));
	else
		return -EINVAL;

	return 0;
}

static int pp_dpm_set_power_profile_state(void *handle,
		struct amd_pp_profile *request)
{
	struct pp_hwmgr *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = -1;

	if (!request || pp_check(pp_handle))
		return -EINVAL;

	hwmgr = pp_handle->hwmgr;

	if (hwmgr->hwmgr_func->set_power_profile_state == NULL) {
		pr_info("%s was not implemented.\n", __func__);
		return 0;
	}

	if (request->min_sclk ||
		request->min_mclk ||
		request->activity_threshold ||
		request->up_hyst ||
		request->down_hyst) {
		if (request->type == AMD_PP_GFX_PROFILE)
			memcpy(&hwmgr->gfx_power_profile, request,
					sizeof(struct amd_pp_profile));
		else if (request->type == AMD_PP_COMPUTE_PROFILE)
			memcpy(&hwmgr->compute_power_profile, request,
					sizeof(struct amd_pp_profile));
		else
			return -EINVAL;

		if (request->type == hwmgr->current_power_profile)
			ret = hwmgr->hwmgr_func->set_power_profile_state(
					hwmgr,
					request);
	} else {
		/* set power profile if it exists */
		switch (request->type) {
		case AMD_PP_GFX_PROFILE:
			ret = hwmgr->hwmgr_func->set_power_profile_state(
					hwmgr,
					&hwmgr->gfx_power_profile);
			break;
		case AMD_PP_COMPUTE_PROFILE:
			ret = hwmgr->hwmgr_func->set_power_profile_state(
					hwmgr,
					&hwmgr->compute_power_profile);
			break;
		default:
			return -EINVAL;
		}
	}

	if (!ret)
		hwmgr->current_power_profile = request->type;

	return 0;
}

static int pp_dpm_switch_power_profile(void *handle,
		enum amd_pp_profile_type type)
{
	struct pp_hwmgr *hwmgr;
	struct amd_pp_profile request = {0};
	struct pp_instance *pp_handle = (struct pp_instance *)handle;

	if (pp_check(pp_handle))
		return -EINVAL;

	hwmgr = pp_handle->hwmgr;

	if (hwmgr->current_power_profile != type) {
		request.type = type;
		pp_dpm_set_power_profile_state(handle, &request);
	}

	return 0;
}

const struct amd_pm_funcs pp_dpm_funcs = {
>>>>>>> temp
	.get_temperature = pp_dpm_get_temperature,
	.load_firmware = pp_dpm_load_fw,
	.wait_for_fw_loading_complete = pp_dpm_fw_loading_complete,
	.force_performance_level = pp_dpm_force_performance_level,
	.get_performance_level = pp_dpm_get_performance_level,
	.get_current_power_state = pp_dpm_get_current_power_state,
	.get_sclk = pp_dpm_get_sclk,
	.get_mclk = pp_dpm_get_mclk,
	.powergate_vce = pp_dpm_powergate_vce,
	.powergate_uvd = pp_dpm_powergate_uvd,
	.dispatch_tasks = pp_dpm_dispatch_tasks,
<<<<<<< HEAD
	.print_current_performance_level = pp_debugfs_print_current_performance_level,
=======
>>>>>>> temp
	.set_fan_control_mode = pp_dpm_set_fan_control_mode,
	.get_fan_control_mode = pp_dpm_get_fan_control_mode,
	.set_fan_speed_percent = pp_dpm_set_fan_speed_percent,
	.get_fan_speed_percent = pp_dpm_get_fan_speed_percent,
<<<<<<< HEAD
};

static int amd_pp_instance_init(struct amd_pp_init *pp_init,
				struct amd_powerplay *amd_pp)
{
	int ret;
	struct pp_instance *handle;

	handle = kzalloc(sizeof(struct pp_instance), GFP_KERNEL);
	if (handle == NULL)
		return -ENOMEM;

	handle->pp_valid = PP_VALID;

	ret = smum_init(pp_init, handle);
	if (ret)
		goto fail_smum;

	ret = hwmgr_init(pp_init, handle);
	if (ret)
		goto fail_hwmgr;

	ret = eventmgr_init(handle);
	if (ret)
		goto fail_eventmgr;

	amd_pp->pp_handle = handle;
	return 0;

fail_eventmgr:
	hwmgr_fini(handle->hwmgr);
fail_hwmgr:
	smum_fini(handle->smu_mgr);
fail_smum:
	kfree(handle);
	return ret;
}

static int amd_pp_instance_fini(void *handle)
{
	struct pp_instance *instance = (struct pp_instance *)handle;

	if (instance == NULL)
		return -EINVAL;

	eventmgr_fini(instance->eventmgr);

	hwmgr_fini(instance->hwmgr);

	smum_fini(instance->smu_mgr);

	kfree(handle);
	return 0;
}

int amd_powerplay_init(struct amd_pp_init *pp_init,
		       struct amd_powerplay *amd_pp)
{
	int ret;

	if (pp_init == NULL || amd_pp == NULL)
		return -EINVAL;

	ret = amd_pp_instance_init(pp_init, amd_pp);

	if (ret)
		return ret;

	amd_pp->ip_funcs = &pp_ip_funcs;
	amd_pp->pp_funcs = &pp_dpm_funcs;

	return 0;
}

int amd_powerplay_fini(void *handle)
{
	amd_pp_instance_fini(handle);

	return 0;
}

/* export this function to DAL */

int amd_powerplay_display_configuration_change(void *handle, const void *input)
{
	struct pp_hwmgr  *hwmgr;
	const struct amd_pp_display_configuration *display_config = input;

	PP_CHECK((struct pp_instance *)handle);

	hwmgr = ((struct pp_instance *)handle)->hwmgr;

	phm_store_dal_configuration_data(hwmgr, display_config);

	return 0;
}

int amd_powerplay_get_display_power_level(void *handle,
		struct amd_pp_dal_clock_info *output)
{
	struct pp_hwmgr  *hwmgr;

	PP_CHECK((struct pp_instance *)handle);

	if (output == NULL)
		return -EINVAL;

	hwmgr = ((struct pp_instance *)handle)->hwmgr;

	return phm_get_dal_power_level(hwmgr, output);
}
=======
	.get_fan_speed_rpm = pp_dpm_get_fan_speed_rpm,
	.get_pp_num_states = pp_dpm_get_pp_num_states,
	.get_pp_table = pp_dpm_get_pp_table,
	.set_pp_table = pp_dpm_set_pp_table,
	.force_clock_level = pp_dpm_force_clock_level,
	.print_clock_levels = pp_dpm_print_clock_levels,
	.get_sclk_od = pp_dpm_get_sclk_od,
	.set_sclk_od = pp_dpm_set_sclk_od,
	.get_mclk_od = pp_dpm_get_mclk_od,
	.set_mclk_od = pp_dpm_set_mclk_od,
	.read_sensor = pp_dpm_read_sensor,
	.get_vce_clock_state = pp_dpm_get_vce_clock_state,
	.reset_power_profile_state = pp_dpm_reset_power_profile_state,
	.get_power_profile_state = pp_dpm_get_power_profile_state,
	.set_power_profile_state = pp_dpm_set_power_profile_state,
	.switch_power_profile = pp_dpm_switch_power_profile,
	.set_clockgating_by_smu = pp_set_clockgating_by_smu,
};

int amd_powerplay_reset(void *handle)
{
	struct pp_instance *instance = (struct pp_instance *)handle;
	int ret;

	ret = pp_check(instance);
	if (ret)
		return ret;

	ret = pp_hw_fini(instance);
	if (ret)
		return ret;

	ret = hwmgr_hw_init(instance);
	if (ret)
		return ret;

	return hwmgr_handle_task(instance, AMD_PP_TASK_COMPLETE_INIT, NULL, NULL);
}

/* export this function to DAL */

int amd_powerplay_display_configuration_change(void *handle,
	const struct amd_pp_display_configuration *display_config)
{
	struct pp_hwmgr  *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);

	if (ret)
		return ret;

	hwmgr = pp_handle->hwmgr;
	mutex_lock(&pp_handle->pp_lock);
	phm_store_dal_configuration_data(hwmgr, display_config);
	mutex_unlock(&pp_handle->pp_lock);
	return 0;
}

int amd_powerplay_get_display_power_level(void *handle,
		struct amd_pp_simple_clock_info *output)
{
	struct pp_hwmgr  *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);

	if (ret)
		return ret;

	hwmgr = pp_handle->hwmgr;

	if (output == NULL)
		return -EINVAL;

	mutex_lock(&pp_handle->pp_lock);
	ret = phm_get_dal_power_level(hwmgr, output);
	mutex_unlock(&pp_handle->pp_lock);
	return ret;
}

int amd_powerplay_get_current_clocks(void *handle,
		struct amd_pp_clock_info *clocks)
{
	struct amd_pp_simple_clock_info simple_clocks;
	struct pp_clock_info hw_clocks;
	struct pp_hwmgr  *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);

	if (ret)
		return ret;

	hwmgr = pp_handle->hwmgr;

	mutex_lock(&pp_handle->pp_lock);

	phm_get_dal_power_level(hwmgr, &simple_clocks);

	if (phm_cap_enabled(hwmgr->platform_descriptor.platformCaps,
					PHM_PlatformCaps_PowerContainment))
		ret = phm_get_clock_info(hwmgr, &hwmgr->current_ps->hardware,
					&hw_clocks, PHM_PerformanceLevelDesignation_PowerContainment);
	else
		ret = phm_get_clock_info(hwmgr, &hwmgr->current_ps->hardware,
					&hw_clocks, PHM_PerformanceLevelDesignation_Activity);

	if (ret) {
		pr_info("Error in phm_get_clock_info \n");
		mutex_unlock(&pp_handle->pp_lock);
		return -EINVAL;
	}

	clocks->min_engine_clock = hw_clocks.min_eng_clk;
	clocks->max_engine_clock = hw_clocks.max_eng_clk;
	clocks->min_memory_clock = hw_clocks.min_mem_clk;
	clocks->max_memory_clock = hw_clocks.max_mem_clk;
	clocks->min_bus_bandwidth = hw_clocks.min_bus_bandwidth;
	clocks->max_bus_bandwidth = hw_clocks.max_bus_bandwidth;

	clocks->max_engine_clock_in_sr = hw_clocks.max_eng_clk;
	clocks->min_engine_clock_in_sr = hw_clocks.min_eng_clk;

	clocks->max_clocks_state = simple_clocks.level;

	if (0 == phm_get_current_shallow_sleep_clocks(hwmgr, &hwmgr->current_ps->hardware, &hw_clocks)) {
		clocks->max_engine_clock_in_sr = hw_clocks.max_eng_clk;
		clocks->min_engine_clock_in_sr = hw_clocks.min_eng_clk;
	}
	mutex_unlock(&pp_handle->pp_lock);
	return 0;
}

int amd_powerplay_get_clock_by_type(void *handle, enum amd_pp_clock_type type, struct amd_pp_clocks *clocks)
{
	struct pp_hwmgr  *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);

	if (ret)
		return ret;

	hwmgr = pp_handle->hwmgr;

	if (clocks == NULL)
		return -EINVAL;

	mutex_lock(&pp_handle->pp_lock);
	ret = phm_get_clock_by_type(hwmgr, type, clocks);
	mutex_unlock(&pp_handle->pp_lock);
	return ret;
}

int amd_powerplay_get_clock_by_type_with_latency(void *handle,
		enum amd_pp_clock_type type,
		struct pp_clock_levels_with_latency *clocks)
{
	struct pp_hwmgr *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);
	if (ret)
		return ret;

	if (!clocks)
		return -EINVAL;

	mutex_lock(&pp_handle->pp_lock);
	hwmgr = ((struct pp_instance *)handle)->hwmgr;
	ret = phm_get_clock_by_type_with_latency(hwmgr, type, clocks);
	mutex_unlock(&pp_handle->pp_lock);
	return ret;
}

int amd_powerplay_get_clock_by_type_with_voltage(void *handle,
		enum amd_pp_clock_type type,
		struct pp_clock_levels_with_voltage *clocks)
{
	struct pp_hwmgr *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);
	if (ret)
		return ret;

	if (!clocks)
		return -EINVAL;

	hwmgr = ((struct pp_instance *)handle)->hwmgr;

	mutex_lock(&pp_handle->pp_lock);

	ret = phm_get_clock_by_type_with_voltage(hwmgr, type, clocks);

	mutex_unlock(&pp_handle->pp_lock);
	return ret;
}

int amd_powerplay_set_watermarks_for_clocks_ranges(void *handle,
		struct pp_wm_sets_with_clock_ranges_soc15 *wm_with_clock_ranges)
{
	struct pp_hwmgr *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);
	if (ret)
		return ret;

	if (!wm_with_clock_ranges)
		return -EINVAL;

	hwmgr = ((struct pp_instance *)handle)->hwmgr;

	mutex_lock(&pp_handle->pp_lock);
	ret = phm_set_watermarks_for_clocks_ranges(hwmgr,
			wm_with_clock_ranges);
	mutex_unlock(&pp_handle->pp_lock);

	return ret;
}

int amd_powerplay_display_clock_voltage_request(void *handle,
		struct pp_display_clock_request *clock)
{
	struct pp_hwmgr *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);
	if (ret)
		return ret;

	if (!clock)
		return -EINVAL;

	hwmgr = ((struct pp_instance *)handle)->hwmgr;

	mutex_lock(&pp_handle->pp_lock);
	ret = phm_display_clock_voltage_request(hwmgr, clock);
	mutex_unlock(&pp_handle->pp_lock);

	return ret;
}

int amd_powerplay_get_display_mode_validation_clocks(void *handle,
		struct amd_pp_simple_clock_info *clocks)
{
	struct pp_hwmgr  *hwmgr;
	struct pp_instance *pp_handle = (struct pp_instance *)handle;
	int ret = 0;

	ret = pp_check(pp_handle);

	if (ret)
		return ret;

	hwmgr = pp_handle->hwmgr;

	if (clocks == NULL)
		return -EINVAL;

	mutex_lock(&pp_handle->pp_lock);

	if (phm_cap_enabled(hwmgr->platform_descriptor.platformCaps, PHM_PlatformCaps_DynamicPatchPowerState))
		ret = phm_get_max_high_clocks(hwmgr, clocks);

	mutex_unlock(&pp_handle->pp_lock);
	return ret;
}

>>>>>>> temp
