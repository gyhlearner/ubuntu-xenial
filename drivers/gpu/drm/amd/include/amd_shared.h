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
 */

#ifndef __AMD_SHARED_H__
#define __AMD_SHARED_H__

#include <drm/amd_asic_type.h>

struct seq_file;

#define AMD_MAX_USEC_TIMEOUT		200000  /* 200 ms */

/*
 * Chip flags
 */
enum amd_chip_flags {
	AMD_ASIC_MASK = 0x0000ffffUL,
	AMD_FLAGS_MASK  = 0xffff0000UL,
	AMD_IS_MOBILITY = 0x00010000UL,
	AMD_IS_APU      = 0x00020000UL,
	AMD_IS_PX       = 0x00040000UL,
	AMD_EXP_HW_SUPPORT = 0x00080000UL,
};

enum amd_ip_block_type {
	AMD_IP_BLOCK_TYPE_COMMON,
	AMD_IP_BLOCK_TYPE_GMC,
	AMD_IP_BLOCK_TYPE_IH,
	AMD_IP_BLOCK_TYPE_SMC,
	AMD_IP_BLOCK_TYPE_PSP,
	AMD_IP_BLOCK_TYPE_DCE,
	AMD_IP_BLOCK_TYPE_GFX,
	AMD_IP_BLOCK_TYPE_SDMA,
	AMD_IP_BLOCK_TYPE_UVD,
	AMD_IP_BLOCK_TYPE_VCE,
	AMD_IP_BLOCK_TYPE_ACP,
	AMD_IP_BLOCK_TYPE_VCN
};

enum amd_clockgating_state {
	AMD_CG_STATE_GATE = 0,
	AMD_CG_STATE_UNGATE,
};

enum amd_dpm_forced_level {
	AMD_DPM_FORCED_LEVEL_AUTO = 0x1,
	AMD_DPM_FORCED_LEVEL_MANUAL = 0x2,
	AMD_DPM_FORCED_LEVEL_LOW = 0x4,
	AMD_DPM_FORCED_LEVEL_HIGH = 0x8,
	AMD_DPM_FORCED_LEVEL_PROFILE_STANDARD = 0x10,
	AMD_DPM_FORCED_LEVEL_PROFILE_MIN_SCLK = 0x20,
	AMD_DPM_FORCED_LEVEL_PROFILE_MIN_MCLK = 0x40,
	AMD_DPM_FORCED_LEVEL_PROFILE_PEAK = 0x80,
	AMD_DPM_FORCED_LEVEL_PROFILE_EXIT = 0x100,
};

enum amd_powergating_state {
	AMD_PG_STATE_GATE = 0,
	AMD_PG_STATE_UNGATE,
};

<<<<<<< HEAD
=======
struct amd_vce_state {
	/* vce clocks */
	u32 evclk;
	u32 ecclk;
	/* gpu clocks */
	u32 sclk;
	u32 mclk;
	u8 clk_idx;
	u8 pstate;
};


#define AMD_MAX_VCE_LEVELS 6

enum amd_vce_level {
	AMD_VCE_LEVEL_AC_ALL = 0,     /* AC, All cases */
	AMD_VCE_LEVEL_DC_EE = 1,      /* DC, entropy encoding */
	AMD_VCE_LEVEL_DC_LL_LOW = 2,  /* DC, low latency queue, res <= 720 */
	AMD_VCE_LEVEL_DC_LL_HIGH = 3, /* DC, low latency queue, 1080 >= res > 720 */
	AMD_VCE_LEVEL_DC_GP_LOW = 4,  /* DC, general purpose queue, res <= 720 */
	AMD_VCE_LEVEL_DC_GP_HIGH = 5, /* DC, general purpose queue, 1080 >= res > 720 */
};

enum amd_pp_profile_type {
	AMD_PP_GFX_PROFILE,
	AMD_PP_COMPUTE_PROFILE,
};

struct amd_pp_profile {
	enum amd_pp_profile_type type;
	uint32_t min_sclk;
	uint32_t min_mclk;
	uint16_t activity_threshold;
	uint8_t up_hyst;
	uint8_t down_hyst;
};

enum amd_fan_ctrl_mode {
	AMD_FAN_CTRL_NONE = 0,
	AMD_FAN_CTRL_MANUAL = 1,
	AMD_FAN_CTRL_AUTO = 2,
};

enum pp_clock_type {
	PP_SCLK,
	PP_MCLK,
	PP_PCIE,
};

