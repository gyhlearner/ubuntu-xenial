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
#include "smumgr.h"
=======
#include "pp_debug.h"
#include "smumgr.h"
#include "smu7_dyn_defaults.h"
>>>>>>> temp
#include "smu73.h"
#include "smu_ucode_xfer_vi.h"
#include "fiji_smumgr.h"
#include "fiji_ppsmc.h"
#include "smu73_discrete.h"
#include "ppatomctrl.h"
#include "smu/smu_7_1_3_d.h"
#include "smu/smu_7_1_3_sh_mask.h"
#include "gmc/gmc_8_1_d.h"
#include "gmc/gmc_8_1_sh_mask.h"
#include "oss/oss_3_0_d.h"
#include "gca/gfx_8_0_d.h"
#include "bif/bif_5_0_d.h"
#include "bif/bif_5_0_sh_mask.h"
<<<<<<< HEAD
#include "pp_debug.h"
#include "fiji_pwrvirus.h"
=======
#include "dce/dce_10_0_d.h"
#include "dce/dce_10_0_sh_mask.h"
#include "hardwaremanager.h"
#include "cgs_common.h"
#include "atombios.h"
#include "pppcielanes.h"
#include "hwmgr.h"
#include "smu7_hwmgr.h"

>>>>>>> temp

#define AVFS_EN_MSB                                        1568
#define AVFS_EN_LSB                                        1568

#define FIJI_SMC_SIZE 0x20000

<<<<<<< HEAD
struct SMU73_Discrete_GraphicsLevel avfs_graphics_level[8] = {
=======
#define VOLTAGE_SCALE 4
#define POWERTUNE_DEFAULT_SET_MAX    1
#define VOLTAGE_VID_OFFSET_SCALE1   625
#define VOLTAGE_VID_OFFSET_SCALE2   100
#define VDDC_VDDCI_DELTA            300
#define MC_CG_ARB_FREQ_F1           0x0b

/* [2.5%,~2.5%] Clock stretched is multiple of 2.5% vs
 * not and [Fmin, Fmax, LDO_REFSEL, USE_FOR_LOW_FREQ]
 */
static const uint16_t fiji_clock_stretcher_lookup_table[2][4] = {
				{600, 1050, 3, 0}, {600, 1050, 6, 1} };

/* [FF, SS] type, [] 4 voltage ranges, and
 * [Floor Freq, Boundary Freq, VID min , VID max]
 */
static const uint32_t fiji_clock_stretcher_ddt_table[2][4][4] = {
	{ {265, 529, 120, 128}, {325, 650, 96, 119}, {430, 860, 32, 95}, {0, 0, 0, 31} },
	{ {275, 550, 104, 112}, {319, 638, 96, 103}, {360, 720, 64, 95}, {384, 768, 32, 63} } };

/* [Use_For_Low_freq] value, [0%, 5%, 10%, 7.14%, 14.28%, 20%]
 * (coming from PWR_CKS_CNTL.stretch_amount reg spec)
 */
static const uint8_t fiji_clock_stretch_amount_conversion[2][6] = {
				{0, 1, 3, 2, 4, 5}, {0, 2, 4, 5, 6, 5} };

static const struct fiji_pt_defaults fiji_power_tune_data_set_array[POWERTUNE_DEFAULT_SET_MAX] = {
		/*sviLoadLIneEn,  SviLoadLineVddC, TDC_VDDC_ThrottleReleaseLimitPerc */
		{1,               0xF,             0xFD,
		/* TDC_MAWt, TdcWaterfallCtl, DTEAmbientTempBase */
		0x19,        5,               45}
};

static const struct SMU73_Discrete_GraphicsLevel avfs_graphics_level[8] = {
>>>>>>> temp
		/*  Min        Sclk       pcie     DeepSleep Activity  CgSpll      CgSpll    spllSpread  SpllSpread   CcPwr  CcPwr  Sclk   Display     Enabled     Enabled                       Voltage    Power */
		/* Voltage,  Frequency,  DpmLevel,  DivId,    Level,  FuncCntl3,  FuncCntl4,  Spectrum,   Spectrum2,  DynRm, DynRm1  Did, Watermark, ForActivity, ForThrottle, UpHyst, DownHyst, DownHyst, Throttle */
		{ 0x3c0fd047, 0x30750000,   0x00,     0x03,   0x1e00, 0x00200410, 0x87020000, 0x21680000, 0x0c000000,   0,      0,   0x16,   0x00,       0x01,        0x01,      0x00,   0x00,      0x00,     0x00 },
		{ 0xa00fd047, 0x409c0000,   0x01,     0x04,   0x1e00, 0x00800510, 0x87020000, 0x21680000, 0x11000000,   0,      0,   0x16,   0x00,       0x01,        0x01,      0x00,   0x00,      0x00,     0x00 },
		{ 0x0410d047, 0x50c30000,   0x01,     0x00,   0x1e00, 0x00600410, 0x87020000, 0x21680000, 0x0d000000,   0,      0,   0x0e,   0x00,       0x01,        0x01,      0x00,   0x00,      0x00,     0x00 },
		{ 0x6810d047, 0x60ea0000,   0x01,     0x00,   0x1e00, 0x00800410, 0x87020000, 0x21680000, 0x0e000000,   0,      0,   0x0c,   0x00,       0x01,        0x01,      0x00,   0x00,      0x00,     0x00 },
		{ 0xcc10d047, 0xe8fd0000,   0x01,     0x00,   0x1e00, 0x00e00410, 0x87020000, 0x21680000, 0x0f000000,   0,      0,   0x0c,   0x00,       0x01,        0x01,      0x00,   0x00,      0x00,     0x00 },
		{ 0x3011d047, 0x70110100,   0x01,     0x00,   0x1e00, 0x00400510, 0x87020000, 0x21680000, 0x10000000,   0,      0,   0x0c,   0x00,       0x01,        0x01,      0x00,   0x00,      0x00,     0x00 },
		{ 0x9411d047, 0xf8240100,   0x01,     0x00,   0x1e00, 0x00a00510, 0x87020000, 0x21680000, 0x11000000,   0,      0,   0x0c,   0x00,       0x01,        0x01,      0x00,   0x00,      0x00,     0x00 },
		{ 0xf811d047, 0x80380100,   0x01,     0x00,   0x1e00, 0x00000610, 0x87020000, 0x21680000, 0x12000000,   0,      0,   0x0c,   0x01,       0x01,        0x01,      0x00,   0x00,      0x00,     0x00 }
};

<<<<<<< HEAD
static enum cgs_ucode_id fiji_convert_fw_type_to_cgs(uint32_t fw_type)
{
	enum cgs_ucode_id result = CGS_UCODE_ID_MAXIMUM;

	switch (fw_type) {
	case UCODE_ID_SMU:
		result = CGS_UCODE_ID_SMU;
		break;
	case UCODE_ID_SDMA0:
		result = CGS_UCODE_ID_SDMA0;
		break;
	case UCODE_ID_SDMA1:
		result = CGS_UCODE_ID_SDMA1;
		break;
	case UCODE_ID_CP_CE:
		result = CGS_UCODE_ID_CP_CE;
		break;
	case UCODE_ID_CP_PFP:
		result = CGS_UCODE_ID_CP_PFP;
		break;
	case UCODE_ID_CP_ME:
		result = CGS_UCODE_ID_CP_ME;
		break;
	case UCODE_ID_CP_MEC:
		result = CGS_UCODE_ID_CP_MEC;
		break;
	case UCODE_ID_CP_MEC_JT1:
		result = CGS_UCODE_ID_CP_MEC_JT1;
		break;
	case UCODE_ID_CP_MEC_JT2:
		result = CGS_UCODE_ID_CP_MEC_JT2;
		break;
	case UCODE_ID_RLC_G:
		result = CGS_UCODE_ID_RLC_G;
		break;
	default:
		break;
	}

	return result;
}
/**
* Set the address for reading/writing the SMC SRAM space.
* @param    smumgr  the address of the powerplay hardware manager.
* @param    smc_addr the address in the SMC RAM to access.
*/
static int fiji_set_smc_sram_address(struct pp_smumgr *smumgr,
		uint32_t smc_addr, uint32_t limit)
{
	PP_ASSERT_WITH_CODE((0 == (3 & smc_addr)),
			"SMC address must be 4 byte aligned.", return -EINVAL;);
	PP_ASSERT_WITH_CODE((limit > (smc_addr + 3)),
			"SMC address is beyond the SMC RAM area.", return -EINVAL;);

	cgs_write_register(smumgr->device, mmSMC_IND_INDEX_0, smc_addr);
	SMUM_WRITE_FIELD(smumgr->device, SMC_IND_ACCESS_CNTL, AUTO_INCREMENT_IND_0, 0);

	return 0;
}

/**
* Copy bytes from an array into the SMC RAM space.
*
* @param    smumgr  the address of the powerplay SMU manager.
* @param    smcStartAddress the start address in the SMC RAM to copy bytes to.
* @param    src the byte array to copy the bytes from.
* @param    byteCount the number of bytes to copy.
*/
int fiji_copy_bytes_to_smc(struct pp_smumgr *smumgr,
		uint32_t smcStartAddress, const uint8_t *src,
		uint32_t byteCount, uint32_t limit)
{
	int result;
	uint32_t data, originalData;
	uint32_t addr, extraShift;

	PP_ASSERT_WITH_CODE((0 == (3 & smcStartAddress)),
			"SMC address must be 4 byte aligned.", return -EINVAL;);
	PP_ASSERT_WITH_CODE((limit > (smcStartAddress + byteCount)),
			"SMC address is beyond the SMC RAM area.", return -EINVAL;);

	addr = smcStartAddress;

	while (byteCount >= 4) {
		/* Bytes are written into the SMC addres space with the MSB first. */
		data = src[0] * 0x1000000 + src[1] * 0x10000 + src[2] * 0x100 + src[3];

		result = fiji_set_smc_sram_address(smumgr, addr, limit);
		if (result)
			return result;

		cgs_write_register(smumgr->device, mmSMC_IND_DATA_0, data);

		src += 4;
		byteCount -= 4;
		addr += 4;
	}

	if (byteCount) {
		/* Now write the odd bytes left.
		 * Do a read modify write cycle.
		 */
		data = 0;

		result = fiji_set_smc_sram_address(smumgr, addr, limit);
		if (result)
			return result;

		originalData = cgs_read_register(smumgr->device, mmSMC_IND_DATA_0);
		extraShift = 8 * (4 - byteCount);

		while (byteCount > 0) {
			/* Bytes are written into the SMC addres
			 * space with the MSB first.
			 */
			data = (0x100 * data) + *src++;
			byteCount--;
		}
		data <<= extraShift;
		data |= (originalData & ~((~0UL) << extraShift));

		result = fiji_set_smc_sram_address(smumgr, addr, limit);
		if (!result)
			return result;

		cgs_write_register(smumgr->device, mmSMC_IND_DATA_0, data);
	}
	return 0;
}

int fiji_program_jump_on_start(struct pp_smumgr *smumgr)
{
	static unsigned char data[] = { 0xE0, 0x00, 0x80, 0x40 };

	fiji_copy_bytes_to_smc(smumgr, 0x0, data, 4, sizeof(data) + 1);

	return 0;
}

/**
* Return if the SMC is currently running.
*
* @param    smumgr  the address of the powerplay hardware manager.
*/
bool fiji_is_smc_ram_running(struct pp_smumgr *smumgr)
{
	return ((0 == SMUM_READ_VFPF_INDIRECT_FIELD(smumgr->device,
			CGS_IND_REG__SMC,
			SMC_SYSCON_CLOCK_CNTL_0, ck_disable))
			&& (0x20100 <= cgs_read_ind_register(smumgr->device,
					CGS_IND_REG__SMC, ixSMC_PC_C)));
}

/**
* Send a message to the SMC, and wait for its response.
*
* @param    smumgr  the address of the powerplay hardware manager.
* @param    msg the message to send.
* @return   The response that came from the SMC.
*/
int fiji_send_msg_to_smc(struct pp_smumgr *smumgr, uint16_t msg)
{
	if (!fiji_is_smc_ram_running(smumgr))
		return -1;

	if (1 != SMUM_READ_FIELD(smumgr->device, SMC_RESP_0, SMC_RESP)) {
		printk(KERN_ERR "Failed to send Previous Message.");
		SMUM_WAIT_FIELD_UNEQUAL(smumgr, SMC_RESP_0, SMC_RESP, 0);
	}

	cgs_write_register(smumgr->device, mmSMC_MESSAGE_0, msg);
	SMUM_WAIT_FIELD_UNEQUAL(smumgr, SMC_RESP_0, SMC_RESP, 0);

	return 0;
}

/**
 * Send a message to the SMC with parameter
 * @param    smumgr:  the address of the powerplay hardware manager.
 * @param    msg: the message to send.
 * @param    parameter: the parameter to send
 * @return   The response that came from the SMC.
 */
int fiji_send_msg_to_smc_with_parameter(struct pp_smumgr *smumgr,
		uint16_t msg, uint32_t parameter)
{
	if (!fiji_is_smc_ram_running(smumgr))
		return -1;

	if (1 != SMUM_READ_FIELD(smumgr->device, SMC_RESP_0, SMC_RESP)) {
		printk(KERN_ERR "Failed to send Previous Message.");
		SMUM_WAIT_FIELD_UNEQUAL(smumgr, SMC_RESP_0, SMC_RESP, 0);
	}

	cgs_write_register(smumgr->device, mmSMC_MSG_ARG_0, parameter);
	cgs_write_register(smumgr->device, mmSMC_MESSAGE_0, msg);
	SMUM_WAIT_FIELD_UNEQUAL(smumgr, SMC_RESP_0, SMC_RESP, 0);

	return 0;
}


/**
* Send a message to the SMC with parameter, do not wait for response
*
* @param    smumgr:  the address of the powerplay hardware manager.
* @param    msg: the message to send.
* @param    parameter: the parameter to send
* @return   The response that came from the SMC.
*/
int fiji_send_msg_to_smc_with_parameter_without_waiting(
		struct pp_smumgr *smumgr, uint16_t msg, uint32_t parameter)
{
	if (1 != SMUM_READ_FIELD(smumgr->device, SMC_RESP_0, SMC_RESP)) {
		printk(KERN_ERR "Failed to send Previous Message.");
		SMUM_WAIT_FIELD_UNEQUAL(smumgr, SMC_RESP_0, SMC_RESP, 0);
	}
	cgs_write_register(smumgr->device, mmSMC_MSG_ARG_0, parameter);
	cgs_write_register(smumgr->device, mmSMC_MESSAGE_0, msg);

