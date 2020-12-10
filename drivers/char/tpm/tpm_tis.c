/*
 * Copyright (C) 2005, 2006 IBM Corporation
 * Copyright (C) 2014, 2015 Intel Corporation
 *
 * Authors:
 * Leendert van Doorn <leendert@watson.ibm.com>
 * Kylene Hall <kjhall@us.ibm.com>
 *
 * Maintained by: <tpmdd-devel@lists.sourceforge.net>
 *
 * Device driver for TCG/TCPA TPM (trusted platform module).
 * Specifications at www.trustedcomputinggroup.org
 *
 * This device driver implements the TPM interface as defined in
 * the TCG TPM Interface Spec version 1.2, revision 1.0.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2 of the
 * License.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/pnp.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/acpi.h>
#include <linux/freezer.h>
<<<<<<< HEAD
=======
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/kernel.h>
>>>>>>> temp
#include "tpm.h"
#include "tpm_tis_core.h"

struct tpm_info {
	struct resource res;
	/* irq > 0 means: use irq $irq;
	 * irq = 0 means: autoprobe for an irq;
	 * irq = -1 means: no irq support
	 */
	int irq;
};

struct tpm_tis_tcg_phy {
	struct tpm_tis_data priv;
	void __iomem *iobase;
};

<<<<<<< HEAD
enum tis_defaults {
	TIS_MEM_LEN = 0x5000,
	TIS_SHORT_TIMEOUT = 750,	/* ms */
	TIS_LONG_TIMEOUT = 2000,	/* 2 sec */
};

struct tpm_info {
	struct resource res;
	/* irq > 0 means: use irq $irq;
	 * irq = 0 means: autoprobe for an irq;
	 * irq = -1 means: no irq support
	 */
	int irq;
};
=======
static inline struct tpm_tis_tcg_phy *to_tpm_tis_tcg_phy(struct tpm_tis_data *data)
{
	return container_of(data, struct tpm_tis_tcg_phy, priv);
}

static bool interrupts = true;
module_param(interrupts, bool, 0444);
MODULE_PARM_DESC(interrupts, "Enable interrupts");

static bool itpm;
module_param(itpm, bool, 0444);
MODULE_PARM_DESC(itpm, "Force iTPM workarounds (found on some Lenovo laptops)");
>>>>>>> temp

static bool force;
#ifdef CONFIG_X86
module_param(force, bool, 0444);
MODULE_PARM_DESC(force, "Force device probe rather than using ACPI entry");
#endif

#if defined(CONFIG_PNP) && defined(CONFIG_ACPI)
static int has_hid(struct acpi_device *dev, const char *hid)
{
	struct acpi_hardware_id *id;

	list_for_each_entry(id, &dev->pnp.ids, list)
		if (!strcmp(hid, id->id))
			return 1;

	return 0;
}

static inline int is_itpm(struct acpi_device *dev)
{
<<<<<<< HEAD
=======
	if (!dev)
		return 0;
>>>>>>> temp
	return has_hid(dev, "INTC0102");
}
#else
static inline int is_itpm(struct acpi_device *dev)
{
	return 0;
}
#endif

#if defined(CONFIG_ACPI)
#define DEVICE_IS_TPM2 1

static const struct acpi_device_id tpm_acpi_tbl[] = {
	{"MSFT0101", DEVICE_IS_TPM2},
	{},
};
MODULE_DEVICE_TABLE(acpi, tpm_acpi_tbl);

static int check_acpi_tpm2(struct device *dev)
{
	const struct acpi_device_id *aid = acpi_match_device(tpm_acpi_tbl, dev);
	struct acpi_table_tpm2 *tbl;
	acpi_status st;

	if (!aid || aid->driver_data != DEVICE_IS_TPM2)
		return 0;

	/* If the ACPI TPM2 signature is matched then a global ACPI_SIG_TPM2
	 * table is mandatory
	 */
	st =
	    acpi_get_table(ACPI_SIG_TPM2, 1, (struct acpi_table_header **)&tbl);
	if (ACPI_FAILURE(st) || tbl->header.length < sizeof(*tbl)) {
		dev_err(dev, FW_BUG "failed to get TPM2 ACPI table\n");
		return -EINVAL;
	}

	/* The tpm2_crb driver handles this device */
	if (tbl->start_method != ACPI_TPM2_MEMORY_MAPPED)
		return -ENODEV;

	return 0;
}
#else
static int check_acpi_tpm2(struct device *dev)
{
	return 0;
}
#endif

