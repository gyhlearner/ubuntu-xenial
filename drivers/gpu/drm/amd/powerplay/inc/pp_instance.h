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
#ifndef _PP_INSTANCE_H_
#define _PP_INSTANCE_H_

<<<<<<< HEAD
#include "smumgr.h"
#include "hwmgr.h"
#include "eventmgr.h"

#define PP_VALID  0x1F1F1F1F

struct pp_instance {
	uint32_t pp_valid;
	struct pp_smumgr *smu_mgr;
	struct pp_hwmgr *hwmgr;
	struct pp_eventmgr *eventmgr;
=======
#include "hwmgr.h"

struct pp_instance {
	uint32_t chip_family;
	uint32_t chip_id;
	bool pm_en;
	uint32_t feature_mask;
	void *device;
	struct pp_hwmgr *hwmgr;
	struct mutex pp_lock;
>>>>>>> temp
};

#endif