	return 0;
}

/**
* Uploads the SMU firmware from .hex file
*
* @param    smumgr  the address of the powerplay SMU manager.
* @return   0 or -1.
*/

static int fiji_upload_smu_firmware_image(struct pp_smumgr *smumgr)
{
	const uint8_t *src;
	uint32_t byte_count;
	uint32_t *data;
	struct cgs_firmware_info info = {0};

	cgs_get_firmware_info(smumgr->device,
			fiji_convert_fw_type_to_cgs(UCODE_ID_SMU), &info);

	if (info.image_size & 3) {
		printk(KERN_ERR "SMC ucode is not 4 bytes aligned\n");
		return -EINVAL;
	}

	if (info.image_size > FIJI_SMC_SIZE) {
		printk(KERN_ERR "SMC address is beyond the SMC RAM area\n");
		return -EINVAL;
	}

	cgs_write_register(smumgr->device, mmSMC_IND_INDEX_0, 0x20000);
	SMUM_WRITE_FIELD(smumgr->device, SMC_IND_ACCESS_CNTL, AUTO_INCREMENT_IND_0, 1);

	byte_count = info.image_size;
	src = (const uint8_t *)info.kptr;

	data = (uint32_t *)src;
	for (; byte_count >= 4; data++, byte_count -= 4)
		cgs_write_register(smumgr->device, mmSMC_IND_DATA_0, data[0]);

	SMUM_WRITE_FIELD(smumgr->device, SMC_IND_ACCESS_CNTL, AUTO_INCREMENT_IND_0, 0);
	return 0;
}

/**
* Read a 32bit value from the SMC SRAM space.
* ALL PARAMETERS ARE IN HOST BYTE ORDER.
* @param    smumgr  the address of the powerplay hardware manager.
* @param    smc_addr the address in the SMC RAM to access.
* @param    value and output parameter for the data read from the SMC SRAM.
*/
int fiji_read_smc_sram_dword(struct pp_smumgr *smumgr, uint32_t smc_addr,
		uint32_t *value, uint32_t limit)
{
	int	result = fiji_set_smc_sram_address(smumgr, smc_addr, limit);

	if (result)
		return result;

	*value = cgs_read_register(smumgr->device, mmSMC_IND_DATA_0);
	return 0;
}

/**
* Write a 32bit value to the SMC SRAM space.
* ALL PARAMETERS ARE IN HOST BYTE ORDER.
* @param    smumgr  the address of the powerplay hardware manager.
* @param    smc_addr the address in the SMC RAM to access.
* @param    value to write to the SMC SRAM.
*/
int fiji_write_smc_sram_dword(struct pp_smumgr *smumgr, uint32_t smc_addr,
		uint32_t value, uint32_t limit)
{
	int result;

	result = fiji_set_smc_sram_address(smumgr, smc_addr, limit);

	if (result)
		return result;

	cgs_write_register(smumgr->device, mmSMC_IND_DATA_0, value);
	return 0;
}

static uint32_t fiji_get_mask_for_firmware_type(uint32_t fw_type)
{
	uint32_t result = 0;

	switch (fw_type) {
	case UCODE_ID_SDMA0:
		result = UCODE_ID_SDMA0_MASK;
		break;
	case UCODE_ID_SDMA1:
		result = UCODE_ID_SDMA1_MASK;
		break;
	case UCODE_ID_CP_CE:
		result = UCODE_ID_CP_CE_MASK;
		break;
	case UCODE_ID_CP_PFP:
		result = UCODE_ID_CP_PFP_MASK;
		break;
	case UCODE_ID_CP_ME:
		result = UCODE_ID_CP_ME_MASK;
		break;
	case UCODE_ID_CP_MEC_JT1:
		result = UCODE_ID_CP_MEC_MASK | UCODE_ID_CP_MEC_JT1_MASK;
		break;
	case UCODE_ID_CP_MEC_JT2:
		result = UCODE_ID_CP_MEC_MASK | UCODE_ID_CP_MEC_JT2_MASK;
		break;
	case UCODE_ID_RLC_G:
		result = UCODE_ID_RLC_G_MASK;
		break;
	default:
		printk(KERN_ERR "UCode type is out of range!");
		result = 0;
	}

	return result;
}

/* Populate one firmware image to the data structure */
static int fiji_populate_single_firmware_entry(struct pp_smumgr *smumgr,
		uint32_t fw_type, struct SMU_Entry *entry)
{
	int result;
	struct cgs_firmware_info info = {0};

	result = cgs_get_firmware_info(
			smumgr->device,
			fiji_convert_fw_type_to_cgs(fw_type),
			&info);

	if (!result) {
		entry->version = 0;
		entry->id = (uint16_t)fw_type;
		entry->image_addr_high = smu_upper_32_bits(info.mc_addr);
		entry->image_addr_low = smu_lower_32_bits(info.mc_addr);
		entry->meta_data_addr_high = 0;
		entry->meta_data_addr_low = 0;
		entry->data_size_byte = info.image_size;
		entry->num_register_entries = 0;

		if (fw_type == UCODE_ID_RLC_G)
			entry->flags = 1;
		else
			entry->flags = 0;
	}

	return result;
}

static int fiji_request_smu_load_fw(struct pp_smumgr *smumgr)
{
	struct fiji_smumgr *priv = (struct fiji_smumgr *)(smumgr->backend);
	uint32_t fw_to_load;
	struct SMU_DRAMData_TOC *toc;

	if (priv->soft_regs_start)
		cgs_write_ind_register(smumgr->device, CGS_IND_REG__SMC,
				priv->soft_regs_start +
				offsetof(SMU73_SoftRegisters, UcodeLoadStatus),
				0x0);

	toc = (struct SMU_DRAMData_TOC *)priv->header;
	toc->num_entries = 0;
	toc->structure_version = 1;

	PP_ASSERT_WITH_CODE(
			0 == fiji_populate_single_firmware_entry(smumgr,
					UCODE_ID_RLC_G, &toc->entry[toc->num_entries++]),
			"Failed to Get Firmware Entry.\n" , return -1 );
	PP_ASSERT_WITH_CODE(
			0 == fiji_populate_single_firmware_entry(smumgr,
					UCODE_ID_CP_CE, &toc->entry[toc->num_entries++]),
			"Failed to Get Firmware Entry.\n" , return -1 );
	PP_ASSERT_WITH_CODE(
			0 == fiji_populate_single_firmware_entry(smumgr,
					UCODE_ID_CP_PFP, &toc->entry[toc->num_entries++]),
			"Failed to Get Firmware Entry.\n" , return -1 );
	PP_ASSERT_WITH_CODE(
			0 == fiji_populate_single_firmware_entry(smumgr,
					UCODE_ID_CP_ME, &toc->entry[toc->num_entries++]),
			"Failed to Get Firmware Entry.\n" , return -1 );
	PP_ASSERT_WITH_CODE(
			0 == fiji_populate_single_firmware_entry(smumgr,
					UCODE_ID_CP_MEC, &toc->entry[toc->num_entries++]),
			"Failed to Get Firmware Entry.\n" , return -1 );
	PP_ASSERT_WITH_CODE(
			0 == fiji_populate_single_firmware_entry(smumgr,
					UCODE_ID_CP_MEC_JT1, &toc->entry[toc->num_entries++]),
					"Failed to Get Firmware Entry.\n" , return -1 );
	PP_ASSERT_WITH_CODE(
			0 == fiji_populate_single_firmware_entry(smumgr,
					UCODE_ID_CP_MEC_JT2, &toc->entry[toc->num_entries++]),
					"Failed to Get Firmware Entry.\n" , return -1 );
	PP_ASSERT_WITH_CODE(
			0 == fiji_populate_single_firmware_entry(smumgr,
					UCODE_ID_SDMA0, &toc->entry[toc->num_entries++]),
					"Failed to Get Firmware Entry.\n" , return -1 );
	PP_ASSERT_WITH_CODE(
			0 == fiji_populate_single_firmware_entry(smumgr,
					UCODE_ID_SDMA1, &toc->entry[toc->num_entries++]),
					"Failed to Get Firmware Entry.\n" , return -1 );

	fiji_send_msg_to_smc_with_parameter(smumgr, PPSMC_MSG_DRV_DRAM_ADDR_HI,
			priv->header_buffer.mc_addr_high);
	fiji_send_msg_to_smc_with_parameter(smumgr,PPSMC_MSG_DRV_DRAM_ADDR_LO,
			priv->header_buffer.mc_addr_low);

	fw_to_load = UCODE_ID_RLC_G_MASK
			+ UCODE_ID_SDMA0_MASK
			+ UCODE_ID_SDMA1_MASK
			+ UCODE_ID_CP_CE_MASK
			+ UCODE_ID_CP_ME_MASK
			+ UCODE_ID_CP_PFP_MASK
			+ UCODE_ID_CP_MEC_MASK
			+ UCODE_ID_CP_MEC_JT1_MASK
			+ UCODE_ID_CP_MEC_JT2_MASK;

	if (fiji_send_msg_to_smc_with_parameter(smumgr,
			PPSMC_MSG_LoadUcodes, fw_to_load))
		printk(KERN_ERR "Fail to Request SMU Load uCode");

	return 0;
}


/* Check if the FW has been loaded, SMU will not return
 * if loading has not finished.
 */
static int fiji_check_fw_load_finish(struct pp_smumgr *smumgr,
		uint32_t fw_type)
{
	struct fiji_smumgr *priv = (struct fiji_smumgr *)(smumgr->backend);
	uint32_t mask = fiji_get_mask_for_firmware_type(fw_type);

	/* Check SOFT_REGISTERS_TABLE_28.UcodeLoadStatus */
	if (smum_wait_on_indirect_register(smumgr, mmSMC_IND_INDEX,
			priv->soft_regs_start +
			offsetof(SMU73_SoftRegisters, UcodeLoadStatus),
			mask, mask)) {
		printk(KERN_ERR "check firmware loading failed\n");
		return -EINVAL;
	}
	return 0;
}


static int fiji_reload_firmware(struct pp_smumgr *smumgr)
{
	return smumgr->smumgr_funcs->start_smu(smumgr);
}

static bool fiji_is_hw_virtualization_enabled(struct pp_smumgr *smumgr)
{
	uint32_t value;

	value = cgs_read_register(smumgr->device, mmBIF_IOV_FUNC_IDENTIFIER);
	if (value & BIF_IOV_FUNC_IDENTIFIER__IOV_ENABLE_MASK) {
		/* driver reads on SR-IOV enabled PF: 0x80000000
		 * driver reads on SR-IOV enabled VF: 0x80000001
		 * driver reads on SR-IOV disabled:   0x00000000
		 */
		return true;
	}
	return false;
}

static int fiji_request_smu_specific_fw_load(struct pp_smumgr *smumgr, uint32_t fw_type)
{
	if (fiji_is_hw_virtualization_enabled(smumgr)) {
		uint32_t masks = fiji_get_mask_for_firmware_type(fw_type);
		if (fiji_send_msg_to_smc_with_parameter_without_waiting(smumgr,
				PPSMC_MSG_LoadUcodes, masks))
			printk(KERN_ERR "Fail to Request SMU Load uCode");
	}
	/* For non-virtualization cases,
	 * SMU loads all FWs at once in fiji_request_smu_load_fw.
	 */
	return 0;
}

static int fiji_start_smu_in_protection_mode(struct pp_smumgr *smumgr)
=======
static int fiji_start_smu_in_protection_mode(struct pp_hwmgr *hwmgr)
>>>>>>> temp
{
	int result = 0;

	/* Wait for smc boot up */
<<<<<<< HEAD
	/* SMUM_WAIT_INDIRECT_FIELD_UNEQUAL(smumgr, SMC_IND,
		RCU_UC_EVENTS, boot_seq_done, 0); */

	SMUM_WRITE_VFPF_INDIRECT_FIELD(smumgr->device, CGS_IND_REG__SMC,
			SMC_SYSCON_RESET_CNTL, rst_reg, 1);

	result = fiji_upload_smu_firmware_image(smumgr);
=======
	/* PHM_WAIT_INDIRECT_FIELD_UNEQUAL(hwmgr, SMC_IND,
		RCU_UC_EVENTS, boot_seq_done, 0); */

	PHM_WRITE_VFPF_INDIRECT_FIELD(hwmgr->device, CGS_IND_REG__SMC,
			SMC_SYSCON_RESET_CNTL, rst_reg, 1);

	result = smu7_upload_smu_firmware_image(hwmgr);
>>>>>>> temp
	if (result)
		return result;

	/* Clear status */
<<<<<<< HEAD
	cgs_write_ind_register(smumgr->device, CGS_IND_REG__SMC,
			ixSMU_STATUS, 0);

	SMUM_WRITE_VFPF_INDIRECT_FIELD(smumgr->device, CGS_IND_REG__SMC,
			SMC_SYSCON_CLOCK_CNTL_0, ck_disable, 0);

	/* De-assert reset */
	SMUM_WRITE_VFPF_INDIRECT_FIELD(smumgr->device, CGS_IND_REG__SMC,
			SMC_SYSCON_RESET_CNTL, rst_reg, 0);

	/* Wait for ROM firmware to initialize interrupt hendler */
	/*SMUM_WAIT_VFPF_INDIRECT_REGISTER(smumgr, SMC_IND,
			SMC_INTR_CNTL_MASK_0, 0x10040, 0xFFFFFFFF); */

	/* Set SMU Auto Start */
	SMUM_WRITE_VFPF_INDIRECT_FIELD(smumgr->device, CGS_IND_REG__SMC,
			SMU_INPUT_DATA, AUTO_START, 1);

	/* Clear firmware interrupt enable flag */
	cgs_write_ind_register(smumgr->device, CGS_IND_REG__SMC,
			ixFIRMWARE_FLAGS, 0);

	SMUM_WAIT_VFPF_INDIRECT_FIELD(smumgr, SMC_IND, RCU_UC_EVENTS,
			INTERRUPTS_ENABLED, 1);

	cgs_write_register(smumgr->device, mmSMC_MSG_ARG_0, 0x20000);
	cgs_write_register(smumgr->device, mmSMC_MESSAGE_0, PPSMC_MSG_Test);
	SMUM_WAIT_FIELD_UNEQUAL(smumgr, SMC_RESP_0, SMC_RESP, 0);

	/* Wait for done bit to be set */
	SMUM_WAIT_VFPF_INDIRECT_FIELD_UNEQUAL(smumgr, SMC_IND,
			SMU_STATUS, SMU_DONE, 0);

	/* Check pass/failed indicator */
	if (1 != SMUM_READ_VFPF_INDIRECT_FIELD(smumgr->device, CGS_IND_REG__SMC,
			SMU_STATUS, SMU_PASS)) {
=======
	cgs_write_ind_register(hwmgr->device, CGS_IND_REG__SMC,
			ixSMU_STATUS, 0);

	PHM_WRITE_VFPF_INDIRECT_FIELD(hwmgr->device, CGS_IND_REG__SMC,
			SMC_SYSCON_CLOCK_CNTL_0, ck_disable, 0);

	/* De-assert reset */
	PHM_WRITE_VFPF_INDIRECT_FIELD(hwmgr->device, CGS_IND_REG__SMC,
			SMC_SYSCON_RESET_CNTL, rst_reg, 0);

	/* Wait for ROM firmware to initialize interrupt hendler */
	/*SMUM_WAIT_VFPF_INDIRECT_REGISTER(hwmgr, SMC_IND,
			SMC_INTR_CNTL_MASK_0, 0x10040, 0xFFFFFFFF); */

	/* Set SMU Auto Start */
	PHM_WRITE_VFPF_INDIRECT_FIELD(hwmgr->device, CGS_IND_REG__SMC,
			SMU_INPUT_DATA, AUTO_START, 1);

	/* Clear firmware interrupt enable flag */
	cgs_write_ind_register(hwmgr->device, CGS_IND_REG__SMC,
			ixFIRMWARE_FLAGS, 0);

	PHM_WAIT_VFPF_INDIRECT_FIELD(hwmgr, SMC_IND, RCU_UC_EVENTS,
			INTERRUPTS_ENABLED, 1);

	cgs_write_register(hwmgr->device, mmSMC_MSG_ARG_0, 0x20000);
	cgs_write_register(hwmgr->device, mmSMC_MESSAGE_0, PPSMC_MSG_Test);
	PHM_WAIT_FIELD_UNEQUAL(hwmgr, SMC_RESP_0, SMC_RESP, 0);

	/* Wait for done bit to be set */
	PHM_WAIT_VFPF_INDIRECT_FIELD_UNEQUAL(hwmgr, SMC_IND,
			SMU_STATUS, SMU_DONE, 0);

	/* Check pass/failed indicator */
	if (PHM_READ_VFPF_INDIRECT_FIELD(hwmgr->device, CGS_IND_REG__SMC,
			SMU_STATUS, SMU_PASS) != 1) {
>>>>>>> temp
		PP_ASSERT_WITH_CODE(false,
				"SMU Firmware start failed!", return -1);
	}

	/* Wait for firmware to initialize */
<<<<<<< HEAD
	SMUM_WAIT_VFPF_INDIRECT_FIELD(smumgr, SMC_IND,
=======
	PHM_WAIT_VFPF_INDIRECT_FIELD(hwmgr, SMC_IND,
>>>>>>> temp
			FIRMWARE_FLAGS, INTERRUPTS_ENABLED, 1);

	return result;
}

<<<<<<< HEAD
static int fiji_start_smu_in_non_protection_mode(struct pp_smumgr *smumgr)
=======
static int fiji_start_smu_in_non_protection_mode(struct pp_hwmgr *hwmgr)
>>>>>>> temp
{
	int result = 0;

	/* wait for smc boot up */
<<<<<<< HEAD
	SMUM_WAIT_VFPF_INDIRECT_FIELD_UNEQUAL(smumgr, SMC_IND,
			RCU_UC_EVENTS, boot_seq_done, 0);

	/* Clear firmware interrupt enable flag */
	cgs_write_ind_register(smumgr->device, CGS_IND_REG__SMC,
			ixFIRMWARE_FLAGS, 0);

	/* Assert reset */
	SMUM_WRITE_VFPF_INDIRECT_FIELD(smumgr->device, CGS_IND_REG__SMC,
			SMC_SYSCON_RESET_CNTL, rst_reg, 1);

	result = fiji_upload_smu_firmware_image(smumgr);
=======
	PHM_WAIT_VFPF_INDIRECT_FIELD_UNEQUAL(hwmgr, SMC_IND,
			RCU_UC_EVENTS, boot_seq_done, 0);

	/* Clear firmware interrupt enable flag */
	cgs_write_ind_register(hwmgr->device, CGS_IND_REG__SMC,
			ixFIRMWARE_FLAGS, 0);

	/* Assert reset */
	PHM_WRITE_VFPF_INDIRECT_FIELD(hwmgr->device, CGS_IND_REG__SMC,
			SMC_SYSCON_RESET_CNTL, rst_reg, 1);

	result = smu7_upload_smu_firmware_image(hwmgr);
>>>>>>> temp
	if (result)
		return result;

	/* Set smc instruct start point at 0x0 */
<<<<<<< HEAD
	fiji_program_jump_on_start(smumgr);

	/* Enable clock */
	SMUM_WRITE_VFPF_INDIRECT_FIELD(smumgr->device, CGS_IND_REG__SMC,
			SMC_SYSCON_CLOCK_CNTL_0, ck_disable, 0);

	/* De-assert reset */
	SMUM_WRITE_VFPF_INDIRECT_FIELD(smumgr->device, CGS_IND_REG__SMC,
			SMC_SYSCON_RESET_CNTL, rst_reg, 0);

	/* Wait for firmware to initialize */
	SMUM_WAIT_VFPF_INDIRECT_FIELD(smumgr, SMC_IND,
=======
	smu7_program_jump_on_start(hwmgr);

	/* Enable clock */
	PHM_WRITE_VFPF_INDIRECT_FIELD(hwmgr->device, CGS_IND_REG__SMC,
			SMC_SYSCON_CLOCK_CNTL_0, ck_disable, 0);

	/* De-assert reset */
	PHM_WRITE_VFPF_INDIRECT_FIELD(hwmgr->device, CGS_IND_REG__SMC,
			SMC_SYSCON_RESET_CNTL, rst_reg, 0);

	/* Wait for firmware to initialize */
	PHM_WAIT_VFPF_INDIRECT_FIELD(hwmgr, SMC_IND,
>>>>>>> temp
			FIRMWARE_FLAGS, INTERRUPTS_ENABLED, 1);

	return result;
}

<<<<<<< HEAD
int fiji_setup_pwr_virus(struct pp_smumgr *smumgr)
{
	int i, result = -1;
	uint32_t reg, data;
	PWR_Command_Table *virus = PwrVirusTable;
	struct fiji_smumgr *priv = (struct fiji_smumgr *)(smumgr->backend);

	priv->avfs.AvfsBtcStatus = AVFS_LOAD_VIRUS;
	for (i = 0; (i < PWR_VIRUS_TABLE_SIZE); i++) {
		switch (virus->command) {
		case PwrCmdWrite:
			reg  = virus->reg;
			data = virus->data;
			cgs_write_register(smumgr->device, reg, data);
			break;
		case PwrCmdEnd:
			priv->avfs.AvfsBtcStatus = AVFS_BTC_VIRUS_LOADED;
			result = 0;
			break;
		default:
			printk(KERN_ERR "Table Exit with Invalid Command!");
			priv->avfs.AvfsBtcStatus = AVFS_BTC_VIRUS_FAIL;
			result = -1;
			break;
		}
		virus++;
	}
	return result;
}

static int fiji_start_avfs_btc(struct pp_smumgr *smumgr)
{
	int result = 0;
	struct fiji_smumgr *priv = (struct fiji_smumgr *)(smumgr->backend);

	priv->avfs.AvfsBtcStatus = AVFS_BTC_STARTED;
	if (priv->avfs.AvfsBtcParam) {
		if (!fiji_send_msg_to_smc_with_parameter(smumgr,
				PPSMC_MSG_PerformBtc, priv->avfs.AvfsBtcParam)) {
			if (!fiji_send_msg_to_smc(smumgr, PPSMC_MSG_EnableAvfs)) {
				priv->avfs.AvfsBtcStatus = AVFS_BTC_COMPLETED_UNSAVED;
				result = 0;
			} else {
				printk(KERN_ERR "[AVFS][fiji_start_avfs_btc] Attempt"
						" to Enable AVFS Failed!");
				fiji_send_msg_to_smc(smumgr, PPSMC_MSG_DisableAvfs);
				result = -1;
			}
		} else {
			printk(KERN_ERR "[AVFS][fiji_start_avfs_btc] "
					"PerformBTC SMU msg failed");
			result = -1;
=======
static int fiji_start_avfs_btc(struct pp_hwmgr *hwmgr)
{
	int result = 0;
	struct smu7_smumgr *smu_data = (struct smu7_smumgr *)(hwmgr->smu_backend);

	if (0 != smu_data->avfs.avfs_btc_param) {
		if (0 != smu7_send_msg_to_smc_with_parameter(hwmgr,
				PPSMC_MSG_PerformBtc, smu_data->avfs.avfs_btc_param)) {
			pr_info("[AVFS][Fiji_PerformBtc] PerformBTC SMU msg failed");
			result = -EINVAL;
>>>>>>> temp
		}
	}
	/* Soft-Reset to reset the engine before loading uCode */
	 /* halt */
<<<<<<< HEAD
	cgs_write_register(smumgr->device, mmCP_MEC_CNTL, 0x50000000);
	/* reset everything */
	cgs_write_register(smumgr->device, mmGRBM_SOFT_RESET, 0xffffffff);
	/* clear reset */
	cgs_write_register(smumgr->device, mmGRBM_SOFT_RESET, 0);

	return result;
}

int fiji_setup_pm_fuse_for_avfs(struct pp_smumgr *smumgr)
{
	int result = 0;
	uint32_t table_start;
	uint32_t charz_freq_addr, inversion_voltage_addr, charz_freq;
	uint16_t inversion_voltage;

	charz_freq = 0x30750000; /* In 10KHz units 0x00007530 Actual value */
	inversion_voltage = 0x1A04; /* mV Q14.2 0x41A Actual value */

	PP_ASSERT_WITH_CODE(0 == fiji_read_smc_sram_dword(smumgr,
			SMU7_FIRMWARE_HEADER_LOCATION + offsetof(SMU73_Firmware_Header,
					PmFuseTable), &table_start, 0x40000),
			"[AVFS][Fiji_SetupGfxLvlStruct] SMU could not communicate "
			"starting address of PmFuse structure",
			return -1;);

	charz_freq_addr = table_start +
			offsetof(struct SMU73_Discrete_PmFuses, PsmCharzFreq);
	inversion_voltage_addr = table_start +
			offsetof(struct SMU73_Discrete_PmFuses, InversionVoltage);

	result = fiji_copy_bytes_to_smc(smumgr, charz_freq_addr,
			(uint8_t *)(&charz_freq), sizeof(charz_freq), 0x40000);
	PP_ASSERT_WITH_CODE(0 == result,
			"[AVFS][fiji_setup_pm_fuse_for_avfs] charz_freq could not "
			"be populated.", return -1;);

	result = fiji_copy_bytes_to_smc(smumgr, inversion_voltage_addr,
			(uint8_t *)(&inversion_voltage), sizeof(inversion_voltage), 0x40000);
	PP_ASSERT_WITH_CODE(0 == result, "[AVFS][fiji_setup_pm_fuse_for_avfs] "
			"charz_freq could not be populated.", return -1;);
=======
	cgs_write_register(hwmgr->device, mmCP_MEC_CNTL, 0x50000000);
	/* reset everything */
	cgs_write_register(hwmgr->device, mmGRBM_SOFT_RESET, 0xffffffff);
	/* clear reset */
	cgs_write_register(hwmgr->device, mmGRBM_SOFT_RESET, 0);
>>>>>>> temp

	return result;
}

<<<<<<< HEAD
int fiji_setup_graphics_level_structure(struct pp_smumgr *smumgr)
=======
static int fiji_setup_graphics_level_structure(struct pp_hwmgr *hwmgr)
>>>>>>> temp
{
	int32_t vr_config;
	uint32_t table_start;
	uint32_t level_addr, vr_config_addr;
	uint32_t level_size = sizeof(avfs_graphics_level);

<<<<<<< HEAD
	PP_ASSERT_WITH_CODE(0 == fiji_read_smc_sram_dword(smumgr,
=======
	PP_ASSERT_WITH_CODE(0 == smu7_read_smc_sram_dword(hwmgr,
>>>>>>> temp
			SMU7_FIRMWARE_HEADER_LOCATION +
			offsetof(SMU73_Firmware_Header, DpmTable),
			&table_start, 0x40000),
			"[AVFS][Fiji_SetupGfxLvlStruct] SMU could not "
			"communicate starting address of DPM table",
			return -1;);

	/* Default value for vr_config =
	 * VR_MERGED_WITH_VDDC + VR_STATIC_VOLTAGE(VDDCI) */
	vr_config = 0x01000500;   /* Real value:0x50001 */

	vr_config_addr = table_start +
			offsetof(SMU73_Discrete_DpmTable, VRConfig);

<<<<<<< HEAD
	PP_ASSERT_WITH_CODE(0 == fiji_copy_bytes_to_smc(smumgr, vr_config_addr,
=======
	PP_ASSERT_WITH_CODE(0 == smu7_copy_bytes_to_smc(hwmgr, vr_config_addr,
>>>>>>> temp
			(uint8_t *)&vr_config, sizeof(int32_t), 0x40000),
			"[AVFS][Fiji_SetupGfxLvlStruct] Problems copying "
			"vr_config value over to SMC",
			return -1;);

	level_addr = table_start + offsetof(SMU73_Discrete_DpmTable, GraphicsLevel);

<<<<<<< HEAD
	PP_ASSERT_WITH_CODE(0 == fiji_copy_bytes_to_smc(smumgr, level_addr,
=======
	PP_ASSERT_WITH_CODE(0 == smu7_copy_bytes_to_smc(hwmgr, level_addr,
>>>>>>> temp
			(uint8_t *)(&avfs_graphics_level), level_size, 0x40000),
			"[AVFS][Fiji_SetupGfxLvlStruct] Copying of DPM table failed!",
			return -1;);

	return 0;
}

<<<<<<< HEAD
/* Work in Progress */
int fiji_restore_vft_table(struct pp_smumgr *smumgr)
{
	struct fiji_smumgr *priv = (struct fiji_smumgr *)(smumgr->backend);

	if (AVFS_BTC_COMPLETED_SAVED == priv->avfs.AvfsBtcStatus) {
		priv->avfs.AvfsBtcStatus = AVFS_BTC_COMPLETED_RESTORED;
		return 0;
	} else
		return -EINVAL;
}

/* Work in Progress */
int fiji_save_vft_table(struct pp_smumgr *smumgr)
{
	struct fiji_smumgr *priv = (struct fiji_smumgr *)(smumgr->backend);

	if (AVFS_BTC_COMPLETED_SAVED == priv->avfs.AvfsBtcStatus) {
		priv->avfs.AvfsBtcStatus = AVFS_BTC_COMPLETED_RESTORED;
		return 0;
	} else
		return -EINVAL;
}

int fiji_avfs_event_mgr(struct pp_smumgr *smumgr, bool smu_started)
{
	struct fiji_smumgr *priv = (struct fiji_smumgr *)(smumgr->backend);

	switch (priv->avfs.AvfsBtcStatus) {
	case AVFS_BTC_COMPLETED_SAVED: /*S3 State - Pre SMU Start */
		priv->avfs.AvfsBtcStatus = AVFS_BTC_RESTOREVFT_FAILED;
		PP_ASSERT_WITH_CODE(0 == fiji_restore_vft_table(smumgr),
				"[AVFS][fiji_avfs_event_mgr] Could not Copy Graphics "
				"Level table over to SMU",
				return -1;);
		priv->avfs.AvfsBtcStatus = AVFS_BTC_COMPLETED_RESTORED;
		break;
	case AVFS_BTC_COMPLETED_RESTORED: /*S3 State - Post SMU Start*/
		priv->avfs.AvfsBtcStatus = AVFS_BTC_SMUMSG_ERROR;
		PP_ASSERT_WITH_CODE(0 == fiji_send_msg_to_smc(smumgr,
				PPSMC_MSG_VftTableIsValid),
				"[AVFS][fiji_avfs_event_mgr] SMU did not respond "
				"correctly to VftTableIsValid Msg",
				return -1;);
		priv->avfs.AvfsBtcStatus = AVFS_BTC_SMUMSG_ERROR;
		PP_ASSERT_WITH_CODE(0 == fiji_send_msg_to_smc(smumgr,
				PPSMC_MSG_EnableAvfs),
				"[AVFS][fiji_avfs_event_mgr] SMU did not respond "
				"correctly to EnableAvfs Message Msg",
				return -1;);
		priv->avfs.AvfsBtcStatus = AVFS_BTC_COMPLETED_SAVED;
		break;
	case AVFS_BTC_BOOT: /*Cold Boot State - Post SMU Start*/
		if (!smu_started)
			break;
		priv->avfs.AvfsBtcStatus = AVFS_BTC_FAILED;
		PP_ASSERT_WITH_CODE(0 == fiji_setup_pm_fuse_for_avfs(smumgr),
				"[AVFS][fiji_avfs_event_mgr] Failure at "
				"fiji_setup_pm_fuse_for_avfs",
				return -1;);
		priv->avfs.AvfsBtcStatus = AVFS_BTC_DPMTABLESETUP_FAILED;
		PP_ASSERT_WITH_CODE(0 == fiji_setup_graphics_level_structure(smumgr),
				"[AVFS][fiji_avfs_event_mgr] Could not Copy Graphics Level"
				" table over to SMU",
				return -1;);
		priv->avfs.AvfsBtcStatus = AVFS_BTC_VIRUS_FAIL;
		PP_ASSERT_WITH_CODE(0 == fiji_setup_pwr_virus(smumgr),
				"[AVFS][fiji_avfs_event_mgr] Could not setup "
				"Pwr Virus for AVFS ",
				return -1;);
		priv->avfs.AvfsBtcStatus = AVFS_BTC_FAILED;
		PP_ASSERT_WITH_CODE(0 == fiji_start_avfs_btc(smumgr),
				"[AVFS][fiji_avfs_event_mgr] Failure at "
				"fiji_start_avfs_btc. AVFS Disabled",
				return -1;);
		priv->avfs.AvfsBtcStatus = AVFS_BTC_SAVEVFT_FAILED;
		PP_ASSERT_WITH_CODE(0 == fiji_save_vft_table(smumgr),
				"[AVFS][fiji_avfs_event_mgr] Could not save VFT Table",
				return -1;);
		priv->avfs.AvfsBtcStatus = AVFS_BTC_COMPLETED_SAVED;
		break;
	case AVFS_BTC_DISABLED: /* Do nothing */
		break;
	case AVFS_BTC_NOTSUPPORTED: /* Do nothing */
		break;
	default:
		printk(KERN_ERR "[AVFS] Something is broken. See log!");
=======
static int fiji_avfs_event_mgr(struct pp_hwmgr *hwmgr, bool smu_started)
{
	struct smu7_smumgr *smu_data = (struct smu7_smumgr *)(hwmgr->smu_backend);

	switch (smu_data->avfs.avfs_btc_status) {
	case AVFS_BTC_COMPLETED_PREVIOUSLY:
		break;

	case AVFS_BTC_BOOT: /*Cold Boot State - Post SMU Start*/
		if (!smu_started)
			break;
		smu_data->avfs.avfs_btc_status = AVFS_BTC_FAILED;
		PP_ASSERT_WITH_CODE(0 == fiji_setup_graphics_level_structure(hwmgr),
				"[AVFS][fiji_avfs_event_mgr] Could not Copy Graphics Level"
				" table over to SMU",
				return -EINVAL;);
		smu_data->avfs.avfs_btc_status = AVFS_BTC_VIRUS_FAIL;
		PP_ASSERT_WITH_CODE(0 == smu7_setup_pwr_virus(hwmgr),
				"[AVFS][fiji_avfs_event_mgr] Could not setup "
				"Pwr Virus for AVFS ",
				return -EINVAL;);
		smu_data->avfs.avfs_btc_status = AVFS_BTC_FAILED;
		PP_ASSERT_WITH_CODE(0 == fiji_start_avfs_btc(hwmgr),
				"[AVFS][fiji_avfs_event_mgr] Failure at "
				"fiji_start_avfs_btc. AVFS Disabled",
				return -EINVAL;);

		smu_data->avfs.avfs_btc_status = AVFS_BTC_ENABLEAVFS;
		break;
	case AVFS_BTC_DISABLED: /* Do nothing */
	case AVFS_BTC_NOTSUPPORTED: /* Do nothing */
	case AVFS_BTC_ENABLEAVFS:
		break;
	default:
		pr_err("AVFS failed status is %x !\n", smu_data->avfs.avfs_btc_status);
>>>>>>> temp
		break;
	}
	return 0;
}

<<<<<<< HEAD
static int fiji_start_smu(struct pp_smumgr *smumgr)
{
	int result = 0;
	struct fiji_smumgr *priv = (struct fiji_smumgr *)(smumgr->backend);

	/* Only start SMC if SMC RAM is not running */
	if (!fiji_is_smc_ram_running(smumgr)) {
		fiji_avfs_event_mgr(smumgr, false);

		/* Check if SMU is running in protected mode */
		if (0 == SMUM_READ_VFPF_INDIRECT_FIELD(smumgr->device,
				CGS_IND_REG__SMC,
				SMU_FIRMWARE, SMU_MODE)) {
			result = fiji_start_smu_in_non_protection_mode(smumgr);
			if (result)
				return result;
		} else {
			result = fiji_start_smu_in_protection_mode(smumgr);
			if (result)
				return result;
		}
		fiji_avfs_event_mgr(smumgr, true);
	}

	/* To initialize all clock gating before RLC loaded and running.*/
	cgs_set_clockgating_state(smumgr->device,
			AMD_IP_BLOCK_TYPE_GFX, AMD_CG_STATE_GATE);
	cgs_set_clockgating_state(smumgr->device,
			AMD_IP_BLOCK_TYPE_GMC, AMD_CG_STATE_GATE);
	cgs_set_clockgating_state(smumgr->device,
			AMD_IP_BLOCK_TYPE_SDMA, AMD_CG_STATE_GATE);
	cgs_set_clockgating_state(smumgr->device,
=======
static int fiji_start_smu(struct pp_hwmgr *hwmgr)
{
	int result = 0;
	struct fiji_smumgr *priv = (struct fiji_smumgr *)(hwmgr->smu_backend);

	/* Only start SMC if SMC RAM is not running */
	if (!(smu7_is_smc_ram_running(hwmgr)
		|| cgs_is_virtualization_enabled(hwmgr->device))) {
		fiji_avfs_event_mgr(hwmgr, false);

		/* Check if SMU is running in protected mode */
		if (0 == PHM_READ_VFPF_INDIRECT_FIELD(hwmgr->device,
				CGS_IND_REG__SMC,
				SMU_FIRMWARE, SMU_MODE)) {
			result = fiji_start_smu_in_non_protection_mode(hwmgr);
			if (result)
				return result;
		} else {
			result = fiji_start_smu_in_protection_mode(hwmgr);
			if (result)
				return result;
		}
		fiji_avfs_event_mgr(hwmgr, true);
	}

	/* To initialize all clock gating before RLC loaded and running.*/
	cgs_set_clockgating_state(hwmgr->device,
			AMD_IP_BLOCK_TYPE_GFX, AMD_CG_STATE_GATE);
	cgs_set_clockgating_state(hwmgr->device,
			AMD_IP_BLOCK_TYPE_GMC, AMD_CG_STATE_GATE);
	cgs_set_clockgating_state(hwmgr->device,
			AMD_IP_BLOCK_TYPE_SDMA, AMD_CG_STATE_GATE);
	cgs_set_clockgating_state(hwmgr->device,
>>>>>>> temp
			AMD_IP_BLOCK_TYPE_COMMON, AMD_CG_STATE_GATE);

	/* Setup SoftRegsStart here for register lookup in case
	 * DummyBackEnd is used and ProcessFirmwareHeader is not executed
	 */
<<<<<<< HEAD
	fiji_read_smc_sram_dword(smumgr,
			SMU7_FIRMWARE_HEADER_LOCATION +
			offsetof(SMU73_Firmware_Header, SoftRegisters),
			&(priv->soft_regs_start), 0x40000);

	result = fiji_request_smu_load_fw(smumgr);
=======
	smu7_read_smc_sram_dword(hwmgr,
			SMU7_FIRMWARE_HEADER_LOCATION +
			offsetof(SMU73_Firmware_Header, SoftRegisters),
			&(priv->smu7_data.soft_regs_start), 0x40000);

	result = smu7_request_smu_load_fw(hwmgr);
>>>>>>> temp

	return result;
}

<<<<<<< HEAD
static bool fiji_is_hw_avfs_present(struct pp_smumgr *smumgr)
=======
static bool fiji_is_hw_avfs_present(struct pp_hwmgr *hwmgr)
>>>>>>> temp
{

	uint32_t efuse = 0;
	uint32_t mask = (1 << ((AVFS_EN_MSB - AVFS_EN_LSB) + 1)) - 1;

<<<<<<< HEAD
	if (!atomctrl_read_efuse(smumgr->device, AVFS_EN_LSB, AVFS_EN_MSB,
=======
	if (cgs_is_virtualization_enabled(hwmgr->device))
		return 0;

	if (!atomctrl_read_efuse(hwmgr->device, AVFS_EN_LSB, AVFS_EN_MSB,
>>>>>>> temp
			mask, &efuse)) {
		if (efuse)
			return true;
	}
	return false;
}

<<<<<<< HEAD
/**
* Write a 32bit value to the SMC SRAM space.
* ALL PARAMETERS ARE IN HOST BYTE ORDER.
* @param    smumgr  the address of the powerplay hardware manager.
* @param    smc_addr the address in the SMC RAM to access.
* @param    value to write to the SMC SRAM.
*/
static int fiji_smu_init(struct pp_smumgr *smumgr)
{
	struct fiji_smumgr *priv = (struct fiji_smumgr *)(smumgr->backend);
	uint64_t mc_addr;

	priv->header_buffer.data_size =
			((sizeof(struct SMU_DRAMData_TOC) / 4096) + 1) * 4096;
	smu_allocate_memory(smumgr->device,
			priv->header_buffer.data_size,
			CGS_GPU_MEM_TYPE__VISIBLE_CONTIG_FB,
			PAGE_SIZE,
			&mc_addr,
			&priv->header_buffer.kaddr,
			&priv->header_buffer.handle);

	priv->header = priv->header_buffer.kaddr;
	priv->header_buffer.mc_addr_high = smu_upper_32_bits(mc_addr);
	priv->header_buffer.mc_addr_low = smu_lower_32_bits(mc_addr);

	PP_ASSERT_WITH_CODE((NULL != priv->header),
			"Out of memory.",
			kfree(smumgr->backend);
			cgs_free_gpu_mem(smumgr->device,
			(cgs_handle_t)priv->header_buffer.handle);
			return -1);

	priv->avfs.AvfsBtcStatus = AVFS_BTC_BOOT;
	if (fiji_is_hw_avfs_present(smumgr))
		/* AVFS Parameter
		 * 0 - BTC DC disabled, BTC AC disabled
		 * 1 - BTC DC enabled,  BTC AC disabled
		 * 2 - BTC DC disabled, BTC AC enabled
		 * 3 - BTC DC enabled,  BTC AC enabled
		 * Default is 0 - BTC DC disabled, BTC AC disabled
		 */
		priv->avfs.AvfsBtcParam = 0;
	else
		priv->avfs.AvfsBtcStatus = AVFS_BTC_NOTSUPPORTED;

	priv->acpi_optimization = 1;
=======
static int fiji_smu_init(struct pp_hwmgr *hwmgr)
{
	int i;
	struct fiji_smumgr *fiji_priv = NULL;

	fiji_priv = kzalloc(sizeof(struct fiji_smumgr), GFP_KERNEL);

	if (fiji_priv == NULL)
		return -ENOMEM;

	hwmgr->smu_backend = fiji_priv;

	if (smu7_init(hwmgr))
		return -EINVAL;

	for (i = 0; i < SMU73_MAX_LEVELS_GRAPHICS; i++)
		fiji_priv->activity_target[i] = 30;
>>>>>>> temp

	return 0;
}

<<<<<<< HEAD
static int fiji_smu_fini(struct pp_smumgr *smumgr)
{
	if (smumgr->backend) {
		kfree(smumgr->backend);
		smumgr->backend = NULL;
	}
	return 0;
}

static const struct pp_smumgr_func fiji_smu_funcs = {
	.smu_init = &fiji_smu_init,
	.smu_fini = &fiji_smu_fini,
	.start_smu = &fiji_start_smu,
	.check_fw_load_finish = &fiji_check_fw_load_finish,
	.request_smu_load_fw = &fiji_reload_firmware,
	.request_smu_load_specific_fw = &fiji_request_smu_specific_fw_load,
	.send_msg_to_smc = &fiji_send_msg_to_smc,
	.send_msg_to_smc_with_parameter = &fiji_send_msg_to_smc_with_parameter,
	.download_pptable_settings = NULL,
	.upload_pptable_settings = NULL,
};

int fiji_smum_init(struct pp_smumgr *smumgr)
{
	struct fiji_smumgr *fiji_smu = NULL;

	fiji_smu = kzalloc(sizeof(struct fiji_smumgr), GFP_KERNEL);

	if (fiji_smu == NULL)
		return -ENOMEM;

	smumgr->backend = fiji_smu;
	smumgr->smumgr_funcs = &fiji_smu_funcs;

	return 0;
}
=======
static int fiji_get_dependency_volt_by_clk(struct pp_hwmgr *hwmgr,
		struct phm_ppt_v1_clock_voltage_dependency_table *dep_table,
		uint32_t clock, uint32_t *voltage, uint32_t *mvdd)
{
	uint32_t i;
	uint16_t vddci;
	struct smu7_hwmgr *data = (struct smu7_hwmgr *)(hwmgr->backend);
	*voltage = *mvdd = 0;


	/* clock - voltage dependency table is empty table */
	if (dep_table->count == 0)
		return -EINVAL;

	for (i = 0; i < dep_table->count; i++) {
		/* find first sclk bigger than request */
		if (dep_table->entries[i].clk >= clock) {
			*voltage |= (dep_table->entries[i].vddc *
					VOLTAGE_SCALE) << VDDC_SHIFT;
			if (SMU7_VOLTAGE_CONTROL_NONE == data->vddci_control)
				*voltage |= (data->vbios_boot_state.vddci_bootup_value *
						VOLTAGE_SCALE) << VDDCI_SHIFT;
			else if (dep_table->entries[i].vddci)
				*voltage |= (dep_table->entries[i].vddci *
						VOLTAGE_SCALE) << VDDCI_SHIFT;
			else {
				vddci = phm_find_closest_vddci(&(data->vddci_voltage_table),
						(dep_table->entries[i].vddc -
								VDDC_VDDCI_DELTA));
				*voltage |= (vddci * VOLTAGE_SCALE) << VDDCI_SHIFT;
			}

			if (SMU7_VOLTAGE_CONTROL_NONE == data->mvdd_control)
				*mvdd = data->vbios_boot_state.mvdd_bootup_value *
					VOLTAGE_SCALE;
			else if (dep_table->entries[i].mvdd)
				*mvdd = (uint32_t) dep_table->entries[i].mvdd *
					VOLTAGE_SCALE;

			*voltage |= 1 << PHASES_SHIFT;
			return 0;
		}
	}

	/* sclk is bigger than max sclk in the dependence table */
	*voltage |= (dep_table->entries[i - 1].vddc * VOLTAGE_SCALE) << VDDC_SHIFT;

	if (SMU7_VOLTAGE_CONTROL_NONE == data->vddci_control)
		*voltage |= (data->vbios_boot_state.vddci_bootup_value *
				VOLTAGE_SCALE) << VDDCI_SHIFT;
	else if (dep_table->entries[i-1].vddci) {
		vddci = phm_find_closest_vddci(&(data->vddci_voltage_table),
				(dep_table->entries[i].vddc -
						VDDC_VDDCI_DELTA));
		*voltage |= (vddci * VOLTAGE_SCALE) << VDDCI_SHIFT;
	}

	if (SMU7_VOLTAGE_CONTROL_NONE == data->mvdd_control)
		*mvdd = data->vbios_boot_state.mvdd_bootup_value * VOLTAGE_SCALE;
	else if (dep_table->entries[i].mvdd)
		*mvdd = (uint32_t) dep_table->entries[i - 1].mvdd * VOLTAGE_SCALE;

	return 0;
}


static uint16_t scale_fan_gain_settings(uint16_t raw_setting)
{
	uint32_t tmp;
	tmp = raw_setting * 4096 / 100;
	return (uint16_t)tmp;
}

static void get_scl_sda_value(uint8_t line, uint8_t *scl, uint8_t *sda)
{
	switch (line) {
	case SMU7_I2CLineID_DDC1:
		*scl = SMU7_I2C_DDC1CLK;
		*sda = SMU7_I2C_DDC1DATA;
		break;
	case SMU7_I2CLineID_DDC2:
		*scl = SMU7_I2C_DDC2CLK;
		*sda = SMU7_I2C_DDC2DATA;
		break;
	case SMU7_I2CLineID_DDC3:
		*scl = SMU7_I2C_DDC3CLK;
		*sda = SMU7_I2C_DDC3DATA;
		break;
	case SMU7_I2CLineID_DDC4:
		*scl = SMU7_I2C_DDC4CLK;
		*sda = SMU7_I2C_DDC4DATA;
		break;
	case SMU7_I2CLineID_DDC5:
		*scl = SMU7_I2C_DDC5CLK;
		*sda = SMU7_I2C_DDC5DATA;
		break;
	case SMU7_I2CLineID_DDC6:
		*scl = SMU7_I2C_DDC6CLK;
		*sda = SMU7_I2C_DDC6DATA;
		break;
	case SMU7_I2CLineID_SCLSDA:
		*scl = SMU7_I2C_SCL;
		*sda = SMU7_I2C_SDA;
		break;
	case SMU7_I2CLineID_DDCVGA:
		*scl = SMU7_I2C_DDCVGACLK;
		*sda = SMU7_I2C_DDCVGADATA;
		break;
	default:
		*scl = 0;
		*sda = 0;
		break;
	}
}

static void fiji_initialize_power_tune_defaults(struct pp_hwmgr *hwmgr)
{
	struct fiji_smumgr *smu_data = (struct fiji_smumgr *)(hwmgr->smu_backend);
	struct phm_ppt_v1_information *table_info =
			(struct  phm_ppt_v1_information *)(hwmgr->pptable);

	if (table_info &&
			table_info->cac_dtp_table->usPowerTuneDataSetID <= POWERTUNE_DEFAULT_SET_MAX &&
			table_info->cac_dtp_table->usPowerTuneDataSetID)
		smu_data->power_tune_defaults =
				&fiji_power_tune_data_set_array
				[table_info->cac_dtp_table->usPowerTuneDataSetID - 1];
	else
		smu_data->power_tune_defaults = &fiji_power_tune_data_set_array[0];

}

static int fiji_populate_bapm_parameters_in_dpm_table(struct pp_hwmgr *hwmgr)
{

	struct fiji_smumgr *smu_data = (struct fiji_smumgr *)(hwmgr->smu_backend);
	const struct fiji_pt_defaults *defaults = smu_data->power_tune_defaults;

	SMU73_Discrete_DpmTable  *dpm_table = &(smu_data->smc_state_table);

	struct phm_ppt_v1_information *table_info =
			(struct phm_ppt_v1_information *)(hwmgr->pptable);
	struct phm_cac_tdp_table *cac_dtp_table = table_info->cac_dtp_table;
	struct pp_advance_fan_control_parameters *fan_table =
			&hwmgr->thermal_controller.advanceFanControlParameters;
	uint8_t uc_scl, uc_sda;

	/* TDP number of fraction bits are changed from 8 to 7 for Fiji
	 * as requested by SMC team
	 */
	dpm_table->DefaultTdp = PP_HOST_TO_SMC_US(
			(uint16_t)(cac_dtp_table->usTDP * 128));
	dpm_table->TargetTdp = PP_HOST_TO_SMC_US(
			(uint16_t)(cac_dtp_table->usTDP * 128));

	PP_ASSERT_WITH_CODE(cac_dtp_table->usTargetOperatingTemp <= 255,
			"Target Operating Temp is out of Range!",
			);

	dpm_table->GpuTjMax = (uint8_t)(cac_dtp_table->usTargetOperatingTemp);
	dpm_table->GpuTjHyst = 8;

	dpm_table->DTEAmbientTempBase = defaults->DTEAmbientTempBase;

	/* The following are for new Fiji Multi-input fan/thermal control */
	dpm_table->TemperatureLimitEdge = PP_HOST_TO_SMC_US(
			cac_dtp_table->usTargetOperatingTemp * 256);
	dpm_table->TemperatureLimitHotspot = PP_HOST_TO_SMC_US(
			cac_dtp_table->usTemperatureLimitHotspot * 256);
	dpm_table->TemperatureLimitLiquid1 = PP_HOST_TO_SMC_US(
			cac_dtp_table->usTemperatureLimitLiquid1 * 256);
	dpm_table->TemperatureLimitLiquid2 = PP_HOST_TO_SMC_US(
			cac_dtp_table->usTemperatureLimitLiquid2 * 256);
	dpm_table->TemperatureLimitVrVddc = PP_HOST_TO_SMC_US(
			cac_dtp_table->usTemperatureLimitVrVddc * 256);
	dpm_table->TemperatureLimitVrMvdd = PP_HOST_TO_SMC_US(
			cac_dtp_table->usTemperatureLimitVrMvdd * 256);
	dpm_table->TemperatureLimitPlx = PP_HOST_TO_SMC_US(
			cac_dtp_table->usTemperatureLimitPlx * 256);

	dpm_table->FanGainEdge = PP_HOST_TO_SMC_US(
			scale_fan_gain_settings(fan_table->usFanGainEdge));
	dpm_table->FanGainHotspot = PP_HOST_TO_SMC_US(
			scale_fan_gain_settings(fan_table->usFanGainHotspot));
	dpm_table->FanGainLiquid = PP_HOST_TO_SMC_US(
			scale_fan_gain_settings(fan_table->usFanGainLiquid));
	dpm_table->FanGainVrVddc = PP_HOST_TO_SMC_US(
			scale_fan_gain_settings(fan_table->usFanGainVrVddc));
	dpm_table->FanGainVrMvdd = PP_HOST_TO_SMC_US(
			scale_fan_gain_settings(fan_table->usFanGainVrMvdd));
	dpm_table->FanGainPlx = PP_HOST_TO_SMC_US(
			scale_fan_gain_settings(fan_table->usFanGainPlx));
	dpm_table->FanGainHbm = PP_HOST_TO_SMC_US(
			scale_fan_gain_settings(fan_table->usFanGainHbm));

	dpm_table->Liquid1_I2C_address = cac_dtp_table->ucLiquid1_I2C_address;
	dpm_table->Liquid2_I2C_address = cac_dtp_table->ucLiquid2_I2C_address;
	dpm_table->Vr_I2C_address = cac_dtp_table->ucVr_I2C_address;
	dpm_table->Plx_I2C_address = cac_dtp_table->ucPlx_I2C_address;

	get_scl_sda_value(cac_dtp_table->ucLiquid_I2C_Line, &uc_scl, &uc_sda);
	dpm_table->Liquid_I2C_LineSCL = uc_scl;
	dpm_table->Liquid_I2C_LineSDA = uc_sda;

	get_scl_sda_value(cac_dtp_table->ucVr_I2C_Line, &uc_scl, &uc_sda);
	dpm_table->Vr_I2C_LineSCL = uc_scl;
	dpm_table->Vr_I2C_LineSDA = uc_sda;

	get_scl_sda_value(cac_dtp_table->ucPlx_I2C_Line, &uc_scl, &uc_sda);
	dpm_table->Plx_I2C_LineSCL = uc_scl;
	dpm_table->Plx_I2C_LineSDA = uc_sda;

	return 0;
}


static int fiji_populate_svi_load_line(struct pp_hwmgr *hwmgr)
{
	struct fiji_smumgr *smu_data = (struct fiji_smumgr *)(hwmgr->smu_backend);
	const struct fiji_pt_defaults *defaults = smu_data->power_tune_defaults;

	smu_data->power_tune_table.SviLoadLineEn = defaults->SviLoadLineEn;
	smu_data->power_tune_table.SviLoadLineVddC = defaults->SviLoadLineVddC;
	smu_data->power_tune_table.SviLoadLineTrimVddC = 3;
	smu_data->power_tune_table.SviLoadLineOffsetVddC = 0;

	return 0;
}


static int fiji_populate_tdc_limit(struct pp_hwmgr *hwmgr)
{
	uint16_t tdc_limit;
	struct fiji_smumgr *smu_data = (struct fiji_smumgr *)(hwmgr->smu_backend);
	struct phm_ppt_v1_information *table_info =
			(struct phm_ppt_v1_information *)(hwmgr->pptable);
	const struct fiji_pt_defaults *defaults = smu_data->power_tune_defaults;

	/* TDC number of fraction bits are changed from 8 to 7
	 * for Fiji as requested by SMC team
	 */
	tdc_limit = (uint16_t)(table_info->cac_dtp_table->usTDC * 128);
	smu_data->power_tune_table.TDC_VDDC_PkgLimit =
			CONVERT_FROM_HOST_TO_SMC_US(tdc_limit);
	smu_data->power_tune_table.TDC_VDDC_ThrottleReleaseLimitPerc =
			defaults->TDC_VDDC_ThrottleReleaseLimitPerc;
	smu_data->power_tune_table.TDC_MAWt = defaults->TDC_MAWt;

	return 0;
}

static int fiji_populate_dw8(struct pp_hwmgr *hwmgr, uint32_t fuse_table_offset)
{
	struct fiji_smumgr *smu_data = (struct fiji_smumgr *)(hwmgr->smu_backend);
	const struct fiji_pt_defaults *defaults = smu_data->power_tune_defaults;
	uint32_t temp;

	if (smu7_read_smc_sram_dword(hwmgr,
			fuse_table_offset +
			offsetof(SMU73_Discrete_PmFuses, TdcWaterfallCtl),
			(uint32_t *)&temp, SMC_RAM_END))
		PP_ASSERT_WITH_CODE(false,
				"Attempt to read PmFuses.DW6 (SviLoadLineEn) from SMC Failed!",
				return -EINVAL);
	else {
		smu_data->power_tune_table.TdcWaterfallCtl = defaults->TdcWaterfallCtl;
		smu_data->power_tune_table.LPMLTemperatureMin =
				(uint8_t)((temp >> 16) & 0xff);
		smu_data->power_tune_table.LPMLTemperatureMax =
				(uint8_t)((temp >> 8) & 0xff);
		smu_data->power_tune_table.Reserved = (uint8_t)(temp & 0xff);
	}
	return 0;
}

static int fiji_populate_temperature_scaler(struct pp_hwmgr *hwmgr)
{
	int i;
	struct fiji_smumgr *smu_data = (struct fiji_smumgr *)(hwmgr->smu_backend);

	/* Currently not used. Set all to zero. */
	for (i = 0; i < 16; i++)
		smu_data->power_tune_table.LPMLTemperatureScaler[i] = 0;

	return 0;
}

static int fiji_populate_fuzzy_fan(struct pp_hwmgr *hwmgr)
{
	struct fiji_smumgr *smu_data = (struct fiji_smumgr *)(hwmgr->smu_backend);

	if ((hwmgr->thermal_controller.advanceFanControlParameters.
			usFanOutputSensitivity & (1 << 15)) ||
			0 == hwmgr->thermal_controller.advanceFanControlParameters.
			usFanOutputSensitivity)
		hwmgr->thermal_controller.advanceFanControlParameters.
		usFanOutputSensitivity = hwmgr->thermal_controller.
			advanceFanControlParameters.usDefaultFanOutputSensitivity;

	smu_data->power_tune_table.FuzzyFan_PwmSetDelta =
			PP_HOST_TO_SMC_US(hwmgr->thermal_controller.
					advanceFanControlParameters.usFanOutputSensitivity);
	return 0;
}

static int fiji_populate_gnb_lpml(struct pp_hwmgr *hwmgr)
{
	int i;
	struct fiji_smumgr *smu_data = (struct fiji_smumgr *)(hwmgr->smu_backend);

	/* Currently not used. Set all to zero. */
	for (i = 0; i < 16; i++)
		smu_data->power_tune_table.GnbLPML[i] = 0;

	return 0;
}

static int fiji_populate_bapm_vddc_base_leakage_sidd(struct pp_hwmgr *hwmgr)
{
	struct fiji_smumgr *smu_data = (struct fiji_smumgr *)(hwmgr->smu_backend);
	struct phm_ppt_v1_information *table_info =
			(struct phm_ppt_v1_information *)(hwmgr->pptable);
	uint16_t HiSidd = smu_data->power_tune_table.BapmVddCBaseLeakageHiSidd;
	uint16_t LoSidd = smu_data->power_tune_table.BapmVddCBaseLeakageLoSidd;
	struct phm_cac_tdp_table *cac_table = table_info->cac_dtp_table;

	HiSidd = (uint16_t)(cac_table->usHighCACLeakage / 100 * 256);
	LoSidd = (uint16_t)(cac_table->usLowCACLeakage / 100 * 256);

	smu_data->power_tune_table.BapmVddCBaseLeakageHiSidd =
			CONVERT_FROM_HOST_TO_SMC_US(HiSidd);
	smu_data->power_tune_table.BapmVddCBaseLeakageLoSidd =
			CONVERT_FROM_HOST_TO_SMC_US(LoSidd);

	return 0;
}

static int fiji_populate_pm_fuses(struct pp_hwmgr *hwmgr)
{
	uint32_t pm_fuse_table_offset;
	struct fiji_smumgr *smu_data = (struct fiji_smumgr *)(hwmgr->smu_backend);

	if (phm_cap_enabled(hwmgr->platform_descriptor.platformCaps,
			PHM_PlatformCaps_PowerContainment)) {
		if (smu7_read_smc_sram_dword(hwmgr,
				SMU7_FIRMWARE_HEADER_LOCATION +
				offsetof(SMU73_Firmware_Header, PmFuseTable),
				&pm_fuse_table_offset, SMC_RAM_END))
			PP_ASSERT_WITH_CODE(false,
					"Attempt to get pm_fuse_table_offset Failed!",
					return -EINVAL);

		/* DW6 */
		if (fiji_populate_svi_load_line(hwmgr))
			PP_ASSERT_WITH_CODE(false,
					"Attempt to populate SviLoadLine Failed!",
					return -EINVAL);
		/* DW7 */
		if (fiji_populate_tdc_limit(hwmgr))
			PP_ASSERT_WITH_CODE(false,
					"Attempt to populate TDCLimit Failed!", return -EINVAL);
		/* DW8 */
		if (fiji_populate_dw8(hwmgr, pm_fuse_table_offset))
			PP_ASSERT_WITH_CODE(false,
					"Attempt to populate TdcWaterfallCtl, "
					"LPMLTemperature Min and Max Failed!",
					return -EINVAL);

		/* DW9-DW12 */
		if (0 != fiji_populate_temperature_scaler(hwmgr))
			PP_ASSERT_WITH_CODE(false,
					"Attempt to populate LPMLTemperatureScaler Failed!",
					return -EINVAL);

		/* DW13-DW14 */
		if (fiji_populate_fuzzy_fan(hwmgr))
			PP_ASSERT_WITH_CODE(false,
					"Attempt to populate Fuzzy Fan Control parameters Failed!",
					return -EINVAL);

		/* DW15-DW18 */
		if (fiji_populate_gnb_lpml(hwmgr))
			PP_ASSERT_WITH_CODE(false,
					"Attempt to populate GnbLPML Failed!",
					return -EINVAL);

		/* DW20 */
		if (fiji_populate_bapm_vddc_base_leakage_sidd(hwmgr))
			PP_ASSERT_WITH_CODE(false,
					"Attempt to populate BapmVddCBaseLeakage Hi and Lo "
					"Sidd Failed!", return -EINVAL);

		if (smu7_copy_bytes_to_smc(hwmgr, pm_fuse_table_offset,
				(uint8_t *)&smu_data->power_tune_table,
				sizeof(struct SMU73_Discrete_PmFuses), SMC_RAM_END))
			PP_ASSERT_WITH_CODE(false,
					"Attempt to download PmFuseTable Failed!",
					return -EINVAL);
	}
	return 0;
}

static int fiji_populate_cac_table(struct pp_hwmgr *hwmgr,
		struct SMU73_Discrete_DpmTable *table)
{
	uint32_t count;
	uint8_t index;
	struct smu7_hwmgr *data = (struct smu7_hwmgr *)(hwmgr->backend);
	struct phm_ppt_v1_information *table_info =
			(struct phm_ppt_v1_information *)(hwmgr->pptable);
	struct phm_ppt_v1_voltage_lookup_table *lookup_table =
			table_info->vddc_lookup_table;
	/* tables is already swapped, so in order to use the value from it,
	 * we need to swap it back.
	 * We are populating vddc CAC data to BapmVddc table
	 * in split and merged mode
	 */

	for (count = 0; count < lookup_table->count; count++) {
		index = phm_get_voltage_index(lookup_table,
				data->vddc_voltage_table.entries[count].value);
		table->BapmVddcVidLoSidd[count] =
			convert_to_vid(lookup_table->entries[index].us_cac_low);
		table->BapmVddcVidHiSidd[count] =
			convert_to_vid(lookup_table->entries[index].us_cac_high);
	}

	return 0;
}

static int fiji_populate_smc_voltage_tables(struct pp_hwmgr *hwmgr,
		struct SMU73_Discrete_DpmTable *table)
{
	int result;

	result = fiji_populate_cac_table(hwmgr, table);
	PP_ASSERT_WITH_CODE(0 == result,
			"can not populate CAC voltage tables to SMC",
			return -EINVAL);

	return 0;
}

static int fiji_populate_ulv_level(struct pp_hwmgr *hwmgr,
		struct SMU73_Discrete_Ulv *state)
{
	int result = 0;

	struct phm_ppt_v1_information *table_info =
			(struct phm_ppt_v1_information *)(hwmgr->pptable);

	state->CcPwrDynRm = 0;
	state->CcPwrDynRm1 = 0;

	state->VddcOffset = (uint16_t) table_info->us_ulv_voltage_offset;
	state->VddcOffsetVid = (uint8_t)(table_info->us_ulv_voltage_offset *
			VOLTAGE_VID_OFFSET_SCALE2 / VOLTAGE_VID_OFFSET_SCALE1);

	state->VddcPhase = 1;

	if (!result) {
		CONVERT_FROM_HOST_TO_SMC_UL(state->CcPwrDynRm);
		CONVERT_FROM_HOST_TO_SMC_UL(state->CcPwrDynRm1);
		CONVERT_FROM_HOST_TO_SMC_US(state->VddcOffset);
	}
	return result;
}

static int fiji_populate_ulv_state(struct pp_hwmgr *hwmgr,
		struct SMU73_Discrete_DpmTable *table)
{
	return fiji_populate_ulv_level(hwmgr, &table->Ulv);
}

static int fiji_populate_smc_link_level(struct pp_hwmgr *hwmgr,
		struct SMU73_Discrete_DpmTable *table)
{
	struct smu7_hwmgr *data = (struct smu7_hwmgr *)(hwmgr->backend);
	struct smu7_dpm_table *dpm_table = &data->dpm_table;
	struct fiji_smumgr *smu_data = (struct fiji_smumgr *)(hwmgr->smu_backend);
	int i;

	/* Index (dpm_table->pcie_speed_table.count)
	 * is reserved for PCIE boot level. */
	for (i = 0; i <= dpm_table->pcie_speed_table.count; i++) {
		table->LinkLevel[i].PcieGenSpeed  =
				(uint8_t)dpm_table->pcie_speed_table.dpm_levels[i].value;
		table->LinkLevel[i].PcieLaneCount = (uint8_t)encode_pcie_lane_width(
				dpm_table->pcie_speed_table.dpm_levels[i].param1);
		table->LinkLevel[i].EnabledForActivity = 1;
		table->LinkLevel[i].SPC = (uint8_t)(data->pcie_spc_cap & 0xff);
		table->LinkLevel[i].DownThreshold = PP_HOST_TO_SMC_UL(5);
		table->LinkLevel[i].UpThreshold = PP_HOST_TO_SMC_UL(30);
	}

	smu_data->smc_state_table.LinkLevelCount =
			(uint8_t)dpm_table->pcie_speed_table.count;
	data->dpm_level_enable_mask.pcie_dpm_enable_mask =
			phm_get_dpm_level_enable_mask_value(&dpm_table->pcie_speed_table);

	return 0;
}

static int fiji_calculate_sclk_params(struct pp_hwmgr *hwmgr,
		uint32_t clock, struct SMU73_Discrete_GraphicsLevel *sclk)
{
	const struct smu7_hwmgr *data = (struct smu7_hwmgr *)(hwmgr->backend);
	struct pp_atomctrl_clock_dividers_vi dividers;
	uint32_t spll_func_cntl            = data->clock_registers.vCG_SPLL_FUNC_CNTL;
	uint32_t spll_func_cntl_3          = data->clock_registers.vCG_SPLL_FUNC_CNTL_3;
	uint32_t spll_func_cntl_4          = data->clock_registers.vCG_SPLL_FUNC_CNTL_4;
	uint32_t cg_spll_spread_spectrum   = data->clock_registers.vCG_SPLL_SPREAD_SPECTRUM;
	uint32_t cg_spll_spread_spectrum_2 = data->clock_registers.vCG_SPLL_SPREAD_SPECTRUM_2;
	uint32_t ref_clock;
	uint32_t ref_divider;
	uint32_t fbdiv;
	int result;

	/* get the engine clock dividers for this clock value */
	result = atomctrl_get_engine_pll_dividers_vi(hwmgr, clock,  &dividers);

	PP_ASSERT_WITH_CODE(result == 0,
			"Error retrieving Engine Clock dividers from VBIOS.",
			return result);

	/* To get FBDIV we need to multiply this by 16384 and divide it by Fref. */
	ref_clock = atomctrl_get_reference_clock(hwmgr);
	ref_divider = 1 + dividers.uc_pll_ref_div;

	/* low 14 bits is fraction and high 12 bits is divider */
	fbdiv = dividers.ul_fb_div.ul_fb_divider & 0x3FFFFFF;

	/* SPLL_FUNC_CNTL setup */
	spll_func_cntl = PHM_SET_FIELD(spll_func_cntl, CG_SPLL_FUNC_CNTL,
			SPLL_REF_DIV, dividers.uc_pll_ref_div);
	spll_func_cntl = PHM_SET_FIELD(spll_func_cntl, CG_SPLL_FUNC_CNTL,
			SPLL_PDIV_A,  dividers.uc_pll_post_div);

	/* SPLL_FUNC_CNTL_3 setup*/
	spll_func_cntl_3 = PHM_SET_FIELD(spll_func_cntl_3, CG_SPLL_FUNC_CNTL_3,
			SPLL_FB_DIV, fbdiv);

	/* set to use fractional accumulation*/
	spll_func_cntl_3 = PHM_SET_FIELD(spll_func_cntl_3, CG_SPLL_FUNC_CNTL_3,
			SPLL_DITHEN, 1);

	if (phm_cap_enabled(hwmgr->platform_descriptor.platformCaps,
				PHM_PlatformCaps_EngineSpreadSpectrumSupport)) {
		struct pp_atomctrl_internal_ss_info ssInfo;

		uint32_t vco_freq = clock * dividers.uc_pll_post_div;
		if (!atomctrl_get_engine_clock_spread_spectrum(hwmgr,
				vco_freq, &ssInfo)) {
			/*
			 * ss_info.speed_spectrum_percentage -- in unit of 0.01%
			 * ss_info.speed_spectrum_rate -- in unit of khz
			 *
			 * clks = reference_clock * 10 / (REFDIV + 1) / speed_spectrum_rate / 2
			 */
			uint32_t clk_s = ref_clock * 5 /
					(ref_divider * ssInfo.speed_spectrum_rate);
			/* clkv = 2 * D * fbdiv / NS */
			uint32_t clk_v = 4 * ssInfo.speed_spectrum_percentage *
					fbdiv / (clk_s * 10000);

			cg_spll_spread_spectrum = PHM_SET_FIELD(cg_spll_spread_spectrum,
					CG_SPLL_SPREAD_SPECTRUM, CLKS, clk_s);
			cg_spll_spread_spectrum = PHM_SET_FIELD(cg_spll_spread_spectrum,
					CG_SPLL_SPREAD_SPECTRUM, SSEN, 1);
			cg_spll_spread_spectrum_2 = PHM_SET_FIELD(cg_spll_spread_spectrum_2,
					CG_SPLL_SPREAD_SPECTRUM_2, CLKV, clk_v);
		}
	}

	sclk->SclkFrequency        = clock;
	sclk->CgSpllFuncCntl3      = spll_func_cntl_3;
	sclk->CgSpllFuncCntl4      = spll_func_cntl_4;
	sclk->SpllSpreadSpectrum   = cg_spll_spread_spectrum;
	sclk->SpllSpreadSpectrum2  = cg_spll_spread_spectrum_2;
	sclk->SclkDid              = (uint8_t)dividers.pll_post_divider;

	return 0;
}

static int fiji_populate_single_graphic_level(struct pp_hwmgr *hwmgr,
		uint32_t clock, uint16_t sclk_al_threshold,
		struct SMU73_Discrete_GraphicsLevel *level)
{
	int result;
	/* PP_Clocks minClocks; */
	uint32_t threshold, mvdd;
	struct smu7_hwmgr *data = (struct smu7_hwmgr *)(hwmgr->backend);
	struct phm_ppt_v1_information *table_info =
			(struct phm_ppt_v1_information *)(hwmgr->pptable);

	result = fiji_calculate_sclk_params(hwmgr, clock, level);

	/* populate graphics levels */
	result = fiji_get_dependency_volt_by_clk(hwmgr,
			table_info->vdd_dep_on_sclk, clock,
			(uint32_t *)(&level->MinVoltage), &mvdd);
	PP_ASSERT_WITH_CODE((0 == result),
			"can not find VDDC voltage value for "
			"VDDC engine clock dependency table",
			return result);

	level->SclkFrequency = clock;
	level->ActivityLevel = sclk_al_threshold;
	level->CcPwrDynRm = 0;
	level->CcPwrDynRm1 = 0;
	level->EnabledForActivity = 0;
	level->EnabledForThrottle = 1;
	level->UpHyst = 10;
	level->DownHyst = 0;
	level->VoltageDownHyst = 0;
	level->PowerThrottle = 0;

	threshold = clock * data->fast_watermark_threshold / 100;

	data->display_timing.min_clock_in_sr = hwmgr->display_config.min_core_set_clock_in_sr;

	if (phm_cap_enabled(hwmgr->platform_descriptor.platformCaps, PHM_PlatformCaps_SclkDeepSleep))
		level->DeepSleepDivId = smu7_get_sleep_divider_id_from_clock(clock,
								hwmgr->display_config.min_core_set_clock_in_sr);


	/* Default to slow, highest DPM level will be
	 * set to PPSMC_DISPLAY_WATERMARK_LOW later.
	 */
	level->DisplayWatermark = PPSMC_DISPLAY_WATERMARK_LOW;

	CONVERT_FROM_HOST_TO_SMC_UL(level->MinVoltage);
	CONVERT_FROM_HOST_TO_SMC_UL(level->SclkFrequency);
	CONVERT_FROM_HOST_TO_SMC_US(level->ActivityLevel);
	CONVERT_FROM_HOST_TO_SMC_UL(level->CgSpllFuncCntl3);
	CONVERT_FROM_HOST_TO_SMC_UL(level->CgSpllFuncCntl4);
	CONVERT_FROM_HOST_TO_SMC_UL(level->SpllSpreadSpectrum);
	CONVERT_FROM_HOST_TO_SMC_UL(level->SpllSpreadSpectrum2);
	CONVERT_FROM_HOST_TO_SMC_UL(level->CcPwrDynRm);
	CONVERT_FROM_HOST_TO_SMC_UL(level->CcPwrDynRm1);

	return 0;
}

static int fiji_populate_all_graphic_levels(struct pp_hwmgr *hwmgr)
{
	struct smu7_hwmgr *data = (struct smu7_hwmgr *)(hwmgr->backend);
	struct fiji_smumgr *smu_data = (struct fiji_smumgr *)(hwmgr->smu_backend);

	struct smu7_dpm_table *dpm_table = &data->dpm_table;
	struct phm_ppt_v1_information *table_info =
			(struct phm_ppt_v1_information *)(hwmgr->pptable);
	struct phm_ppt_v1_pcie_table *pcie_table = table_info->pcie_table;
	uint8_t pcie_entry_cnt = (uint8_t) data->dpm_table.pcie_speed_table.count;
	int result = 0;
	uint32_t array = smu_data->smu7_data.dpm_table_start +
			offsetof(SMU73_Discrete_DpmTable, GraphicsLevel);
	uint32_t array_size = sizeof(struct SMU73_Discrete_GraphicsLevel) *
			SMU73_MAX_LEVELS_GRAPHICS;
	struct SMU73_Discrete_GraphicsLevel *levels =
			smu_data->smc_state_table.GraphicsLevel;
	uint32_t i, max_entry;
	uint8_t hightest_pcie_level_enabled = 0,
			lowest_pcie_level_enabled = 0,
			mid_pcie_level_enabled = 0,
			count = 0;

	for (i = 0; i < dpm_table->sclk_table.count; i++) {
		result = fiji_populate_single_graphic_level(hwmgr,
				dpm_table->sclk_table.dpm_levels[i].value,
				(uint16_t)smu_data->activity_target[i],
				&levels[i]);
		if (result)
			return result;

		/* Making sure only DPM level 0-1 have Deep Sleep Div ID populated. */
		if (i > 1)
			levels[i].DeepSleepDivId = 0;
	}

	/* Only enable level 0 for now.*/
	levels[0].EnabledForActivity = 1;

	/* set highest level watermark to high */
	levels[dpm_table->sclk_table.count - 1].DisplayWatermark =
			PPSMC_DISPLAY_WATERMARK_HIGH;

	smu_data->smc_state_table.GraphicsDpmLevelCount =
			(uint8_t)dpm_table->sclk_table.count;
	data->dpm_level_enable_mask.sclk_dpm_enable_mask =
			phm_get_dpm_level_enable_mask_value(&dpm_table->sclk_table);

	if (pcie_table != NULL) {
		PP_ASSERT_WITH_CODE((1 <= pcie_entry_cnt),
				"There must be 1 or more PCIE levels defined in PPTable.",
				return -EINVAL);
		max_entry = pcie_entry_cnt - 1;
		for (i = 0; i < dpm_table->sclk_table.count; i++)
			levels[i].pcieDpmLevel =
					(uint8_t) ((i < max_entry) ? i : max_entry);
	} else {
		while (data->dpm_level_enable_mask.pcie_dpm_enable_mask &&
				((data->dpm_level_enable_mask.pcie_dpm_enable_mask &
						(1 << (hightest_pcie_level_enabled + 1))) != 0))
			hightest_pcie_level_enabled++;

		while (data->dpm_level_enable_mask.pcie_dpm_enable_mask &&
				((data->dpm_level_enable_mask.pcie_dpm_enable_mask &
						(1 << lowest_pcie_level_enabled)) == 0))
			lowest_pcie_level_enabled++;

		while ((count < hightest_pcie_level_enabled) &&
				((data->dpm_level_enable_mask.pcie_dpm_enable_mask &
						(1 << (lowest_pcie_level_enabled + 1 + count))) == 0))
			count++;

		mid_pcie_level_enabled = (lowest_pcie_level_enabled + 1 + count) <
				hightest_pcie_level_enabled ?
						(lowest_pcie_level_enabled + 1 + count) :
						hightest_pcie_level_enabled;

		/* set pcieDpmLevel to hightest_pcie_level_enabled */
		for (i = 2; i < dpm_table->sclk_table.count; i++)
			levels[i].pcieDpmLevel = hightest_pcie_level_enabled;

		/* set pcieDpmLevel to lowest_pcie_level_enabled */
		levels[0].pcieDpmLevel = lowest_pcie_level_enabled;

		/* set pcieDpmLevel to mid_pcie_level_enabled */
		levels[1].pcieDpmLevel = mid_pcie_level_enabled;
	}
	/* level count will send to smc once at init smc table and never change */
	result = smu7_copy_bytes_to_smc(hwmgr, array, (uint8_t *)levels,
			(uint32_t)array_size, SMC_RAM_END);

	return result;
}


/**
 * MCLK Frequency Ratio
 * SEQ_CG_RESP  Bit[31:24] - 0x0
 * Bit[27:24] \96 DDR3 Frequency ratio
 * 0x0 <= 100MHz,       450 < 0x8 <= 500MHz
 * 100 < 0x1 <= 150MHz,       500 < 0x9 <= 550MHz
 * 150 < 0x2 <= 200MHz,       550 < 0xA <= 600MHz
 * 200 < 0x3 <= 250MHz,       600 < 0xB <= 650MHz
 * 250 < 0x4 <= 300MHz,       650 < 0xC <= 700MHz
 * 300 < 0x5 <= 350MHz,       700 < 0xD <= 750MHz
 * 350 < 0x6 <= 400MHz,       750 < 0xE <= 800MHz
 * 400 < 0x7 <= 450MHz,       800 < 0xF
 */
static uint8_t fiji_get_mclk_frequency_ratio(uint32_t mem_clock)
{
	if (mem_clock <= 10000)
		return 0x0;
	if (mem_clock <= 15000)
		return 0x1;
	if (mem_clock <= 20000)
		return 0x2;
	if (mem_clock <= 25000)
		return 0x3;
	if (mem_clock <= 30000)
		return 0x4;
	if (mem_clock <= 35000)
		return 0x5;
	if (mem_clock <= 40000)
		return 0x6;
	if (mem_clock <= 45000)
		return 0x7;
	if (mem_clock <= 50000)
		return 0x8;
	if (mem_clock <= 55000)
		return 0x9;
	if (mem_clock <= 60000)
		return 0xa;
	if (mem_clock <= 65000)
		return 0xb;
	if (mem_clock <= 70000)
		return 0xc;
	if (mem_clock <= 75000)
		return 0xd;
	if (mem_clock <= 80000)
		return 0xe;
	/* mem_clock > 800MHz */
	return 0xf;
}

static int fiji_calculate_mclk_params(struct pp_hwmgr *hwmgr,
    uint32_t clock, struct SMU73_Discrete_MemoryLevel *mclk)
{
	struct pp_atomctrl_memory_clock_param mem_param;
	int result;

	result = atomctrl_get_memory_pll_dividers_vi(hwmgr, clock, &mem_param);
	PP_ASSERT_WITH_CODE((0 == result),
			"Failed to get Memory PLL Dividers.",
			);

	/* Save the result data to outpupt memory level structure */
	mclk->MclkFrequency   = clock;
	mclk->MclkDivider     = (uint8_t)mem_param.mpll_post_divider;
	mclk->FreqRange       = fiji_get_mclk_frequency_ratio(clock);

	return result;
}

static int fiji_populate_single_memory_level(struct pp_hwmgr *hwmgr,
		uint32_t clock, struct SMU73_Discrete_MemoryLevel *mem_level)
{
	struct smu7_hwmgr *data = (struct smu7_hwmgr *)(hwmgr->backend);
	struct phm_ppt_v1_information *table_info =
			(struct phm_ppt_v1_information *)(hwmgr->pptable);
	int result = 0;
	uint32_t mclk_stutter_mode_threshold = 60000;

	if (table_info->vdd_dep_on_mclk) {
		result = fiji_get_dependency_volt_by_clk(hwmgr,
				table_info->vdd_dep_on_mclk, clock,
				(uint32_t *)(&mem_level->MinVoltage), &mem_level->MinMvdd);
		PP_ASSERT_WITH_CODE((0 == result),
				"can not find MinVddc voltage value from memory "
				"VDDC voltage dependency table", return result);
	}

	mem_level->EnabledForThrottle = 1;
	mem_level->EnabledForActivity = 0;
	mem_level->UpHyst = 0;
	mem_level->DownHyst = 100;
	mem_level->VoltageDownHyst = 0;
	mem_level->ActivityLevel = (uint16_t)data->mclk_activity_target;
	mem_level->StutterEnable = false;

	mem_level->DisplayWatermark = PPSMC_DISPLAY_WATERMARK_LOW;

	/* enable stutter mode if all the follow condition applied
	 * PECI_GetNumberOfActiveDisplays(hwmgr->pPECI,
	 * &(data->DisplayTiming.numExistingDisplays));
	 */
	data->display_timing.num_existing_displays = 1;

	if (mclk_stutter_mode_threshold &&
		(clock <= mclk_stutter_mode_threshold) &&
		(!data->is_uvd_enabled) &&
		(PHM_READ_FIELD(hwmgr->device, DPG_PIPE_STUTTER_CONTROL,
				STUTTER_ENABLE) & 0x1))
		mem_level->StutterEnable = true;

	result = fiji_calculate_mclk_params(hwmgr, clock, mem_level);
	if (!result) {
		CONVERT_FROM_HOST_TO_SMC_UL(mem_level->MinMvdd);
		CONVERT_FROM_HOST_TO_SMC_UL(mem_level->MclkFrequency);
		CONVERT_FROM_HOST_TO_SMC_US(mem_level->ActivityLevel);
		CONVERT_FROM_HOST_TO_SMC_UL(mem_level->MinVoltage);
	}
	return result;
}

static int fiji_populate_all_memory_levels(struct pp_hwmgr *hwmgr)
{
	struct smu7_hwmgr *data = (struct smu7_hwmgr *)(hwmgr->backend);
	struct fiji_smumgr *smu_data = (struct fiji_smumgr *)(hwmgr->smu_backend);
	struct smu7_dpm_table *dpm_table = &data->dpm_table;
	int result;
	/* populate MCLK dpm table to SMU7 */
	uint32_t array = smu_data->smu7_data.dpm_table_start +
			offsetof(SMU73_Discrete_DpmTable, MemoryLevel);
	uint32_t array_size = sizeof(SMU73_Discrete_MemoryLevel) *
			SMU73_MAX_LEVELS_MEMORY;
	struct SMU73_Discrete_MemoryLevel *levels =
			smu_data->smc_state_table.MemoryLevel;
	uint32_t i;

	for (i = 0; i < dpm_table->mclk_table.count; i++) {
		PP_ASSERT_WITH_CODE((0 != dpm_table->mclk_table.dpm_levels[i].value),
				"can not populate memory level as memory clock is zero",
				return -EINVAL);
		result = fiji_populate_single_memory_level(hwmgr,
				dpm_table->mclk_table.dpm_levels[i].value,
				&levels[i]);
		if (result)
			return result;
	}

	/* Only enable level 0 for now. */
	levels[0].EnabledForActivity = 1;

	/* in order to prevent MC activity from stutter mode to push DPM up.
	 * the UVD change complements this by putting the MCLK in
	 * a higher state by default such that we are not effected by
	 * up threshold or and MCLK DPM latency.
	 */
	levels[0].ActivityLevel = (uint16_t)data->mclk_dpm0_activity_target;
	CONVERT_FROM_HOST_TO_SMC_US(levels[0].ActivityLevel);

	smu_data->smc_state_table.MemoryDpmLevelCount =
			(uint8_t)dpm_table->mclk_table.count;
	data->dpm_level_enable_mask.mclk_dpm_enable_mask =
			phm_get_dpm_level_enable_mask_value(&dpm_table->mclk_table);
	/* set highest level watermark to high */
	levels[dpm_table->mclk_table.count - 1].DisplayWatermark =
			PPSMC_DISPLAY_WATERMARK_HIGH;

	/* level count will send to smc once at init smc table and never change */
	result = smu7_copy_bytes_to_smc(hwmgr, array, (uint8_t *)levels,
			(uint32_t)array_size, SMC_RAM_END);

	return result;
}

static int fiji_populate_mvdd_value(struct pp_hwmgr *hwmgr,
		uint32_t mclk, SMIO_Pattern *smio_pat)
{
	const struct smu7_hwmgr *data = (struct smu7_hwmgr *)(hwmgr->backend);
	struct phm_ppt_v1_information *table_info =
			(struct phm_ppt_v1_information *)(hwmgr->pptable);
	uint32_t i = 0;

	if (SMU7_VOLTAGE_CONTROL_NONE != data->mvdd_control) {
		/* find mvdd value which clock is more than request */
		for (i = 0; i < table_info->vdd_dep_on_mclk->count; i++) {
			if (mclk <= table_info->vdd_dep_on_mclk->entries[i].clk) {
				smio_pat->Voltage = data->mvdd_voltage_table.entries[i].value;
				break;
			}
		}
		PP_ASSERT_WITH_CODE(i < table_info->vdd_dep_on_mclk->count,
				"MVDD Voltage is outside the supported range.",
				return -EINVAL);
	} else
		return -EINVAL;

	return 0;
}

static int fiji_populate_smc_acpi_level(struct pp_hwmgr *hwmgr,
		SMU73_Discrete_DpmTable *table)
{
	int result = 0;
	const struct smu7_hwmgr *data = (struct smu7_hwmgr *)(hwmgr->backend);
	struct phm_ppt_v1_information *table_info =
			(struct phm_ppt_v1_information *)(hwmgr->pptable);
	struct pp_atomctrl_clock_dividers_vi dividers;
	SMIO_Pattern vol_level;
	uint32_t mvdd;
	uint16_t us_mvdd;
	uint32_t spll_func_cntl    = data->clock_registers.vCG_SPLL_FUNC_CNTL;
	uint32_t spll_func_cntl_2  = data->clock_registers.vCG_SPLL_FUNC_CNTL_2;

	table->ACPILevel.Flags &= ~PPSMC_SWSTATE_FLAG_DC;

	if (!data->sclk_dpm_key_disabled) {
		/* Get MinVoltage and Frequency from DPM0,
		 * already converted to SMC_UL */
		table->ACPILevel.SclkFrequency =
				data->dpm_table.sclk_table.dpm_levels[0].value;
		result = fiji_get_dependency_volt_by_clk(hwmgr,
				table_info->vdd_dep_on_sclk,
				table->ACPILevel.SclkFrequency,
				(uint32_t *)(&table->ACPILevel.MinVoltage), &mvdd);
		PP_ASSERT_WITH_CODE((0 == result),
				"Cannot find ACPI VDDC voltage value " \
				"in Clock Dependency Table",
				);
	} else {
		table->ACPILevel.SclkFrequency =
				data->vbios_boot_state.sclk_bootup_value;
		table->ACPILevel.MinVoltage =
				data->vbios_boot_state.vddc_bootup_value * VOLTAGE_SCALE;
	}

	/* get the engine clock dividers for this clock value */
	result = atomctrl_get_engine_pll_dividers_vi(hwmgr,
			table->ACPILevel.SclkFrequency,  &dividers);
	PP_ASSERT_WITH_CODE(result == 0,
			"Error retrieving Engine Clock dividers from VBIOS.",
			return result);

	table->ACPILevel.SclkDid = (uint8_t)dividers.pll_post_divider;
	table->ACPILevel.DisplayWatermark = PPSMC_DISPLAY_WATERMARK_LOW;
	table->ACPILevel.DeepSleepDivId = 0;

	spll_func_cntl = PHM_SET_FIELD(spll_func_cntl, CG_SPLL_FUNC_CNTL,
			SPLL_PWRON, 0);
	spll_func_cntl = PHM_SET_FIELD(spll_func_cntl, CG_SPLL_FUNC_CNTL,
			SPLL_RESET, 1);
	spll_func_cntl_2 = PHM_SET_FIELD(spll_func_cntl_2, CG_SPLL_FUNC_CNTL_2,
			SCLK_MUX_SEL, 4);

	table->ACPILevel.CgSpllFuncCntl = spll_func_cntl;
	table->ACPILevel.CgSpllFuncCntl2 = spll_func_cntl_2;
	table->ACPILevel.CgSpllFuncCntl3 = data->clock_registers.vCG_SPLL_FUNC_CNTL_3;
	table->ACPILevel.CgSpllFuncCntl4 = data->clock_registers.vCG_SPLL_FUNC_CNTL_4;
	table->ACPILevel.SpllSpreadSpectrum = data->clock_registers.vCG_SPLL_SPREAD_SPECTRUM;
	table->ACPILevel.SpllSpreadSpectrum2 = data->clock_registers.vCG_SPLL_SPREAD_SPECTRUM_2;
	table->ACPILevel.CcPwrDynRm = 0;
	table->ACPILevel.CcPwrDynRm1 = 0;

	CONVERT_FROM_HOST_TO_SMC_UL(table->ACPILevel.Flags);
	CONVERT_FROM_HOST_TO_SMC_UL(table->ACPILevel.SclkFrequency);
	CONVERT_FROM_HOST_TO_SMC_UL(table->ACPILevel.MinVoltage);
	CONVERT_FROM_HOST_TO_SMC_UL(table->ACPILevel.CgSpllFuncCntl);
	CONVERT_FROM_HOST_TO_SMC_UL(table->ACPILevel.CgSpllFuncCntl2);
	CONVERT_FROM_HOST_TO_SMC_UL(table->ACPILevel.CgSpllFuncCntl3);
	CONVERT_FROM_HOST_TO_SMC_UL(table->ACPILevel.CgSpllFuncCntl4);
	CONVERT_FROM_HOST_TO_SMC_UL(table->ACPILevel.SpllSpreadSpectrum);
	CONVERT_FROM_HOST_TO_SMC_UL(table->ACPILevel.SpllSpreadSpectrum2);
	CONVERT_FROM_HOST_TO_SMC_UL(table->ACPILevel.CcPwrDynRm);
	CONVERT_FROM_HOST_TO_SMC_UL(table->ACPILevel.CcPwrDynRm1);

	if (!data->mclk_dpm_key_disabled) {
		/* Get MinVoltage and Frequency from DPM0, already converted to SMC_UL */
		table->MemoryACPILevel.MclkFrequency =
				data->dpm_table.mclk_table.dpm_levels[0].value;
		result = fiji_get_dependency_volt_by_clk(hwmgr,
				table_info->vdd_dep_on_mclk,
				table->MemoryACPILevel.MclkFrequency,
			(uint32_t *)(&table->MemoryACPILevel.MinVoltage), &mvdd);
		PP_ASSERT_WITH_CODE((0 == result),
				"Cannot find ACPI VDDCI voltage value in Clock Dependency Table",
				);
	} else {
		table->MemoryACPILevel.MclkFrequency =
				data->vbios_boot_state.mclk_bootup_value;
		table->MemoryACPILevel.MinVoltage =
				data->vbios_boot_state.vddci_bootup_value * VOLTAGE_SCALE;
	}

	us_mvdd = 0;
	if ((SMU7_VOLTAGE_CONTROL_NONE == data->mvdd_control) ||
			(data->mclk_dpm_key_disabled))
		us_mvdd = data->vbios_boot_state.mvdd_bootup_value;
	else {
		if (!fiji_populate_mvdd_value(hwmgr,
				data->dpm_table.mclk_table.dpm_levels[0].value,
				&vol_level))
			us_mvdd = vol_level.Voltage;
	}

	table->MemoryACPILevel.MinMvdd =
			PP_HOST_TO_SMC_UL(us_mvdd * VOLTAGE_SCALE);

	table->MemoryACPILevel.EnabledForThrottle = 0;
	table->MemoryACPILevel.EnabledForActivity = 0;
	table->MemoryACPILevel.UpHyst = 0;
	table->MemoryACPILevel.DownHyst = 100;
	table->MemoryACPILevel.VoltageDownHyst = 0;
	table->MemoryACPILevel.ActivityLevel =
			PP_HOST_TO_SMC_US((uint16_t)data->mclk_activity_target);

	table->MemoryACPILevel.StutterEnable = false;
	CONVERT_FROM_HOST_TO_SMC_UL(table->MemoryACPILevel.MclkFrequency);
	CONVERT_FROM_HOST_TO_SMC_UL(table->MemoryACPILevel.MinVoltage);

	return result;
}

static int fiji_populate_smc_vce_level(struct pp_hwmgr *hwmgr,
		SMU73_Discrete_DpmTable *table)
{
	int result = -EINVAL;
	uint8_t count;
	struct pp_atomctrl_clock_dividers_vi dividers;
	struct phm_ppt_v1_information *table_info =
			(struct phm_ppt_v1_information *)(hwmgr->pptable);
	struct phm_ppt_v1_mm_clock_voltage_dependency_table *mm_table =
			table_info->mm_dep_table;

	table->VceLevelCount = (uint8_t)(mm_table->count);
	table->VceBootLevel = 0;

	for (count = 0; count < table->VceLevelCount; count++) {
		table->VceLevel[count].Frequency = mm_table->entries[count].eclk;
		table->VceLevel[count].MinVoltage = 0;
		table->VceLevel[count].MinVoltage |=
				(mm_table->entries[count].vddc * VOLTAGE_SCALE) << VDDC_SHIFT;
		table->VceLevel[count].MinVoltage |=
				((mm_table->entries[count].vddc - VDDC_VDDCI_DELTA) *
						VOLTAGE_SCALE) << VDDCI_SHIFT;
		table->VceLevel[count].MinVoltage |= 1 << PHASES_SHIFT;

		/*retrieve divider value for VBIOS */
		result = atomctrl_get_dfs_pll_dividers_vi(hwmgr,
				table->VceLevel[count].Frequency, &dividers);
		PP_ASSERT_WITH_CODE((0 == result),
				"can not find divide id for VCE engine clock",
				return result);

		table->VceLevel[count].Divider = (uint8_t)dividers.pll_post_divider;

		CONVERT_FROM_HOST_TO_SMC_UL(table->VceLevel[count].Frequency);
		CONVERT_FROM_HOST_TO_SMC_UL(table->VceLevel[count].MinVoltage);
	}
	return result;
}

static int fiji_populate_smc_acp_level(struct pp_hwmgr *hwmgr,
		SMU73_Discrete_DpmTable *table)
{
	int result = -EINVAL;
	uint8_t count;
	struct pp_atomctrl_clock_dividers_vi dividers;
	struct phm_ppt_v1_information *table_info =
			(struct phm_ppt_v1_information *)(hwmgr->pptable);
	struct phm_ppt_v1_mm_clock_voltage_dependency_table *mm_table =
			table_info->mm_dep_table;

	table->AcpLevelCount = (uint8_t)(mm_table->count);
	table->AcpBootLevel = 0;

	for (count = 0; count < table->AcpLevelCount; count++) {
		table->AcpLevel[count].Frequency = mm_table->entries[count].aclk;
		table->AcpLevel[count].MinVoltage |= (mm_table->entries[count].vddc *
				VOLTAGE_SCALE) << VDDC_SHIFT;
		table->AcpLevel[count].MinVoltage |= ((mm_table->entries[count].vddc -
				VDDC_VDDCI_DELTA) * VOLTAGE_SCALE) << VDDCI_SHIFT;
		table->AcpLevel[count].MinVoltage |= 1 << PHASES_SHIFT;

		/* retrieve divider value for VBIOS */
		result = atomctrl_get_dfs_pll_dividers_vi(hwmgr,
				table->AcpLevel[count].Frequency, &dividers);
		PP_ASSERT_WITH_CODE((0 == result),
				"can not find divide id for engine clock", return result);

		table->AcpLevel[count].Divider = (uint8_t)dividers.pll_post_divider;

		CONVERT_FROM_HOST_TO_SMC_UL(table->AcpLevel[count].Frequency);
		CONVERT_FROM_HOST_TO_SMC_UL(table->AcpLevel[count].MinVoltage);
	}
	return result;
}

static int fiji_populate_smc_samu_level(struct pp_hwmgr *hwmgr,
		SMU73_Discrete_DpmTable *table)
{
	int result = -EINVAL;
	uint8_t count;
	struct pp_atomctrl_clock_dividers_vi dividers;
	struct phm_ppt_v1_information *table_info =
			(struct phm_ppt_v1_information *)(hwmgr->pptable);
	struct phm_ppt_v1_mm_clock_voltage_dependency_table *mm_table =
			table_info->mm_dep_table;

	table->SamuBootLevel = 0;
	table->SamuLevelCount = (uint8_t)(mm_table->count);

	for (count = 0; count < table->SamuLevelCount; count++) {
		/* not sure whether we need evclk or not */
		table->SamuLevel[count].MinVoltage = 0;
		table->SamuLevel[count].Frequency = mm_table->entries[count].samclock;
		table->SamuLevel[count].MinVoltage |= (mm_table->entries[count].vddc *
				VOLTAGE_SCALE) << VDDC_SHIFT;
		table->SamuLevel[count].MinVoltage |= ((mm_table->entries[count].vddc -
				VDDC_VDDCI_DELTA) * VOLTAGE_SCALE) << VDDCI_SHIFT;
		table->SamuLevel[count].MinVoltage |= 1 << PHASES_SHIFT;

		/* retrieve divider value for VBIOS */
		result = atomctrl_get_dfs_pll_dividers_vi(hwmgr,
				table->SamuLevel[count].Frequency, &dividers);
		PP_ASSERT_WITH_CODE((0 == result),
				"can not find divide id for samu clock", return result);

		table->SamuLevel[count].Divider = (uint8_t)dividers.pll_post_divider;

		CONVERT_FROM_HOST_TO_SMC_UL(table->SamuLevel[count].Frequency);
		CONVERT_FROM_HOST_TO_SMC_UL(table->SamuLevel[count].MinVoltage);
	}
	return result;
}

static int fiji_populate_memory_timing_parameters(struct pp_hwmgr *hwmgr,
		int32_t eng_clock, int32_t mem_clock,
		struct SMU73_Discrete_MCArbDramTimingTableEntry *arb_regs)
{
	uint32_t dram_timing;
	uint32_t dram_timing2;
	uint32_t burstTime;
	ULONG state, trrds, trrdl;
	int result;

	result = atomctrl_set_engine_dram_timings_rv770(hwmgr,
			eng_clock, mem_clock);
	PP_ASSERT_WITH_CODE(result == 0,
			"Error calling VBIOS to set DRAM_TIMING.", return result);

	dram_timing = cgs_read_register(hwmgr->device, mmMC_ARB_DRAM_TIMING);
	dram_timing2 = cgs_read_register(hwmgr->device, mmMC_ARB_DRAM_TIMING2);
	burstTime = cgs_read_register(hwmgr->device, mmMC_ARB_BURST_TIME);

	state = PHM_GET_FIELD(burstTime, MC_ARB_BURST_TIME, STATE0);
	trrds = PHM_GET_FIELD(burstTime, MC_ARB_BURST_TIME, TRRDS0);
	trrdl = PHM_GET_FIELD(burstTime, MC_ARB_BURST_TIME, TRRDL0);

	arb_regs->McArbDramTiming  = PP_HOST_TO_SMC_UL(dram_timing);
	arb_regs->McArbDramTiming2 = PP_HOST_TO_SMC_UL(dram_timing2);
	arb_regs->McArbBurstTime   = (uint8_t)burstTime;
	arb_regs->TRRDS            = (uint8_t)trrds;
	arb_regs->TRRDL            = (uint8_t)trrdl;

	return 0;
}

static int fiji_program_memory_timing_parameters(struct pp_hwmgr *hwmgr)
{
	struct smu7_hwmgr *data = (struct smu7_hwmgr *)(hwmgr->backend);
	struct fiji_smumgr *smu_data = (struct fiji_smumgr *)(hwmgr->smu_backend);
	struct SMU73_Discrete_MCArbDramTimingTable arb_regs;
	uint32_t i, j;
	int result = 0;

	for (i = 0; i < data->dpm_table.sclk_table.count; i++) {
		for (j = 0; j < data->dpm_table.mclk_table.count; j++) {
			result = fiji_populate_memory_timing_parameters(hwmgr,
					data->dpm_table.sclk_table.dpm_levels[i].value,
					data->dpm_table.mclk_table.dpm_levels[j].value,
					&arb_regs.entries[i][j]);
			if (result)
				break;
		}
	}

	if (!result)
		result = smu7_copy_bytes_to_smc(
				hwmgr,
				smu_data->smu7_data.arb_table_start,
				(uint8_t *)&arb_regs,
				sizeof(SMU73_Discrete_MCArbDramTimingTable),
				SMC_RAM_END);
	return result;
}

static int fiji_populate_smc_uvd_level(struct pp_hwmgr *hwmgr,
		struct SMU73_Discrete_DpmTable *table)
{
	int result = -EINVAL;
	uint8_t count;
	struct pp_atomctrl_clock_dividers_vi dividers;
	struct phm_ppt_v1_information *table_info =
			(struct phm_ppt_v1_information *)(hwmgr->pptable);
	struct phm_ppt_v1_mm_clock_voltage_dependency_table *mm_table =
			table_info->mm_dep_table;

	table->UvdLevelCount = (uint8_t)(mm_table->count);
	table->UvdBootLevel = 0;

	for (count = 0; count < table->UvdLevelCount; count++) {
		table->UvdLevel[count].MinVoltage = 0;
		table->UvdLevel[count].VclkFrequency = mm_table->entries[count].vclk;
		table->UvdLevel[count].DclkFrequency = mm_table->entries[count].dclk;
		table->UvdLevel[count].MinVoltage |= (mm_table->entries[count].vddc *
				VOLTAGE_SCALE) << VDDC_SHIFT;
		table->UvdLevel[count].MinVoltage |= ((mm_table->entries[count].vddc -
				VDDC_VDDCI_DELTA) * VOLTAGE_SCALE) << VDDCI_SHIFT;
		table->UvdLevel[count].MinVoltage |= 1 << PHASES_SHIFT;

		/* retrieve divider value for VBIOS */
		result = atomctrl_get_dfs_pll_dividers_vi(hwmgr,
				table->UvdLevel[count].VclkFrequency, &dividers);
		PP_ASSERT_WITH_CODE((0 == result),
				"can not find divide id for Vclk clock", return result);

		table->UvdLevel[count].VclkDivider = (uint8_t)dividers.pll_post_divider;

		result = atomctrl_get_dfs_pll_dividers_vi(hwmgr,
				table->UvdLevel[count].DclkFrequency, &dividers);
		PP_ASSERT_WITH_CODE((0 == result),
				"can not find divide id for Dclk clock", return result);

		table->UvdLevel[count].DclkDivider = (uint8_t)dividers.pll_post_divider;

		CONVERT_FROM_HOST_TO_SMC_UL(table->UvdLevel[count].VclkFrequency);
		CONVERT_FROM_HOST_TO_SMC_UL(table->UvdLevel[count].DclkFrequency);
		CONVERT_FROM_HOST_TO_SMC_UL(table->UvdLevel[count].MinVoltage);

	}
	return result;
}

static int fiji_populate_smc_boot_level(struct pp_hwmgr *hwmgr,
		struct SMU73_Discrete_DpmTable *table)
{
	int result = 0;
	struct smu7_hwmgr *data = (struct smu7_hwmgr *)(hwmgr->backend);

	table->GraphicsBootLevel = 0;
	table->MemoryBootLevel = 0;

	/* find boot level from dpm table */
	result = phm_find_boot_level(&(data->dpm_table.sclk_table),
			data->vbios_boot_state.sclk_bootup_value,
			(uint32_t *)&(table->GraphicsBootLevel));

	result = phm_find_boot_level(&(data->dpm_table.mclk_table),
			data->vbios_boot_state.mclk_bootup_value,
			(uint32_t *)&(table->MemoryBootLevel));

	table->BootVddc  = data->vbios_boot_state.vddc_bootup_value *
			VOLTAGE_SCALE;
	table->BootVddci = data->vbios_boot_state.vddci_bootup_value *
			VOLTAGE_SCALE;
	table->BootMVdd  = data->vbios_boot_state.mvdd_bootup_value *
			VOLTAGE_SCALE;

	CONVERT_FROM_HOST_TO_SMC_US(table->BootVddc);
	CONVERT_FROM_HOST_TO_SMC_US(table->BootVddci);
	CONVERT_FROM_HOST_TO_SMC_US(table->BootMVdd);

	return 0;
}

static int fiji_populate_smc_initailial_state(struct pp_hwmgr *hwmgr)
{
	struct smu7_hwmgr *data = (struct smu7_hwmgr *)(hwmgr->backend);
	struct fiji_smumgr *smu_data = (struct fiji_smumgr *)(hwmgr->smu_backend);
	struct phm_ppt_v1_information *table_info =
			(struct phm_ppt_v1_information *)(hwmgr->pptable);
	uint8_t count, level;

	count = (uint8_t)(table_info->vdd_dep_on_sclk->count);
	for (level = 0; level < count; level++) {
		if (table_info->vdd_dep_on_sclk->entries[level].clk >=
				data->vbios_boot_state.sclk_bootup_value) {
			smu_data->smc_state_table.GraphicsBootLevel = level;
			break;
		}
	}

	count = (uint8_t)(table_info->vdd_dep_on_mclk->count);
	for (level = 0; level < count; level++) {
		if (table_info->vdd_dep_on_mclk->entries[level].clk >=
				data->vbios_boot_state.mclk_bootup_value) {
			smu_data->smc_state_table.MemoryBootLevel = level;
			break;
		}
	}

	return 0;
}

static int fiji_populate_clock_stretcher_data_table(struct pp_hwmgr *hwmgr)
{
	uint32_t ro, efuse, efuse2, clock_freq, volt_without_cks,
			volt_with_cks, value;
	uint16_t clock_freq_u16;
	struct fiji_smumgr *smu_data = (struct fiji_smumgr *)(hwmgr->smu_backend);
	uint8_t type, i, j, cks_setting, stretch_amount, stretch_amount2,
			volt_offset = 0;
	struct phm_ppt_v1_information *table_info =
			(struct phm_ppt_v1_information *)(hwmgr->pptable);
	struct phm_ppt_v1_clock_voltage_dependency_table *sclk_table =
			table_info->vdd_dep_on_sclk;

	stretch_amount = (uint8_t)table_info->cac_dtp_table->usClockStretchAmount;

	/* Read SMU_Eefuse to read and calculate RO and determine
	 * if the part is SS or FF. if RO >= 1660MHz, part is FF.
	 */
	efuse = cgs_read_ind_register(hwmgr->device, CGS_IND_REG__SMC,
			ixSMU_EFUSE_0 + (146 * 4));
	efuse2 = cgs_read_ind_register(hwmgr->device, CGS_IND_REG__SMC,
			ixSMU_EFUSE_0 + (148 * 4));
	efuse &= 0xFF000000;
	efuse = efuse >> 24;
	efuse2 &= 0xF;

	if (efuse2 == 1)
		ro = (2300 - 1350) * efuse / 255 + 1350;
	else
		ro = (2500 - 1000) * efuse / 255 + 1000;

	if (ro >= 1660)
		type = 0;
	else
		type = 1;

	/* Populate Stretch amount */
	smu_data->smc_state_table.ClockStretcherAmount = stretch_amount;

	/* Populate Sclk_CKS_masterEn0_7 and Sclk_voltageOffset */
	for (i = 0; i < sclk_table->count; i++) {
		smu_data->smc_state_table.Sclk_CKS_masterEn0_7 |=
				sclk_table->entries[i].cks_enable << i;
		volt_without_cks = (uint32_t)((14041 *
			(sclk_table->entries[i].clk/100) / 10000 + 3571 + 75 - ro) * 1000 /
			(4026 - (13924 * (sclk_table->entries[i].clk/100) / 10000)));
		volt_with_cks = (uint32_t)((13946 *
			(sclk_table->entries[i].clk/100) / 10000 + 3320 + 45 - ro) * 1000 /
			(3664 - (11454 * (sclk_table->entries[i].clk/100) / 10000)));
		if (volt_without_cks >= volt_with_cks)
			volt_offset = (uint8_t)(((volt_without_cks - volt_with_cks +
					sclk_table->entries[i].cks_voffset) * 100 / 625) + 1);
		smu_data->smc_state_table.Sclk_voltageOffset[i] = volt_offset;
	}

	PHM_WRITE_INDIRECT_FIELD(hwmgr->device, CGS_IND_REG__SMC, PWR_CKS_ENABLE,
			STRETCH_ENABLE, 0x0);
	PHM_WRITE_INDIRECT_FIELD(hwmgr->device, CGS_IND_REG__SMC, PWR_CKS_ENABLE,
			masterReset, 0x1);
	PHM_WRITE_INDIRECT_FIELD(hwmgr->device, CGS_IND_REG__SMC, PWR_CKS_ENABLE,
			staticEnable, 0x1);
	PHM_WRITE_INDIRECT_FIELD(hwmgr->device, CGS_IND_REG__SMC, PWR_CKS_ENABLE,
			masterReset, 0x0);

	/* Populate CKS Lookup Table */
	if (stretch_amount == 1 || stretch_amount == 2 || stretch_amount == 5)
		stretch_amount2 = 0;
	else if (stretch_amount == 3 || stretch_amount == 4)
		stretch_amount2 = 1;
	else {
		phm_cap_unset(hwmgr->platform_descriptor.platformCaps,
				PHM_PlatformCaps_ClockStretcher);
		PP_ASSERT_WITH_CODE(false,
				"Stretch Amount in PPTable not supported\n",
				return -EINVAL);
	}

	value = cgs_read_ind_register(hwmgr->device, CGS_IND_REG__SMC,
			ixPWR_CKS_CNTL);
	value &= 0xFFC2FF87;
	smu_data->smc_state_table.CKS_LOOKUPTable.CKS_LOOKUPTableEntry[0].minFreq =
			fiji_clock_stretcher_lookup_table[stretch_amount2][0];
	smu_data->smc_state_table.CKS_LOOKUPTable.CKS_LOOKUPTableEntry[0].maxFreq =
			fiji_clock_stretcher_lookup_table[stretch_amount2][1];
	clock_freq_u16 = (uint16_t)(PP_SMC_TO_HOST_UL(smu_data->smc_state_table.
			GraphicsLevel[smu_data->smc_state_table.GraphicsDpmLevelCount - 1].
			SclkFrequency) / 100);
	if (fiji_clock_stretcher_lookup_table[stretch_amount2][0] <
			clock_freq_u16 &&
	    fiji_clock_stretcher_lookup_table[stretch_amount2][1] >
			clock_freq_u16) {
		/* Program PWR_CKS_CNTL. CKS_USE_FOR_LOW_FREQ */
		value |= (fiji_clock_stretcher_lookup_table[stretch_amount2][3]) << 16;
		/* Program PWR_CKS_CNTL. CKS_LDO_REFSEL */
		value |= (fiji_clock_stretcher_lookup_table[stretch_amount2][2]) << 18;
		/* Program PWR_CKS_CNTL. CKS_STRETCH_AMOUNT */
		value |= (fiji_clock_stretch_amount_conversion
				[fiji_clock_stretcher_lookup_table[stretch_amount2][3]]
				 [stretch_amount]) << 3;
	}
	CONVERT_FROM_HOST_TO_SMC_US(smu_data->smc_state_table.CKS_LOOKUPTable.
			CKS_LOOKUPTableEntry[0].minFreq);
	CONVERT_FROM_HOST_TO_SMC_US(smu_data->smc_state_table.CKS_LOOKUPTable.
			CKS_LOOKUPTableEntry[0].maxFreq);
	smu_data->smc_state_table.CKS_LOOKUPTable.CKS_LOOKUPTableEntry[0].setting =
			fiji_clock_stretcher_lookup_table[stretch_amount2][2] & 0x7F;
	smu_data->smc_state_table.CKS_LOOKUPTable.CKS_LOOKUPTableEntry[0].setting |=
			(fiji_clock_stretcher_lookup_table[stretch_amount2][3]) << 7;

	cgs_write_ind_register(hwmgr->device, CGS_IND_REG__SMC,
			ixPWR_CKS_CNTL, value);

	/* Populate DDT Lookup Table */
	for (i = 0; i < 4; i++) {
		/* Assign the minimum and maximum VID stored
		 * in the last row of Clock Stretcher Voltage Table.
		 */
		smu_data->smc_state_table.ClockStretcherDataTable.
		ClockStretcherDataTableEntry[i].minVID =
				(uint8_t) fiji_clock_stretcher_ddt_table[type][i][2];
		smu_data->smc_state_table.ClockStretcherDataTable.
		ClockStretcherDataTableEntry[i].maxVID =
				(uint8_t) fiji_clock_stretcher_ddt_table[type][i][3];
		/* Loop through each SCLK and check the frequency
		 * to see if it lies within the frequency for clock stretcher.
		 */
		for (j = 0; j < smu_data->smc_state_table.GraphicsDpmLevelCount; j++) {
			cks_setting = 0;
			clock_freq = PP_SMC_TO_HOST_UL(
					smu_data->smc_state_table.GraphicsLevel[j].SclkFrequency);
			/* Check the allowed frequency against the sclk level[j].
			 *  Sclk's endianness has already been converted,
			 *  and it's in 10Khz unit,
			 *  as opposed to Data table, which is in Mhz unit.
			 */
			if (clock_freq >=
					(fiji_clock_stretcher_ddt_table[type][i][0]) * 100) {
				cks_setting |= 0x2;
				if (clock_freq <
						(fiji_clock_stretcher_ddt_table[type][i][1]) * 100)
					cks_setting |= 0x1;
			}
			smu_data->smc_state_table.ClockStretcherDataTable.
			ClockStretcherDataTableEntry[i].setting |= cks_setting << (j * 2);
		}
		CONVERT_FROM_HOST_TO_SMC_US(smu_data->smc_state_table.
				ClockStretcherDataTable.
				ClockStretcherDataTableEntry[i].setting);
	}

	value = cgs_read_ind_register(hwmgr->device, CGS_IND_REG__SMC, ixPWR_CKS_CNTL);
	value &= 0xFFFFFFFE;
	cgs_write_ind_register(hwmgr->device, CGS_IND_REG__SMC, ixPWR_CKS_CNTL, value);

	return 0;
}

static int fiji_populate_vr_config(struct pp_hwmgr *hwmgr,
		struct SMU73_Discrete_DpmTable *table)
{
	struct smu7_hwmgr *data = (struct smu7_hwmgr *)(hwmgr->backend);
	uint16_t config;

	config = VR_MERGED_WITH_VDDC;
	table->VRConfig |= (config << VRCONF_VDDGFX_SHIFT);

	/* Set Vddc Voltage Controller */
	if (SMU7_VOLTAGE_CONTROL_BY_SVID2 == data->voltage_control) {
		config = VR_SVI2_PLANE_1;
		table->VRConfig |= config;
	} else {
		PP_ASSERT_WITH_CODE(false,
				"VDDC should be on SVI2 control in merged mode!",
				);
	}
	/* Set Vddci Voltage Controller */
	if (SMU7_VOLTAGE_CONTROL_BY_SVID2 == data->vddci_control) {
		config = VR_SVI2_PLANE_2;  /* only in merged mode */
		table->VRConfig |= (config << VRCONF_VDDCI_SHIFT);
	} else if (SMU7_VOLTAGE_CONTROL_BY_GPIO == data->vddci_control) {
		config = VR_SMIO_PATTERN_1;
		table->VRConfig |= (config << VRCONF_VDDCI_SHIFT);
	} else {
		config = VR_STATIC_VOLTAGE;
		table->VRConfig |= (config << VRCONF_VDDCI_SHIFT);
	}
	/* Set Mvdd Voltage Controller */
	if (SMU7_VOLTAGE_CONTROL_BY_SVID2 == data->mvdd_control) {
		config = VR_SVI2_PLANE_2;
		table->VRConfig |= (config << VRCONF_MVDD_SHIFT);
	} else if (SMU7_VOLTAGE_CONTROL_BY_GPIO == data->mvdd_control) {
		config = VR_SMIO_PATTERN_2;
		table->VRConfig |= (config << VRCONF_MVDD_SHIFT);
	} else {
		config = VR_STATIC_VOLTAGE;
		table->VRConfig |= (config << VRCONF_MVDD_SHIFT);
	}

	return 0;
}

static int fiji_init_arb_table_index(struct pp_hwmgr *hwmgr)
{
	struct fiji_smumgr *smu_data = (struct fiji_smumgr *)(hwmgr->smu_backend);
	uint32_t tmp;
	int result;

	/* This is a read-modify-write on the first byte of the ARB table.
	 * The first byte in the SMU73_Discrete_MCArbDramTimingTable structure
	 * is the field 'current'.
	 * This solution is ugly, but we never write the whole table only
	 * individual fields in it.
	 * In reality this field should not be in that structure
	 * but in a soft register.
	 */
	result = smu7_read_smc_sram_dword(hwmgr,
			smu_data->smu7_data.arb_table_start, &tmp, SMC_RAM_END);

	if (result)
		return result;

	tmp &= 0x00FFFFFF;
	tmp |= ((uint32_t)MC_CG_ARB_FREQ_F1) << 24;

	return smu7_write_smc_sram_dword(hwmgr,
			smu_data->smu7_data.arb_table_start,  tmp, SMC_RAM_END);
}

static int fiji_save_default_power_profile(struct pp_hwmgr *hwmgr)
{
	struct fiji_smumgr *data = (struct fiji_smumgr *)(hwmgr->smu_backend);
	struct SMU73_Discrete_GraphicsLevel *levels =
				data->smc_state_table.GraphicsLevel;
	unsigned min_level = 1;

	hwmgr->default_gfx_power_profile.activity_threshold =
			be16_to_cpu(levels[0].ActivityLevel);
	hwmgr->default_gfx_power_profile.up_hyst = levels[0].UpHyst;
	hwmgr->default_gfx_power_profile.down_hyst = levels[0].DownHyst;
	hwmgr->default_gfx_power_profile.type = AMD_PP_GFX_PROFILE;

	hwmgr->default_compute_power_profile = hwmgr->default_gfx_power_profile;
	hwmgr->default_compute_power_profile.type = AMD_PP_COMPUTE_PROFILE;

	/* Workaround compute SDMA instability: disable lowest SCLK
	 * DPM level. Optimize compute power profile: Use only highest
	 * 2 power levels (if more than 2 are available), Hysteresis:
	 * 0ms up, 5ms down
	 */
	if (data->smc_state_table.GraphicsDpmLevelCount > 2)
		min_level = data->smc_state_table.GraphicsDpmLevelCount - 2;
	else if (data->smc_state_table.GraphicsDpmLevelCount == 2)
		min_level = 1;
	else
		min_level = 0;
	hwmgr->default_compute_power_profile.min_sclk =
			be32_to_cpu(levels[min_level].SclkFrequency);
	hwmgr->default_compute_power_profile.up_hyst = 0;
	hwmgr->default_compute_power_profile.down_hyst = 5;

	hwmgr->gfx_power_profile = hwmgr->default_gfx_power_profile;
	hwmgr->compute_power_profile = hwmgr->default_compute_power_profile;

	return 0;
}

static int fiji_setup_dpm_led_config(struct pp_hwmgr *hwmgr)
{
	pp_atomctrl_voltage_table param_led_dpm;
	int result = 0;
	u32 mask = 0;

	result = atomctrl_get_voltage_table_v3(hwmgr,
					       VOLTAGE_TYPE_LEDDPM, VOLTAGE_OBJ_GPIO_LUT,
					       &param_led_dpm);
	if (result == 0) {
		int i, j;
		u32 tmp = param_led_dpm.mask_low;

		for (i = 0, j = 0; i < 32; i++) {
			if (tmp & 1) {
				mask |= (i << (8 * j));
				if (++j >= 3)
					break;
			}
			tmp >>= 1;
		}
	}
	if (mask)
		smum_send_msg_to_smc_with_parameter(hwmgr,
						    PPSMC_MSG_LedConfig,
						    mask);
	return 0;
}

static int fiji_init_smc_table(struct pp_hwmgr *hwmgr)
{
	int result;
	struct smu7_hwmgr *data = (struct smu7_hwmgr *)(hwmgr->backend);
	struct fiji_smumgr *smu_data = (struct fiji_smumgr *)(hwmgr->smu_backend);
	struct phm_ppt_v1_information *table_info =
			(struct phm_ppt_v1_information *)(hwmgr->pptable);
	struct SMU73_Discrete_DpmTable *table = &(smu_data->smc_state_table);
	uint8_t i;
	struct pp_atomctrl_gpio_pin_assignment gpio_pin;

	fiji_initialize_power_tune_defaults(hwmgr);

	if (SMU7_VOLTAGE_CONTROL_NONE != data->voltage_control)
		fiji_populate_smc_voltage_tables(hwmgr, table);

	table->SystemFlags = 0;

	if (phm_cap_enabled(hwmgr->platform_descriptor.platformCaps,
			PHM_PlatformCaps_AutomaticDCTransition))
		table->SystemFlags |= PPSMC_SYSTEMFLAG_GPIO_DC;

	if (phm_cap_enabled(hwmgr->platform_descriptor.platformCaps,
			PHM_PlatformCaps_StepVddc))
		table->SystemFlags |= PPSMC_SYSTEMFLAG_STEPVDDC;

	if (data->is_memory_gddr5)
		table->SystemFlags |= PPSMC_SYSTEMFLAG_GDDR5;

	if (data->ulv_supported && table_info->us_ulv_voltage_offset) {
		result = fiji_populate_ulv_state(hwmgr, table);
		PP_ASSERT_WITH_CODE(0 == result,
				"Failed to initialize ULV state!", return result);
		cgs_write_ind_register(hwmgr->device, CGS_IND_REG__SMC,
				ixCG_ULV_PARAMETER, 0x40035);
	}

	result = fiji_populate_smc_link_level(hwmgr, table);
	PP_ASSERT_WITH_CODE(0 == result,
			"Failed to initialize Link Level!", return result);

	result = fiji_populate_all_graphic_levels(hwmgr);
	PP_ASSERT_WITH_CODE(0 == result,
			"Failed to initialize Graphics Level!", return result);

	result = fiji_populate_all_memory_levels(hwmgr);
	PP_ASSERT_WITH_CODE(0 == result,
			"Failed to initialize Memory Level!", return result);

	result = fiji_populate_smc_acpi_level(hwmgr, table);
	PP_ASSERT_WITH_CODE(0 == result,
			"Failed to initialize ACPI Level!", return result);

	result = fiji_populate_smc_vce_level(hwmgr, table);
	PP_ASSERT_WITH_CODE(0 == result,
			"Failed to initialize VCE Level!", return result);

	result = fiji_populate_smc_acp_level(hwmgr, table);
	PP_ASSERT_WITH_CODE(0 == result,
			"Failed to initialize ACP Level!", return result);

	result = fiji_populate_smc_samu_level(hwmgr, table);
	PP_ASSERT_WITH_CODE(0 == result,
			"Failed to initialize SAMU Level!", return result);

	/* Since only the initial state is completely set up at this point
	 * (the other states are just copies of the boot state) we only
	 * need to populate the  ARB settings for the initial state.
	 */
	result = fiji_program_memory_timing_parameters(hwmgr);
	PP_ASSERT_WITH_CODE(0 == result,
			"Failed to Write ARB settings for the initial state.", return result);

	result = fiji_populate_smc_uvd_level(hwmgr, table);
	PP_ASSERT_WITH_CODE(0 == result,
			"Failed to initialize UVD Level!", return result);

	result = fiji_populate_smc_boot_level(hwmgr, table);
	PP_ASSERT_WITH_CODE(0 == result,
			"Failed to initialize Boot Level!", return result);

	result = fiji_populate_smc_initailial_state(hwmgr);
	PP_ASSERT_WITH_CODE(0 == result,
			"Failed to initialize Boot State!", return result);

	result = fiji_populate_bapm_parameters_in_dpm_table(hwmgr);
	PP_ASSERT_WITH_CODE(0 == result,
			"Failed to populate BAPM Parameters!", return result);

	if (phm_cap_enabled(hwmgr->platform_descriptor.platformCaps,
			PHM_PlatformCaps_ClockStretcher)) {
		result = fiji_populate_clock_stretcher_data_table(hwmgr);
		PP_ASSERT_WITH_CODE(0 == result,
				"Failed to populate Clock Stretcher Data Table!",
				return result);
	}

	table->GraphicsVoltageChangeEnable  = 1;
	table->GraphicsThermThrottleEnable  = 1;
	table->GraphicsInterval = 1;
	table->VoltageInterval  = 1;
	table->ThermalInterval  = 1;
	table->TemperatureLimitHigh =
			table_info->cac_dtp_table->usTargetOperatingTemp *
			SMU7_Q88_FORMAT_CONVERSION_UNIT;
	table->TemperatureLimitLow  =
			(table_info->cac_dtp_table->usTargetOperatingTemp - 1) *
			SMU7_Q88_FORMAT_CONVERSION_UNIT;
	table->MemoryVoltageChangeEnable = 1;
	table->MemoryInterval = 1;
	table->VoltageResponseTime = 0;
	table->PhaseResponseTime = 0;
	table->MemoryThermThrottleEnable = 1;
	table->PCIeBootLinkLevel = 0;      /* 0:Gen1 1:Gen2 2:Gen3*/
	table->PCIeGenInterval = 1;
	table->VRConfig = 0;

	result = fiji_populate_vr_config(hwmgr, table);
	PP_ASSERT_WITH_CODE(0 == result,
			"Failed to populate VRConfig setting!", return result);

	table->ThermGpio = 17;
	table->SclkStepSize = 0x4000;

	if (atomctrl_get_pp_assign_pin(hwmgr, VDDC_VRHOT_GPIO_PINID, &gpio_pin)) {
		table->VRHotGpio = gpio_pin.uc_gpio_pin_bit_shift;
		phm_cap_set(hwmgr->platform_descriptor.platformCaps,
				PHM_PlatformCaps_RegulatorHot);
	} else {
		table->VRHotGpio = SMU7_UNUSED_GPIO_PIN;
		phm_cap_unset(hwmgr->platform_descriptor.platformCaps,
				PHM_PlatformCaps_RegulatorHot);
	}

	if (atomctrl_get_pp_assign_pin(hwmgr, PP_AC_DC_SWITCH_GPIO_PINID,
			&gpio_pin)) {
		table->AcDcGpio = gpio_pin.uc_gpio_pin_bit_shift;
		phm_cap_set(hwmgr->platform_descriptor.platformCaps,
				PHM_PlatformCaps_AutomaticDCTransition);
	} else {
		table->AcDcGpio = SMU7_UNUSED_GPIO_PIN;
		phm_cap_unset(hwmgr->platform_descriptor.platformCaps,
				PHM_PlatformCaps_AutomaticDCTransition);
	}

	/* Thermal Output GPIO */
	if (atomctrl_get_pp_assign_pin(hwmgr, THERMAL_INT_OUTPUT_GPIO_PINID,
			&gpio_pin)) {
		phm_cap_set(hwmgr->platform_descriptor.platformCaps,
				PHM_PlatformCaps_ThermalOutGPIO);

		table->ThermOutGpio = gpio_pin.uc_gpio_pin_bit_shift;

		/* For porlarity read GPIOPAD_A with assigned Gpio pin
		 * since VBIOS will program this register to set 'inactive state',
		 * driver can then determine 'active state' from this and
		 * program SMU with correct polarity
		 */
		table->ThermOutPolarity = (0 == (cgs_read_register(hwmgr->device, mmGPIOPAD_A) &
				(1 << gpio_pin.uc_gpio_pin_bit_shift))) ? 1:0;
		table->ThermOutMode = SMU7_THERM_OUT_MODE_THERM_ONLY;

		/* if required, combine VRHot/PCC with thermal out GPIO */
		if (phm_cap_enabled(hwmgr->platform_descriptor.platformCaps,
				PHM_PlatformCaps_RegulatorHot) &&
			phm_cap_enabled(hwmgr->platform_descriptor.platformCaps,
					PHM_PlatformCaps_CombinePCCWithThermalSignal))
			table->ThermOutMode = SMU7_THERM_OUT_MODE_THERM_VRHOT;
	} else {
		phm_cap_unset(hwmgr->platform_descriptor.platformCaps,
				PHM_PlatformCaps_ThermalOutGPIO);
		table->ThermOutGpio = 17;
		table->ThermOutPolarity = 1;
		table->ThermOutMode = SMU7_THERM_OUT_MODE_DISABLE;
	}

	for (i = 0; i < SMU73_MAX_ENTRIES_SMIO; i++)
		table->Smio[i] = PP_HOST_TO_SMC_UL(table->Smio[i]);

	CONVERT_FROM_HOST_TO_SMC_UL(table->SystemFlags);
	CONVERT_FROM_HOST_TO_SMC_UL(table->VRConfig);
	CONVERT_FROM_HOST_TO_SMC_UL(table->SmioMask1);
	CONVERT_FROM_HOST_TO_SMC_UL(table->SmioMask2);
	CONVERT_FROM_HOST_TO_SMC_UL(table->SclkStepSize);
	CONVERT_FROM_HOST_TO_SMC_US(table->TemperatureLimitHigh);
	CONVERT_FROM_HOST_TO_SMC_US(table->TemperatureLimitLow);
	CONVERT_FROM_HOST_TO_SMC_US(table->VoltageResponseTime);
	CONVERT_FROM_HOST_TO_SMC_US(table->PhaseResponseTime);

	/* Upload all dpm data to SMC memory.(dpm level, dpm level count etc) */
	result = smu7_copy_bytes_to_smc(hwmgr,
			smu_data->smu7_data.dpm_table_start +
			offsetof(SMU73_Discrete_DpmTable, SystemFlags),
			(uint8_t *)&(table->SystemFlags),
			sizeof(SMU73_Discrete_DpmTable) - 3 * sizeof(SMU73_PIDController),
			SMC_RAM_END);
	PP_ASSERT_WITH_CODE(0 == result,
			"Failed to upload dpm data to SMC memory!", return result);

	result = fiji_init_arb_table_index(hwmgr);
	PP_ASSERT_WITH_CODE(0 == result,
			"Failed to upload arb data to SMC memory!", return result);

	result = fiji_populate_pm_fuses(hwmgr);
	PP_ASSERT_WITH_CODE(0 == result,
			"Failed to  populate PM fuses to SMC memory!", return result);

	result = fiji_setup_dpm_led_config(hwmgr);
	PP_ASSERT_WITH_CODE(0 == result,
			    "Failed to setup dpm led config", return result);

	fiji_save_default_power_profile(hwmgr);

	return 0;
}

static int fiji_thermal_setup_fan_table(struct pp_hwmgr *hwmgr)
{
	struct fiji_smumgr *smu_data = (struct fiji_smumgr *)(hwmgr->smu_backend);

	SMU73_Discrete_FanTable fan_table = { FDO_MODE_HARDWARE };
	uint32_t duty100;
	uint32_t t_diff1, t_diff2, pwm_diff1, pwm_diff2;
	uint16_t fdo_min, slope1, slope2;
	uint32_t reference_clock;
	int res;
	uint64_t tmp64;

	if (hwmgr->thermal_controller.fanInfo.bNoFan) {
		phm_cap_unset(hwmgr->platform_descriptor.platformCaps,
			PHM_PlatformCaps_MicrocodeFanControl);
		return 0;
	}

	if (smu_data->smu7_data.fan_table_start == 0) {
		phm_cap_unset(hwmgr->platform_descriptor.platformCaps,
				PHM_PlatformCaps_MicrocodeFanControl);
		return 0;
	}

	duty100 = PHM_READ_VFPF_INDIRECT_FIELD(hwmgr->device, CGS_IND_REG__SMC,
			CG_FDO_CTRL1, FMAX_DUTY100);

	if (duty100 == 0) {
		phm_cap_unset(hwmgr->platform_descriptor.platformCaps,
				PHM_PlatformCaps_MicrocodeFanControl);
		return 0;
	}

	tmp64 = hwmgr->thermal_controller.advanceFanControlParameters.
			usPWMMin * duty100;
	do_div(tmp64, 10000);
	fdo_min = (uint16_t)tmp64;

	t_diff1 = hwmgr->thermal_controller.advanceFanControlParameters.usTMed -
			hwmgr->thermal_controller.advanceFanControlParameters.usTMin;
	t_diff2 = hwmgr->thermal_controller.advanceFanControlParameters.usTHigh -
			hwmgr->thermal_controller.advanceFanControlParameters.usTMed;

	pwm_diff1 = hwmgr->thermal_controller.advanceFanControlParameters.usPWMMed -
			hwmgr->thermal_controller.advanceFanControlParameters.usPWMMin;
	pwm_diff2 = hwmgr->thermal_controller.advanceFanControlParameters.usPWMHigh -
			hwmgr->thermal_controller.advanceFanControlParameters.usPWMMed;

	slope1 = (uint16_t)((50 + ((16 * duty100 * pwm_diff1) / t_diff1)) / 100);
	slope2 = (uint16_t)((50 + ((16 * duty100 * pwm_diff2) / t_diff2)) / 100);

	fan_table.TempMin = cpu_to_be16((50 + hwmgr->
			thermal_controller.advanceFanControlParameters.usTMin) / 100);
	fan_table.TempMed = cpu_to_be16((50 + hwmgr->
			thermal_controller.advanceFanControlParameters.usTMed) / 100);
	fan_table.TempMax = cpu_to_be16((50 + hwmgr->
			thermal_controller.advanceFanControlParameters.usTMax) / 100);

	fan_table.Slope1 = cpu_to_be16(slope1);
	fan_table.Slope2 = cpu_to_be16(slope2);

	fan_table.FdoMin = cpu_to_be16(fdo_min);

	fan_table.HystDown = cpu_to_be16(hwmgr->
			thermal_controller.advanceFanControlParameters.ucTHyst);

	fan_table.HystUp = cpu_to_be16(1);

	fan_table.HystSlope = cpu_to_be16(1);

	fan_table.TempRespLim = cpu_to_be16(5);

	reference_clock = smu7_get_xclk(hwmgr);

	fan_table.RefreshPeriod = cpu_to_be32((hwmgr->
			thermal_controller.advanceFanControlParameters.ulCycleDelay *
			reference_clock) / 1600);

	fan_table.FdoMax = cpu_to_be16((uint16_t)duty100);

	fan_table.TempSrc = (uint8_t)PHM_READ_VFPF_INDIRECT_FIELD(
			hwmgr->device, CGS_IND_REG__SMC,
			CG_MULT_THERMAL_CTRL, TEMP_SEL);

	res = smu7_copy_bytes_to_smc(hwmgr, smu_data->smu7_data.fan_table_start,
			(uint8_t *)&fan_table, (uint32_t)sizeof(fan_table),
			SMC_RAM_END);

	if (!res && hwmgr->thermal_controller.
			advanceFanControlParameters.ucMinimumPWMLimit)
		res = smum_send_msg_to_smc_with_parameter(hwmgr,
				PPSMC_MSG_SetFanMinPwm,
				hwmgr->thermal_controller.
				advanceFanControlParameters.ucMinimumPWMLimit);

	if (!res && hwmgr->thermal_controller.
			advanceFanControlParameters.ulMinFanSCLKAcousticLimit)
		res = smum_send_msg_to_smc_with_parameter(hwmgr,
				PPSMC_MSG_SetFanSclkTarget,
				hwmgr->thermal_controller.
				advanceFanControlParameters.ulMinFanSCLKAcousticLimit);

	if (res)
		phm_cap_unset(hwmgr->platform_descriptor.platformCaps,
				PHM_PlatformCaps_MicrocodeFanControl);

	return 0;
}


static int fiji_thermal_avfs_enable(struct pp_hwmgr *hwmgr)
{
	int ret;
	struct smu7_smumgr *smu_data = (struct smu7_smumgr *)(hwmgr->smu_backend);

	if (smu_data->avfs.avfs_btc_status != AVFS_BTC_ENABLEAVFS)
		return 0;

	ret = smum_send_msg_to_smc(hwmgr, PPSMC_MSG_EnableAvfs);

	if (!ret)
		/* If this param is not changed, this function could fire unnecessarily */
		smu_data->avfs.avfs_btc_status = AVFS_BTC_COMPLETED_PREVIOUSLY;

	return ret;
}

static int fiji_program_mem_timing_parameters(struct pp_hwmgr *hwmgr)
{
	struct smu7_hwmgr *data = (struct smu7_hwmgr *)(hwmgr->backend);

	if (data->need_update_smu7_dpm_table &
		(DPMTABLE_OD_UPDATE_SCLK + DPMTABLE_OD_UPDATE_MCLK))
		return fiji_program_memory_timing_parameters(hwmgr);

	return 0;
}

static int fiji_update_sclk_threshold(struct pp_hwmgr *hwmgr)
{
	struct smu7_hwmgr *data = (struct smu7_hwmgr *)(hwmgr->backend);
	struct fiji_smumgr *smu_data = (struct fiji_smumgr *)(hwmgr->smu_backend);

	int result = 0;
	uint32_t low_sclk_interrupt_threshold = 0;

	if (phm_cap_enabled(hwmgr->platform_descriptor.platformCaps,
			PHM_PlatformCaps_SclkThrottleLowNotification)
		&& (hwmgr->gfx_arbiter.sclk_threshold !=
				data->low_sclk_interrupt_threshold)) {
		data->low_sclk_interrupt_threshold =
				hwmgr->gfx_arbiter.sclk_threshold;
		low_sclk_interrupt_threshold =
				data->low_sclk_interrupt_threshold;

		CONVERT_FROM_HOST_TO_SMC_UL(low_sclk_interrupt_threshold);

		result = smu7_copy_bytes_to_smc(
				hwmgr,
				smu_data->smu7_data.dpm_table_start +
				offsetof(SMU73_Discrete_DpmTable,
					LowSclkInterruptThreshold),
				(uint8_t *)&low_sclk_interrupt_threshold,
				sizeof(uint32_t),
				SMC_RAM_END);
	}
	result = fiji_program_mem_timing_parameters(hwmgr);
	PP_ASSERT_WITH_CODE((result == 0),
			"Failed to program memory timing parameters!",
			);
	return result;
}

static uint32_t fiji_get_offsetof(uint32_t type, uint32_t member)
{
	switch (type) {
	case SMU_SoftRegisters:
		switch (member) {
		case HandshakeDisables:
			return offsetof(SMU73_SoftRegisters, HandshakeDisables);
		case VoltageChangeTimeout:
			return offsetof(SMU73_SoftRegisters, VoltageChangeTimeout);
		case AverageGraphicsActivity:
			return offsetof(SMU73_SoftRegisters, AverageGraphicsActivity);
		case PreVBlankGap:
			return offsetof(SMU73_SoftRegisters, PreVBlankGap);
		case VBlankTimeout:
			return offsetof(SMU73_SoftRegisters, VBlankTimeout);
		case UcodeLoadStatus:
			return offsetof(SMU73_SoftRegisters, UcodeLoadStatus);
		case DRAM_LOG_ADDR_H:
			return offsetof(SMU73_SoftRegisters, DRAM_LOG_ADDR_H);
		case DRAM_LOG_ADDR_L:
			return offsetof(SMU73_SoftRegisters, DRAM_LOG_ADDR_L);
		case DRAM_LOG_PHY_ADDR_H:
			return offsetof(SMU73_SoftRegisters, DRAM_LOG_PHY_ADDR_H);
		case DRAM_LOG_PHY_ADDR_L:
			return offsetof(SMU73_SoftRegisters, DRAM_LOG_PHY_ADDR_L);
		case DRAM_LOG_BUFF_SIZE:
			return offsetof(SMU73_SoftRegisters, DRAM_LOG_BUFF_SIZE);
		}
	case SMU_Discrete_DpmTable:
		switch (member) {
		case UvdBootLevel:
			return offsetof(SMU73_Discrete_DpmTable, UvdBootLevel);
		case VceBootLevel:
			return offsetof(SMU73_Discrete_DpmTable, VceBootLevel);
		case SamuBootLevel:
			return offsetof(SMU73_Discrete_DpmTable, SamuBootLevel);
		case LowSclkInterruptThreshold:
			return offsetof(SMU73_Discrete_DpmTable, LowSclkInterruptThreshold);
		}
	}
	pr_warn("can't get the offset of type %x member %x\n", type, member);
	return 0;
}

static uint32_t fiji_get_mac_definition(uint32_t value)
{
	switch (value) {
	case SMU_MAX_LEVELS_GRAPHICS:
		return SMU73_MAX_LEVELS_GRAPHICS;
	case SMU_MAX_LEVELS_MEMORY:
		return SMU73_MAX_LEVELS_MEMORY;
	case SMU_MAX_LEVELS_LINK:
		return SMU73_MAX_LEVELS_LINK;
	case SMU_MAX_ENTRIES_SMIO:
		return SMU73_MAX_ENTRIES_SMIO;
	case SMU_MAX_LEVELS_VDDC:
		return SMU73_MAX_LEVELS_VDDC;
	case SMU_MAX_LEVELS_VDDGFX:
		return SMU73_MAX_LEVELS_VDDGFX;
	case SMU_MAX_LEVELS_VDDCI:
		return SMU73_MAX_LEVELS_VDDCI;
	case SMU_MAX_LEVELS_MVDD:
		return SMU73_MAX_LEVELS_MVDD;
	}

	pr_warn("can't get the mac of %x\n", value);
	return 0;
}


static int fiji_update_uvd_smc_table(struct pp_hwmgr *hwmgr)
{
	struct fiji_smumgr *smu_data = (struct fiji_smumgr *)(hwmgr->smu_backend);
	uint32_t mm_boot_level_offset, mm_boot_level_value;
	struct phm_ppt_v1_information *table_info =
			(struct phm_ppt_v1_information *)(hwmgr->pptable);

	smu_data->smc_state_table.UvdBootLevel = 0;
	if (table_info->mm_dep_table->count > 0)
		smu_data->smc_state_table.UvdBootLevel =
				(uint8_t) (table_info->mm_dep_table->count - 1);
	mm_boot_level_offset = smu_data->smu7_data.dpm_table_start + offsetof(SMU73_Discrete_DpmTable,
						UvdBootLevel);
	mm_boot_level_offset /= 4;
	mm_boot_level_offset *= 4;
	mm_boot_level_value = cgs_read_ind_register(hwmgr->device,
			CGS_IND_REG__SMC, mm_boot_level_offset);
	mm_boot_level_value &= 0x00FFFFFF;
	mm_boot_level_value |= smu_data->smc_state_table.UvdBootLevel << 24;
	cgs_write_ind_register(hwmgr->device,
			CGS_IND_REG__SMC, mm_boot_level_offset, mm_boot_level_value);

	if (!phm_cap_enabled(hwmgr->platform_descriptor.platformCaps,
			PHM_PlatformCaps_UVDDPM) ||
		phm_cap_enabled(hwmgr->platform_descriptor.platformCaps,
			PHM_PlatformCaps_StablePState))
		smum_send_msg_to_smc_with_parameter(hwmgr,
				PPSMC_MSG_UVDDPM_SetEnabledMask,
				(uint32_t)(1 << smu_data->smc_state_table.UvdBootLevel));
	return 0;
}

static int fiji_update_vce_smc_table(struct pp_hwmgr *hwmgr)
{
	struct fiji_smumgr *smu_data = (struct fiji_smumgr *)(hwmgr->smu_backend);
	uint32_t mm_boot_level_offset, mm_boot_level_value;
	struct phm_ppt_v1_information *table_info =
			(struct phm_ppt_v1_information *)(hwmgr->pptable);

	if (phm_cap_enabled(hwmgr->platform_descriptor.platformCaps,
					PHM_PlatformCaps_StablePState))
		smu_data->smc_state_table.VceBootLevel =
			(uint8_t) (table_info->mm_dep_table->count - 1);
	else
		smu_data->smc_state_table.VceBootLevel = 0;

	mm_boot_level_offset = smu_data->smu7_data.dpm_table_start +
					offsetof(SMU73_Discrete_DpmTable, VceBootLevel);
	mm_boot_level_offset /= 4;
	mm_boot_level_offset *= 4;
	mm_boot_level_value = cgs_read_ind_register(hwmgr->device,
			CGS_IND_REG__SMC, mm_boot_level_offset);
	mm_boot_level_value &= 0xFF00FFFF;
	mm_boot_level_value |= smu_data->smc_state_table.VceBootLevel << 16;
	cgs_write_ind_register(hwmgr->device,
			CGS_IND_REG__SMC, mm_boot_level_offset, mm_boot_level_value);

	if (phm_cap_enabled(hwmgr->platform_descriptor.platformCaps, PHM_PlatformCaps_StablePState))
		smum_send_msg_to_smc_with_parameter(hwmgr,
				PPSMC_MSG_VCEDPM_SetEnabledMask,
				(uint32_t)1 << smu_data->smc_state_table.VceBootLevel);
	return 0;
}

static int fiji_update_samu_smc_table(struct pp_hwmgr *hwmgr)
{
	struct fiji_smumgr *smu_data = (struct fiji_smumgr *)(hwmgr->smu_backend);
	uint32_t mm_boot_level_offset, mm_boot_level_value;


	smu_data->smc_state_table.SamuBootLevel = 0;
	mm_boot_level_offset = smu_data->smu7_data.dpm_table_start +
				offsetof(SMU73_Discrete_DpmTable, SamuBootLevel);

	mm_boot_level_offset /= 4;
	mm_boot_level_offset *= 4;
	mm_boot_level_value = cgs_read_ind_register(hwmgr->device,
			CGS_IND_REG__SMC, mm_boot_level_offset);
	mm_boot_level_value &= 0xFFFFFF00;
	mm_boot_level_value |= smu_data->smc_state_table.SamuBootLevel << 0;
	cgs_write_ind_register(hwmgr->device,
			CGS_IND_REG__SMC, mm_boot_level_offset, mm_boot_level_value);

	if (phm_cap_enabled(hwmgr->platform_descriptor.platformCaps,
			PHM_PlatformCaps_StablePState))
		smum_send_msg_to_smc_with_parameter(hwmgr,
				PPSMC_MSG_SAMUDPM_SetEnabledMask,
				(uint32_t)(1 << smu_data->smc_state_table.SamuBootLevel));
	return 0;
}

static int fiji_update_smc_table(struct pp_hwmgr *hwmgr, uint32_t type)
{
	switch (type) {
	case SMU_UVD_TABLE:
		fiji_update_uvd_smc_table(hwmgr);
		break;
	case SMU_VCE_TABLE:
		fiji_update_vce_smc_table(hwmgr);
		break;
	case SMU_SAMU_TABLE:
		fiji_update_samu_smc_table(hwmgr);
		break;
	default:
		break;
	}
	return 0;
}

static int fiji_process_firmware_header(struct pp_hwmgr *hwmgr)
{
	struct smu7_hwmgr *data = (struct smu7_hwmgr *)(hwmgr->backend);
	struct fiji_smumgr *smu_data = (struct fiji_smumgr *)(hwmgr->smu_backend);
	uint32_t tmp;
	int result;
	bool error = false;

	result = smu7_read_smc_sram_dword(hwmgr,
			SMU7_FIRMWARE_HEADER_LOCATION +
			offsetof(SMU73_Firmware_Header, DpmTable),
			&tmp, SMC_RAM_END);

	if (0 == result)
		smu_data->smu7_data.dpm_table_start = tmp;

	error |= (0 != result);

	result = smu7_read_smc_sram_dword(hwmgr,
			SMU7_FIRMWARE_HEADER_LOCATION +
			offsetof(SMU73_Firmware_Header, SoftRegisters),
			&tmp, SMC_RAM_END);

	if (!result) {
		data->soft_regs_start = tmp;
		smu_data->smu7_data.soft_regs_start = tmp;
	}

	error |= (0 != result);

	result = smu7_read_smc_sram_dword(hwmgr,
			SMU7_FIRMWARE_HEADER_LOCATION +
			offsetof(SMU73_Firmware_Header, mcRegisterTable),
			&tmp, SMC_RAM_END);

	if (!result)
		smu_data->smu7_data.mc_reg_table_start = tmp;

	result = smu7_read_smc_sram_dword(hwmgr,
			SMU7_FIRMWARE_HEADER_LOCATION +
			offsetof(SMU73_Firmware_Header, FanTable),
			&tmp, SMC_RAM_END);

	if (!result)
		smu_data->smu7_data.fan_table_start = tmp;

	error |= (0 != result);

	result = smu7_read_smc_sram_dword(hwmgr,
			SMU7_FIRMWARE_HEADER_LOCATION +
			offsetof(SMU73_Firmware_Header, mcArbDramTimingTable),
			&tmp, SMC_RAM_END);

	if (!result)
		smu_data->smu7_data.arb_table_start = tmp;

	error |= (0 != result);

	result = smu7_read_smc_sram_dword(hwmgr,
			SMU7_FIRMWARE_HEADER_LOCATION +
			offsetof(SMU73_Firmware_Header, Version),
			&tmp, SMC_RAM_END);

	if (!result)
		hwmgr->microcode_version_info.SMC = tmp;

	error |= (0 != result);

	return error ? -1 : 0;
}

static int fiji_initialize_mc_reg_table(struct pp_hwmgr *hwmgr)
{

	/* Program additional LP registers
	 * that are no longer programmed by VBIOS
	 */
	cgs_write_register(hwmgr->device, mmMC_SEQ_RAS_TIMING_LP,
			cgs_read_register(hwmgr->device, mmMC_SEQ_RAS_TIMING));
	cgs_write_register(hwmgr->device, mmMC_SEQ_CAS_TIMING_LP,
			cgs_read_register(hwmgr->device, mmMC_SEQ_CAS_TIMING));
	cgs_write_register(hwmgr->device, mmMC_SEQ_MISC_TIMING2_LP,
			cgs_read_register(hwmgr->device, mmMC_SEQ_MISC_TIMING2));
	cgs_write_register(hwmgr->device, mmMC_SEQ_WR_CTL_D1_LP,
			cgs_read_register(hwmgr->device, mmMC_SEQ_WR_CTL_D1));
	cgs_write_register(hwmgr->device, mmMC_SEQ_RD_CTL_D0_LP,
			cgs_read_register(hwmgr->device, mmMC_SEQ_RD_CTL_D0));
	cgs_write_register(hwmgr->device, mmMC_SEQ_RD_CTL_D1_LP,
			cgs_read_register(hwmgr->device, mmMC_SEQ_RD_CTL_D1));
	cgs_write_register(hwmgr->device, mmMC_SEQ_PMG_TIMING_LP,
			cgs_read_register(hwmgr->device, mmMC_SEQ_PMG_TIMING));

	return 0;
}

static bool fiji_is_dpm_running(struct pp_hwmgr *hwmgr)
{
	return (1 == PHM_READ_INDIRECT_FIELD(hwmgr->device,
			CGS_IND_REG__SMC, FEATURE_STATUS, VOLTAGE_CONTROLLER_ON))
			? true : false;
}

static int fiji_populate_requested_graphic_levels(struct pp_hwmgr *hwmgr,
		struct amd_pp_profile *request)
{
	struct fiji_smumgr *smu_data = (struct fiji_smumgr *)
			(hwmgr->smu_backend);
	struct SMU73_Discrete_GraphicsLevel *levels =
			smu_data->smc_state_table.GraphicsLevel;
	uint32_t array = smu_data->smu7_data.dpm_table_start +
			offsetof(SMU73_Discrete_DpmTable, GraphicsLevel);
	uint32_t array_size = sizeof(struct SMU73_Discrete_GraphicsLevel) *
			SMU73_MAX_LEVELS_GRAPHICS;
	uint32_t i;

	for (i = 0; i < smu_data->smc_state_table.GraphicsDpmLevelCount; i++) {
		levels[i].ActivityLevel =
				cpu_to_be16(request->activity_threshold);
		levels[i].EnabledForActivity = 1;
		levels[i].UpHyst = request->up_hyst;
		levels[i].DownHyst = request->down_hyst;
	}

	return smu7_copy_bytes_to_smc(hwmgr, array, (uint8_t *)levels,
				array_size, SMC_RAM_END);
}

const struct pp_smumgr_func fiji_smu_funcs = {
	.smu_init = &fiji_smu_init,
	.smu_fini = &smu7_smu_fini,
	.start_smu = &fiji_start_smu,
	.check_fw_load_finish = &smu7_check_fw_load_finish,
	.request_smu_load_fw = &smu7_reload_firmware,
	.request_smu_load_specific_fw = NULL,
	.send_msg_to_smc = &smu7_send_msg_to_smc,
	.send_msg_to_smc_with_parameter = &smu7_send_msg_to_smc_with_parameter,
	.download_pptable_settings = NULL,
	.upload_pptable_settings = NULL,
	.update_smc_table = fiji_update_smc_table,
	.get_offsetof = fiji_get_offsetof,
	.process_firmware_header = fiji_process_firmware_header,
	.init_smc_table = fiji_init_smc_table,
	.update_sclk_threshold = fiji_update_sclk_threshold,
	.thermal_setup_fan_table = fiji_thermal_setup_fan_table,
	.thermal_avfs_enable = fiji_thermal_avfs_enable,
	.populate_all_graphic_levels = fiji_populate_all_graphic_levels,
	.populate_all_memory_levels = fiji_populate_all_memory_levels,
	.get_mac_definition = fiji_get_mac_definition,
	.initialize_mc_reg_table = fiji_initialize_mc_reg_table,
	.is_dpm_running = fiji_is_dpm_running,
	.populate_requested_graphic_levels = fiji_populate_requested_graphic_levels,
	.is_hw_avfs_present = fiji_is_hw_avfs_present,
};
>>>>>>> temp