static int tpm_tcg_read_bytes(struct tpm_tis_data *data, u32 addr, u16 len,
			      u8 *result)
{
<<<<<<< HEAD
	int size = 0;
	int status;
	u32 expected;

	if (count < TPM_HEADER_SIZE) {
		size = -EIO;
		goto out;
	}

	/* read first 10 bytes, including tag, paramsize, and result */
	if ((size =
	     recv_data(chip, buf, TPM_HEADER_SIZE)) < TPM_HEADER_SIZE) {
		dev_err(&chip->dev, "Unable to read header\n");
		goto out;
	}

	expected = be32_to_cpu(*(__be32 *) (buf + 2));
	if (expected > count || expected < TPM_HEADER_SIZE) {
		size = -EIO;
		goto out;
	}

	if ((size +=
	     recv_data(chip, &buf[TPM_HEADER_SIZE],
		       expected - TPM_HEADER_SIZE)) < expected) {
		dev_err(&chip->dev, "Unable to read remainder of result\n");
		size = -ETIME;
		goto out;
	}

	wait_for_tpm_stat(chip, TPM_STS_VALID, chip->vendor.timeout_c,
			  &chip->vendor.int_queue, false);
	status = tpm_tis_status(chip);
	if (status & TPM_STS_DATA_AVAIL) {	/* retry? */
		dev_err(&chip->dev, "Error left over data\n");
		size = -EIO;
		goto out;
	}
=======
	struct tpm_tis_tcg_phy *phy = to_tpm_tis_tcg_phy(data);

	while (len--)
		*result++ = ioread8(phy->iobase + addr);
>>>>>>> temp

	return 0;
}

static int tpm_tcg_write_bytes(struct tpm_tis_data *data, u32 addr, u16 len,
			       const u8 *value)
{
	struct tpm_tis_tcg_phy *phy = to_tpm_tis_tcg_phy(data);

	while (len--)
		iowrite8(*value++, phy->iobase + addr);

	return 0;
}

static int tpm_tcg_read16(struct tpm_tis_data *data, u32 addr, u16 *result)
{
<<<<<<< HEAD
	u32 intmask;

	intmask =
	    ioread32(chip->vendor.iobase +
		     TPM_INT_ENABLE(chip->vendor.locality));
	intmask &= ~TPM_GLOBAL_INT_ENABLE;
	iowrite32(intmask,
		  chip->vendor.iobase +
		  TPM_INT_ENABLE(chip->vendor.locality));
	devm_free_irq(&chip->dev, chip->vendor.irq, chip);
	chip->vendor.irq = 0;
}
=======
	struct tpm_tis_tcg_phy *phy = to_tpm_tis_tcg_phy(data);
>>>>>>> temp

	*result = ioread16(phy->iobase + addr);

<<<<<<< HEAD
	if (chip->vendor.irq) {
		ordinal = be32_to_cpu(*((__be32 *) (buf + 6)));

		if (chip->flags & TPM_CHIP_FLAG_TPM2)
			dur = tpm2_calc_ordinal_duration(chip, ordinal);
		else
			dur = tpm_calc_ordinal_duration(chip, ordinal);

		if (wait_for_tpm_stat
		    (chip, TPM_STS_DATA_AVAIL | TPM_STS_VALID, dur,
		     &chip->vendor.read_queue, false) < 0) {
			rc = -ETIME;
			goto out_err;
		}
	}
	return len;