>>>>>>> temp
/* CG flags */
#define AMD_CG_SUPPORT_GFX_MGCG			(1 << 0)
#define AMD_CG_SUPPORT_GFX_MGLS			(1 << 1)
#define AMD_CG_SUPPORT_GFX_CGCG			(1 << 2)
#define AMD_CG_SUPPORT_GFX_CGLS			(1 << 3)
#define AMD_CG_SUPPORT_GFX_CGTS			(1 << 4)
#define AMD_CG_SUPPORT_GFX_CGTS_LS		(1 << 5)
#define AMD_CG_SUPPORT_GFX_CP_LS		(1 << 6)
#define AMD_CG_SUPPORT_GFX_RLC_LS		(1 << 7)
#define AMD_CG_SUPPORT_MC_LS			(1 << 8)
#define AMD_CG_SUPPORT_MC_MGCG			(1 << 9)
#define AMD_CG_SUPPORT_SDMA_LS			(1 << 10)
#define AMD_CG_SUPPORT_SDMA_MGCG		(1 << 11)
#define AMD_CG_SUPPORT_BIF_LS			(1 << 12)
#define AMD_CG_SUPPORT_UVD_MGCG			(1 << 13)
#define AMD_CG_SUPPORT_VCE_MGCG			(1 << 14)
#define AMD_CG_SUPPORT_HDP_LS			(1 << 15)
#define AMD_CG_SUPPORT_HDP_MGCG			(1 << 16)
<<<<<<< HEAD
=======
#define AMD_CG_SUPPORT_ROM_MGCG			(1 << 17)
#define AMD_CG_SUPPORT_DRM_LS			(1 << 18)
#define AMD_CG_SUPPORT_BIF_MGCG			(1 << 19)
#define AMD_CG_SUPPORT_GFX_3D_CGCG		(1 << 20)
#define AMD_CG_SUPPORT_GFX_3D_CGLS		(1 << 21)
#define AMD_CG_SUPPORT_DRM_MGCG			(1 << 22)
#define AMD_CG_SUPPORT_DF_MGCG			(1 << 23)
>>>>>>> temp

/* PG flags */
#define AMD_PG_SUPPORT_GFX_PG			(1 << 0)
#define AMD_PG_SUPPORT_GFX_SMG			(1 << 1)
#define AMD_PG_SUPPORT_GFX_DMG			(1 << 2)
#define AMD_PG_SUPPORT_UVD			(1 << 3)
#define AMD_PG_SUPPORT_VCE			(1 << 4)
#define AMD_PG_SUPPORT_CP			(1 << 5)
#define AMD_PG_SUPPORT_GDS			(1 << 6)
#define AMD_PG_SUPPORT_RLC_SMU_HS		(1 << 7)
#define AMD_PG_SUPPORT_SDMA			(1 << 8)
#define AMD_PG_SUPPORT_ACP			(1 << 9)
#define AMD_PG_SUPPORT_SAMU			(1 << 10)
<<<<<<< HEAD
=======
#define AMD_PG_SUPPORT_GFX_QUICK_MG		(1 << 11)
#define AMD_PG_SUPPORT_GFX_PIPELINE		(1 << 12)
#define AMD_PG_SUPPORT_MMHUB			(1 << 13)
>>>>>>> temp

enum amd_pm_state_type {
	/* not used for dpm */
	POWER_STATE_TYPE_DEFAULT,
	POWER_STATE_TYPE_POWERSAVE,
	/* user selectable states */
	POWER_STATE_TYPE_BATTERY,
	POWER_STATE_TYPE_BALANCED,
	POWER_STATE_TYPE_PERFORMANCE,
	/* internal states */
	POWER_STATE_TYPE_INTERNAL_UVD,
	POWER_STATE_TYPE_INTERNAL_UVD_SD,
	POWER_STATE_TYPE_INTERNAL_UVD_HD,
	POWER_STATE_TYPE_INTERNAL_UVD_HD2,
	POWER_STATE_TYPE_INTERNAL_UVD_MVC,
	POWER_STATE_TYPE_INTERNAL_BOOT,
	POWER_STATE_TYPE_INTERNAL_THERMAL,
	POWER_STATE_TYPE_INTERNAL_ACPI,
	POWER_STATE_TYPE_INTERNAL_ULV,
	POWER_STATE_TYPE_INTERNAL_3DPERF,
};

struct amd_ip_funcs {
	/* Name of IP block */
	char *name;
	/* sets up early driver state (pre sw_init), does not configure hw - Optional */
	int (*early_init)(void *handle);
	/* sets up late driver/hw state (post hw_init) - Optional */
	int (*late_init)(void *handle);
	/* sets up driver state, does not configure hw */
	int (*sw_init)(void *handle);
	/* tears down driver state, does not configure hw */
	int (*sw_fini)(void *handle);
	/* sets up the hw state */
	int (*hw_init)(void *handle);
	/* tears down the hw state */
	int (*hw_fini)(void *handle);
	void (*late_fini)(void *handle);
	/* handles IP specific hw/sw changes for suspend */
	int (*suspend)(void *handle);
	/* handles IP specific hw/sw changes for resume */
	int (*resume)(void *handle);
	/* returns current IP block idle status */
	bool (*is_idle)(void *handle);
	/* poll for idle */
	int (*wait_for_idle)(void *handle);
	/* check soft reset the IP block */
	bool (*check_soft_reset)(void *handle);
	/* pre soft reset the IP block */
	int (*pre_soft_reset)(void *handle);
	/* soft reset the IP block */
	int (*soft_reset)(void *handle);
	/* post soft reset the IP block */
	int (*post_soft_reset)(void *handle);
	/* enable/disable cg for the IP block */
	int (*set_clockgating_state)(void *handle,
				     enum amd_clockgating_state state);
	/* enable/disable pg for the IP block */
	int (*set_powergating_state)(void *handle,
				     enum amd_powergating_state state);
	/* get current clockgating status */
	void (*get_clockgating_state)(void *handle, u32 *flags);
};