out_err:
	tpm_tis_ready(chip);
	release_locality(chip, chip->vendor.locality, 0);
	return rc;
}

static int tpm_tis_send(struct tpm_chip *chip, u8 *buf, size_t len)
{
	int rc, irq;
	struct priv_data *priv = chip->vendor.priv;

	if (!chip->vendor.irq || priv->irq_tested)
		return tpm_tis_send_main(chip, buf, len);

	/* Verify receipt of the expected IRQ */
	irq = chip->vendor.irq;
	chip->vendor.irq = 0;
	rc = tpm_tis_send_main(chip, buf, len);
	chip->vendor.irq = irq;
	if (!priv->irq_tested)
		msleep(1);
	if (!priv->irq_tested)
		disable_interrupts(chip);
	priv->irq_tested = true;
	return rc;
=======
	return 0;
>>>>>>> temp
}

static int tpm_tcg_read32(struct tpm_tis_data *data, u32 addr, u32 *result)
{
	struct tpm_tis_tcg_phy *phy = to_tpm_tis_tcg_phy(data);

	*result = ioread32(phy->iobase + addr);

	return 0;
}

static int tpm_tcg_write32(struct tpm_tis_data *data, u32 addr, u32 value)
{
	struct tpm_tis_tcg_phy *phy = to_tpm_tis_tcg_phy(data);

<<<<<<< HEAD
	rc = tpm_tis_send_data(chip, cmd_getticks, len);
	if (rc == 0) {
		dev_info(&chip->dev, "Detected an iTPM.\n");
		rc = 1;
	} else
		rc = -EFAULT;
=======
	iowrite32(value, phy->iobase + addr);
>>>>>>> temp

	return 0;
}

static const struct tpm_tis_phy_ops tpm_tcg = {
	.read_bytes = tpm_tcg_read_bytes,
	.write_bytes = tpm_tcg_write_bytes,
	.read16 = tpm_tcg_read16,
	.read32 = tpm_tcg_read32,
	.write32 = tpm_tcg_write32,
};

<<<<<<< HEAD
static irqreturn_t tis_int_handler(int dummy, void *dev_id)
{
	struct tpm_chip *chip = dev_id;
	u32 interrupt;
	int i;

	interrupt = ioread32(chip->vendor.iobase +
			     TPM_INT_STATUS(chip->vendor.locality));

	if (interrupt == 0)
		return IRQ_NONE;

	((struct priv_data *)chip->vendor.priv)->irq_tested = true;
	if (interrupt & TPM_INTF_DATA_AVAIL_INT)
		wake_up_interruptible(&chip->vendor.read_queue);
	if (interrupt & TPM_INTF_LOCALITY_CHANGE_INT)
		for (i = 0; i < 5; i++)
			if (check_locality(chip, i) >= 0)
				break;
	if (interrupt &
	    (TPM_INTF_LOCALITY_CHANGE_INT | TPM_INTF_STS_VALID_INT |
	     TPM_INTF_CMD_READY_INT))
		wake_up_interruptible(&chip->vendor.int_queue);

	/* Clear interrupts handled with TPM_EOI */
	iowrite32(interrupt,
		  chip->vendor.iobase +
		  TPM_INT_STATUS(chip->vendor.locality));
	ioread32(chip->vendor.iobase + TPM_INT_STATUS(chip->vendor.locality));
	return IRQ_HANDLED;
}

/* Register the IRQ and issue a command that will cause an interrupt. If an
 * irq is seen then leave the chip setup for IRQ operation, otherwise reverse
 * everything and leave in polling mode. Returns 0 on success.
 */
static int tpm_tis_probe_irq_single(struct tpm_chip *chip, u32 intmask,
				    int flags, int irq)
{
	struct priv_data *priv = chip->vendor.priv;
	u8 original_int_vec;

	if (devm_request_irq(&chip->dev, irq, tis_int_handler, flags,
			     chip->devname, chip) != 0) {
		dev_info(&chip->dev, "Unable to request irq: %d for probe\n",
			 irq);
		return -1;
	}
	chip->vendor.irq = irq;

	original_int_vec = ioread8(chip->vendor.iobase +
				   TPM_INT_VECTOR(chip->vendor.locality));
	iowrite8(irq,
		 chip->vendor.iobase + TPM_INT_VECTOR(chip->vendor.locality));

	/* Clear all existing */
	iowrite32(ioread32(chip->vendor.iobase +
			   TPM_INT_STATUS(chip->vendor.locality)),
		  chip->vendor.iobase + TPM_INT_STATUS(chip->vendor.locality));

	/* Turn on */
	iowrite32(intmask | TPM_GLOBAL_INT_ENABLE,
		  chip->vendor.iobase + TPM_INT_ENABLE(chip->vendor.locality));

	priv->irq_tested = false;

	/* Generate an interrupt by having the core call through to
	 * tpm_tis_send
	 */
	if (chip->flags & TPM_CHIP_FLAG_TPM2)
		tpm2_gen_interrupt(chip);
	else
		tpm_gen_interrupt(chip);

	/* tpm_tis_send will either confirm the interrupt is working or it
	 * will call disable_irq which undoes all of the above.
	 */
	if (!chip->vendor.irq) {
		iowrite8(original_int_vec,
			 chip->vendor.iobase +
			     TPM_INT_VECTOR(chip->vendor.locality));
		return 1;
	}

	return 0;
}

/* Try to find the IRQ the TPM is using. This is for legacy x86 systems that
 * do not have ACPI/etc. We typically expect the interrupt to be declared if
 * present.
 */
static void tpm_tis_probe_irq(struct tpm_chip *chip, u32 intmask)
{
	u8 original_int_vec;
	int i;

	original_int_vec = ioread8(chip->vendor.iobase +
				   TPM_INT_VECTOR(chip->vendor.locality));

	if (!original_int_vec) {
		if (IS_ENABLED(CONFIG_X86))
			for (i = 3; i <= 15; i++)
				if (!tpm_tis_probe_irq_single(chip, intmask, 0,
							      i))
					return;
	} else if (!tpm_tis_probe_irq_single(chip, intmask, 0,
					     original_int_vec))
		return;
}