enum amd_pp_task;
enum amd_pp_clock_type;
struct pp_states_info;
struct amd_pp_simple_clock_info;
struct amd_pp_display_configuration;
struct amd_pp_clock_info;
struct pp_display_clock_request;
struct pp_wm_sets_with_clock_ranges_soc15;
struct pp_clock_levels_with_voltage;
struct pp_clock_levels_with_latency;
struct amd_pp_clocks;

struct amd_pm_funcs {
/* export for dpm on ci and si */
	int (*pre_set_power_state)(void *handle);
	int (*set_power_state)(void *handle);
	void (*post_set_power_state)(void *handle);
	void (*display_configuration_changed)(void *handle);
	void (*print_power_state)(void *handle, void *ps);
	bool (*vblank_too_short)(void *handle);
	void (*enable_bapm)(void *handle, bool enable);
	int (*check_state_equal)(void *handle,
				void  *cps,
				void  *rps,
				bool  *equal);
/* export for sysfs */
	int (*get_temperature)(void *handle);
	void (*set_fan_control_mode)(void *handle, u32 mode);
	u32 (*get_fan_control_mode)(void *handle);
	int (*set_fan_speed_percent)(void *handle, u32 speed);
	int (*get_fan_speed_percent)(void *handle, u32 *speed);
	int (*force_clock_level)(void *handle, enum pp_clock_type type, uint32_t mask);
	int (*print_clock_levels)(void *handle, enum pp_clock_type type, char *buf);
	int (*force_performance_level)(void *handle, enum amd_dpm_forced_level level);
	int (*get_sclk_od)(void *handle);
	int (*set_sclk_od)(void *handle, uint32_t value);
	int (*get_mclk_od)(void *handle);
	int (*set_mclk_od)(void *handle, uint32_t value);
	int (*read_sensor)(void *handle, int idx, void *value, int *size);
	enum amd_dpm_forced_level (*get_performance_level)(void *handle);
	enum amd_pm_state_type (*get_current_power_state)(void *handle);
	int (*get_fan_speed_rpm)(void *handle, uint32_t *rpm);
	int (*get_pp_num_states)(void *handle, struct pp_states_info *data);
	int (*get_pp_table)(void *handle, char **table);
	int (*set_pp_table)(void *handle, const char *buf, size_t size);
	void (*debugfs_print_current_performance_level)(void *handle, struct seq_file *m);

	int (*reset_power_profile_state)(void *handle,
			struct amd_pp_profile *request);
	int (*get_power_profile_state)(void *handle,
			struct amd_pp_profile *query);
	int (*set_power_profile_state)(void *handle,
			struct amd_pp_profile *request);
	int (*switch_power_profile)(void *handle,
			enum amd_pp_profile_type type);
/* export to amdgpu */
	void (*powergate_uvd)(void *handle, bool gate);
	void (*powergate_vce)(void *handle, bool gate);
	struct amd_vce_state* (*get_vce_clock_state)(void *handle, u32 idx);
	int (*dispatch_tasks)(void *handle, enum amd_pp_task task_id,
				   void *input, void *output);
	int (*load_firmware)(void *handle);
	int (*wait_for_fw_loading_complete)(void *handle);
	int (*set_clockgating_by_smu)(void *handle, uint32_t msg_id);
/* export to DC */
	u32 (*get_sclk)(void *handle, bool low);
	u32 (*get_mclk)(void *handle, bool low);
	int (*display_configuration_change)(void *handle,
		const struct amd_pp_display_configuration *input);
	int (*get_display_power_level)(void *handle,
		struct amd_pp_simple_clock_info *output);
	int (*get_current_clocks)(void *handle,
		struct amd_pp_clock_info *clocks);
	int (*get_clock_by_type)(void *handle,
		enum amd_pp_clock_type type,
		struct amd_pp_clocks *clocks);
	int (*get_clock_by_type_with_latency)(void *handle,
		enum amd_pp_clock_type type,
		struct pp_clock_levels_with_latency *clocks);
	int (*get_clock_by_type_with_voltage)(void *handle,
		enum amd_pp_clock_type type,
		struct pp_clock_levels_with_voltage *clocks);
	int (*set_watermarks_for_clocks_ranges)(void *handle,
		struct pp_wm_sets_with_clock_ranges_soc15 *wm_with_clock_ranges);
	int (*display_clock_voltage_request)(void *handle,
		struct pp_display_clock_request *clock);
	int (*get_display_mode_validation_clocks)(void *handle,
		struct amd_pp_simple_clock_info *clocks);
};


#endif /* __AMD_SHARED_H__ */