static bool interrupts = true;
module_param(interrupts, bool, 0444);
MODULE_PARM_DESC(interrupts, "Enable interrupts");
=======
static int tpm_tis_init(struct device *dev, struct tpm_info *tpm_info)
{
	struct tpm_tis_tcg_phy *phy;
	int irq = -1;
	int rc;
>>>>>>> temp

	rc = check_acpi_tpm2(dev);
	if (rc)
		return rc;

<<<<<<< HEAD
static int tpm_tis_init(struct device *dev, struct tpm_info *tpm_info,
			acpi_handle acpi_dev_handle)
{
	u32 vendor, intfcaps, intmask;
	int rc, probe;
	struct tpm_chip *chip;
	struct priv_data *priv;

	priv = devm_kzalloc(dev, sizeof(struct priv_data), GFP_KERNEL);
	if (priv == NULL)
		return -ENOMEM;

	chip = tpmm_chip_alloc(dev, &tpm_tis);
	if (IS_ERR(chip))
		return PTR_ERR(chip);

	chip->vendor.priv = priv;
#ifdef CONFIG_ACPI
	chip->acpi_dev_handle = acpi_dev_handle;
#endif

	chip->vendor.iobase = devm_ioremap_resource(dev, &tpm_info->res);
	if (IS_ERR(chip->vendor.iobase))
		return PTR_ERR(chip->vendor.iobase);

	/* Maximum timeouts */
	chip->vendor.timeout_a = TIS_TIMEOUT_A_MAX;
	chip->vendor.timeout_b = TIS_TIMEOUT_B_MAX;
	chip->vendor.timeout_c = TIS_TIMEOUT_C_MAX;
	chip->vendor.timeout_d = TIS_TIMEOUT_D_MAX;

	if (wait_startup(chip, 0) != 0) {
		rc = -ENODEV;
		goto out_err;
	}

	/* Take control of the TPM's interrupt hardware and shut it off */
	intmask = ioread32(chip->vendor.iobase +
			   TPM_INT_ENABLE(chip->vendor.locality));
	intmask |= TPM_INTF_CMD_READY_INT | TPM_INTF_LOCALITY_CHANGE_INT |
		   TPM_INTF_DATA_AVAIL_INT | TPM_INTF_STS_VALID_INT;
	intmask &= ~TPM_GLOBAL_INT_ENABLE;
	iowrite32(intmask,
		  chip->vendor.iobase + TPM_INT_ENABLE(chip->vendor.locality));

	if (request_locality(chip, 0) != 0) {
		rc = -ENODEV;
		goto out_err;
	}

	rc = tpm2_probe(chip);
	if (rc)
		goto out_err;

	vendor = ioread32(chip->vendor.iobase + TPM_DID_VID(0));
	chip->vendor.manufacturer_id = vendor;

	dev_info(dev, "%s TPM (device-id 0x%X, rev-id %d)\n",
		 (chip->flags & TPM_CHIP_FLAG_TPM2) ? "2.0" : "1.2",
		 vendor >> 16, ioread8(chip->vendor.iobase + TPM_RID(0)));

	if (!itpm) {
		probe = probe_itpm(chip);
		if (probe < 0) {
			rc = -ENODEV;
			goto out_err;
		}
		itpm = !!probe;
	}

	if (itpm)
		dev_info(dev, "Intel iTPM workaround enabled\n");


	/* Figure out the capabilities */
	intfcaps =
	    ioread32(chip->vendor.iobase +
		     TPM_INTF_CAPS(chip->vendor.locality));
	dev_dbg(dev, "TPM interface capabilities (0x%x):\n",
		intfcaps);
	if (intfcaps & TPM_INTF_BURST_COUNT_STATIC)
		dev_dbg(dev, "\tBurst Count Static\n");
	if (intfcaps & TPM_INTF_CMD_READY_INT)
		dev_dbg(dev, "\tCommand Ready Int Support\n");
	if (intfcaps & TPM_INTF_INT_EDGE_FALLING)
		dev_dbg(dev, "\tInterrupt Edge Falling\n");
	if (intfcaps & TPM_INTF_INT_EDGE_RISING)
		dev_dbg(dev, "\tInterrupt Edge Rising\n");
	if (intfcaps & TPM_INTF_INT_LEVEL_LOW)
		dev_dbg(dev, "\tInterrupt Level Low\n");
	if (intfcaps & TPM_INTF_INT_LEVEL_HIGH)
		dev_dbg(dev, "\tInterrupt Level High\n");
	if (intfcaps & TPM_INTF_LOCALITY_CHANGE_INT)
		dev_dbg(dev, "\tLocality Change Int Support\n");
	if (intfcaps & TPM_INTF_STS_VALID_INT)
		dev_dbg(dev, "\tSts Valid Int Support\n");
	if (intfcaps & TPM_INTF_DATA_AVAIL_INT)
		dev_dbg(dev, "\tData Avail Int Support\n");

	/* Very early on issue a command to the TPM in polling mode to make
	 * sure it works. May as well use that command to set the proper
	 *  timeouts for the driver.
	 */
	if (tpm_get_timeouts(chip)) {
		dev_err(dev, "Could not get TPM timeouts and durations\n");
		rc = -ENODEV;
		goto out_err;
	}

	/* INTERRUPT Setup */
	init_waitqueue_head(&chip->vendor.read_queue);
	init_waitqueue_head(&chip->vendor.int_queue);
	if (interrupts && tpm_info->irq != -1) {
		if (tpm_info->irq) {
			tpm_tis_probe_irq_single(chip, intmask, IRQF_SHARED,
						 tpm_info->irq);
			if (!chip->vendor.irq)
				dev_err(&chip->dev, FW_BUG
					"TPM interrupt not working, polling instead\n");
		} else
			tpm_tis_probe_irq(chip, intmask);
	}

	if (chip->flags & TPM_CHIP_FLAG_TPM2) {
		rc = tpm2_do_selftest(chip);
		if (rc == TPM2_RC_INITIALIZE) {
			dev_warn(dev, "Firmware has not started TPM\n");
			rc  = tpm2_startup(chip, TPM2_SU_CLEAR);
			if (!rc)
				rc = tpm2_do_selftest(chip);
		}

		if (rc) {
			dev_err(dev, "TPM self test failed\n");
			if (rc > 0)
				rc = -ENODEV;
			goto out_err;
		}
	} else {
		if (tpm_do_selftest(chip)) {
			dev_err(dev, "TPM self test failed\n");
			rc = -ENODEV;
			goto out_err;
		}
	}

	return tpm_chip_register(chip);
out_err:
	tpm_tis_remove(chip);
	return rc;
}

#ifdef CONFIG_PM_SLEEP
static void tpm_tis_reenable_interrupts(struct tpm_chip *chip)
{
	u32 intmask;

	/* reenable interrupts that device may have lost or
	   BIOS/firmware may have disabled */
	iowrite8(chip->vendor.irq, chip->vendor.iobase +
		 TPM_INT_VECTOR(chip->vendor.locality));

	intmask =
	    ioread32(chip->vendor.iobase +
		     TPM_INT_ENABLE(chip->vendor.locality));
=======
	phy = devm_kzalloc(dev, sizeof(struct tpm_tis_tcg_phy), GFP_KERNEL);
	if (phy == NULL)
		return -ENOMEM;

	phy->iobase = devm_ioremap_resource(dev, &tpm_info->res);
	if (IS_ERR(phy->iobase))
		return PTR_ERR(phy->iobase);

	if (interrupts)
		irq = tpm_info->irq;
>>>>>>> temp

	if (itpm || is_itpm(ACPI_COMPANION(dev)))
		phy->priv.flags |= TPM_TIS_ITPM_WORKAROUND;

	return tpm_tis_core_init(dev, &phy->priv, irq, &tpm_tcg,
				 ACPI_HANDLE(dev));
}

static SIMPLE_DEV_PM_OPS(tpm_tis_pm, tpm_pm_suspend, tpm_tis_resume);

static int tpm_tis_pnp_init(struct pnp_dev *pnp_dev,
			    const struct pnp_device_id *pnp_id)
{
	struct tpm_info tpm_info = {};
<<<<<<< HEAD
	acpi_handle acpi_dev_handle = NULL;
=======
>>>>>>> temp
	struct resource *res;

	res = pnp_get_resource(pnp_dev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENODEV;
	tpm_info.res = *res;

	if (pnp_irq_valid(pnp_dev, 0))
		tpm_info.irq = pnp_irq(pnp_dev, 0);
	else
		tpm_info.irq = -1;
<<<<<<< HEAD

	if (pnp_acpi_device(pnp_dev)) {
		if (is_itpm(pnp_acpi_device(pnp_dev)))
			itpm = true;

		acpi_dev_handle = ACPI_HANDLE(&pnp_dev->dev);
	}
=======
>>>>>>> temp

	return tpm_tis_init(&pnp_dev->dev, &tpm_info);
}

static struct pnp_device_id tpm_pnp_tbl[] = {
	{"PNP0C31", 0},		/* TPM */
	{"ATM1200", 0},		/* Atmel */
	{"IFX0102", 0},		/* Infineon */
	{"BCM0101", 0},		/* Broadcom */
	{"BCM0102", 0},		/* Broadcom */
	{"NSC1200", 0},		/* National */
	{"ICO0102", 0},		/* Intel */
	/* Add new here */
	{"", 0},		/* User Specified */
	{"", 0}			/* Terminator */
};
MODULE_DEVICE_TABLE(pnp, tpm_pnp_tbl);

static void tpm_tis_pnp_remove(struct pnp_dev *dev)
{
	struct tpm_chip *chip = pnp_get_drvdata(dev);

	tpm_chip_unregister(chip);
	tpm_tis_remove(chip);
}

static struct pnp_driver tis_pnp_driver = {
	.name = "tpm_tis",
	.id_table = tpm_pnp_tbl,
	.probe = tpm_tis_pnp_init,
	.remove = tpm_tis_pnp_remove,
	.driver	= {
		.pm = &tpm_tis_pm,
	},
};

#define TIS_HID_USR_IDX (ARRAY_SIZE(tpm_pnp_tbl) - 2)
module_param_string(hid, tpm_pnp_tbl[TIS_HID_USR_IDX].id,
		    sizeof(tpm_pnp_tbl[TIS_HID_USR_IDX].id), 0444);
MODULE_PARM_DESC(hid, "Set additional specific HID for this driver to probe");
<<<<<<< HEAD

#ifdef CONFIG_ACPI
static int tpm_check_resource(struct acpi_resource *ares, void *data)
{
	struct tpm_info *tpm_info = (struct tpm_info *) data;
	struct resource res;

	if (acpi_dev_resource_interrupt(ares, 0, &res))
		tpm_info->irq = res.start;
	else if (acpi_dev_resource_memory(ares, &res)) {
		tpm_info->res = res;
		tpm_info->res.name = NULL;
	}

	return 1;
}
=======

static struct platform_device *force_pdev;
>>>>>>> temp

static int tpm_tis_plat_probe(struct platform_device *pdev)
{
<<<<<<< HEAD
	struct acpi_table_tpm2 *tbl;
	acpi_status st;
	struct list_head resources;
	struct tpm_info tpm_info = {};
	int ret;

	st = acpi_get_table(ACPI_SIG_TPM2, 1,
			    (struct acpi_table_header **) &tbl);
	if (ACPI_FAILURE(st) || tbl->header.length < sizeof(*tbl)) {
		dev_err(&acpi_dev->dev,
			FW_BUG "failed to get TPM2 ACPI table\n");
		return -EINVAL;
	}

	if (tbl->start_method != ACPI_TPM2_MEMORY_MAPPED)
=======
	struct tpm_info tpm_info = {};
	struct resource *res;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "no memory resource defined\n");
>>>>>>> temp
		return -ENODEV;
	}
	tpm_info.res = *res;

<<<<<<< HEAD
	INIT_LIST_HEAD(&resources);
	tpm_info.irq = -1;
	ret = acpi_dev_get_resources(acpi_dev, &resources, tpm_check_resource,
				     &tpm_info);
	if (ret < 0)
		return ret;

	acpi_dev_free_resource_list(&resources);

	if (resource_type(&tpm_info.res) != IORESOURCE_MEM) {
		dev_err(&acpi_dev->dev,
			FW_BUG "TPM2 ACPI table does not define a memory resource\n");
		return -EINVAL;
	}

	if (is_itpm(acpi_dev))
		itpm = true;
=======
	tpm_info.irq = platform_get_irq(pdev, 0);
	if (tpm_info.irq <= 0) {
		if (pdev != force_pdev)
			tpm_info.irq = -1;
		else
			/* When forcing auto probe the IRQ */
			tpm_info.irq = 0;
	}
>>>>>>> temp

	return tpm_tis_init(&pdev->dev, &tpm_info);
}

static int tpm_tis_plat_remove(struct platform_device *pdev)
{
	struct tpm_chip *chip = dev_get_drvdata(&pdev->dev);

	tpm_chip_unregister(chip);
	tpm_tis_remove(chip);

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id tis_of_platform_match[] = {
	{.compatible = "tcg,tpm-tis-mmio"},
	{},
};
MODULE_DEVICE_TABLE(of, tis_of_platform_match);
#endif

static struct platform_device *force_pdev;

static int tpm_tis_plat_probe(struct platform_device *pdev)
{
	struct tpm_info tpm_info = {};
	struct resource *res;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "no memory resource defined\n");
		return -ENODEV;
	}
	tpm_info.res = *res;

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (res) {
		tpm_info.irq = res->start;
	} else {
		if (pdev == force_pdev)
			tpm_info.irq = -1;
		else
			/* When forcing auto probe the IRQ */
			tpm_info.irq = 0;
	}

	return tpm_tis_init(&pdev->dev, &tpm_info, NULL);
}

static int tpm_tis_plat_remove(struct platform_device *pdev)
{
	struct tpm_chip *chip = dev_get_drvdata(&pdev->dev);

	tpm_chip_unregister(chip);
	tpm_tis_remove(chip);

	return 0;
}

static struct platform_driver tis_drv = {
	.probe = tpm_tis_plat_probe,
	.remove = tpm_tis_plat_remove,
	.driver = {
		.name		= "tpm_tis",
		.pm		= &tpm_tis_pm,
		.of_match_table = of_match_ptr(tis_of_platform_match),
		.acpi_match_table = ACPI_PTR(tpm_acpi_tbl),
	},
};

<<<<<<< HEAD
static bool force;
#ifdef CONFIG_X86
module_param(force, bool, 0444);
MODULE_PARM_DESC(force, "Force device probe rather than using ACPI entry");
#endif

=======
>>>>>>> temp
static int tpm_tis_force_device(void)
{
	struct platform_device *pdev;
	static const struct resource x86_resources[] = {
		{
			.start = 0xFED40000,
			.end = 0xFED40000 + TIS_MEM_LEN - 1,
			.flags = IORESOURCE_MEM,
		},
	};

	if (!force)
		return 0;

	/* The driver core will match the name tpm_tis of the device to
	 * the tpm_tis platform driver and complete the setup via
	 * tpm_tis_plat_probe
	 */
	pdev = platform_device_register_simple("tpm_tis", -1, x86_resources,
					       ARRAY_SIZE(x86_resources));
	if (IS_ERR(pdev))
		return PTR_ERR(pdev);
	force_pdev = pdev;

	return 0;
}

static int __init init_tis(void)
{
	int rc;

	rc = tpm_tis_force_device();
	if (rc)
		goto err_force;

	rc = platform_driver_register(&tis_drv);
	if (rc)
		goto err_platform;

<<<<<<< HEAD
#ifdef CONFIG_ACPI
	rc = acpi_bus_register_driver(&tis_acpi_driver);
	if (rc)
		goto err_acpi;
#endif

	if (IS_ENABLED(CONFIG_PNP)) {
		rc = pnp_register_driver(&tis_pnp_driver);
		if (rc)
			goto err_pnp;
	}

	return 0;

err_pnp:
#ifdef CONFIG_ACPI
	acpi_bus_unregister_driver(&tis_acpi_driver);
err_acpi:
#endif
	platform_device_unregister(force_pdev);
=======

	if (IS_ENABLED(CONFIG_PNP)) {
		rc = pnp_register_driver(&tis_pnp_driver);
		if (rc)
			goto err_pnp;
	}

	return 0;

err_pnp:
	platform_driver_unregister(&tis_drv);
>>>>>>> temp
err_platform:
	if (force_pdev)
		platform_device_unregister(force_pdev);
err_force:
	return rc;
}

static void __exit cleanup_tis(void)
{
	pnp_unregister_driver(&tis_pnp_driver);
<<<<<<< HEAD
#ifdef CONFIG_ACPI
	acpi_bus_unregister_driver(&tis_acpi_driver);
#endif
=======
>>>>>>> temp
	platform_driver_unregister(&tis_drv);

	if (force_pdev)
		platform_device_unregister(force_pdev);
}

module_init(init_tis);
module_exit(cleanup_tis);
MODULE_AUTHOR("Leendert van Doorn (leendert@watson.ibm.com)");
MODULE_DESCRIPTION("TPM Driver");
MODULE_VERSION("2.0");
MODULE_LICENSE("GPL");
