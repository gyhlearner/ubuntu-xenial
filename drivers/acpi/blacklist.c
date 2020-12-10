/*
 *  blacklist.c
 *
 *  Check to see if the given machine has a known bad ACPI BIOS
 *  or if the BIOS is too old.
 *  Check given machine against acpi_rev_dmi_table[].
 *
 *  Copyright (C) 2004 Len Brown <len.brown@intel.com>
 *  Copyright (C) 2002 Andy Grover <andrew.grover@intel.com>
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or (at
 *  your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/acpi.h>
#include <linux/dmi.h>

#include "internal.h"

static const struct dmi_system_id acpi_rev_dmi_table[] __initconst;

/*
 * POLICY: If *anything* doesn't work, put it on the blacklist.
 *	   If they are critical errors, mark it critical, and abort driver load.
 */
static struct acpi_platform_list acpi_blacklist[] __initdata = {
	/* Compaq Presario 1700 */
	{"PTLTD ", "  DSDT  ", 0x06040000, ACPI_SIG_DSDT, less_than_or_equal,
	 "Multiple problems", 1},
	/* Sony FX120, FX140, FX150? */
	{"SONY  ", "U0      ", 0x20010313, ACPI_SIG_DSDT, less_than_or_equal,
	 "ACPI driver problem", 1},
	/* Compaq Presario 800, Insyde BIOS */
	{"INT440", "SYSFexxx", 0x00001001, ACPI_SIG_DSDT, less_than_or_equal,
	 "Does not use _REG to protect EC OpRegions", 1},
	/* IBM 600E - _ADR should return 7, but it returns 1 */
	{"IBM   ", "TP600E  ", 0x00000105, ACPI_SIG_DSDT, less_than_or_equal,
	 "Incorrect _ADR", 1},

	{ }
};

int __init acpi_blacklisted(void)
{
	int i;
	int blacklisted = 0;

	i = acpi_match_platform_list(acpi_blacklist);
	if (i >= 0) {
		pr_err(PREFIX "Vendor \"%6.6s\" System \"%8.8s\" Revision 0x%x has a known ACPI BIOS problem.\n",
		       acpi_blacklist[i].oem_id,
		       acpi_blacklist[i].oem_table_id,
		       acpi_blacklist[i].oem_revision);

		pr_err(PREFIX "Reason: %s. This is a %s error\n",
		       acpi_blacklist[i].reason,
		       (acpi_blacklist[i].data ?
			"non-recoverable" : "recoverable"));

		blacklisted = acpi_blacklist[i].data;
	}

	(void)early_acpi_osi_init();
	dmi_check_system(acpi_rev_dmi_table);

	return blacklisted;
}
#ifdef CONFIG_DMI
#ifdef CONFIG_ACPI_REV_OVERRIDE_POSSIBLE
static int __init dmi_enable_rev_override(const struct dmi_system_id *d)
{
	printk(KERN_NOTICE PREFIX "DMI detected: %s (force ACPI _REV to 5)\n",
	       d->ident);
	acpi_rev_override_setup(NULL);
	return 0;
}
#endif

static const struct dmi_system_id acpi_rev_dmi_table[] __initconst = {
#ifdef CONFIG_ACPI_REV_OVERRIDE_POSSIBLE
	/*
	 * DELL XPS 13 (2015) switches sound between HDA and I2S
	 * depending on the ACPI _REV callback. If userspace supports
	 * I2S sufficiently (or if you do not care about sound), you
	 * can safely disable this quirk.
	 */
	{
	 .callback = dmi_enable_rev_override,
	 .ident = "DELL XPS 13 (2015)",
	 .matches = {
		      DMI_MATCH(DMI_SYS_VENDOR, "Dell Inc."),
		      DMI_MATCH(DMI_PRODUCT_NAME, "XPS 13 9343"),
		},
	},
	{
	 .callback = dmi_enable_rev_override,
	 .ident = "DELL Precision 5520",
	 .matches = {
		      DMI_MATCH(DMI_SYS_VENDOR, "Dell Inc."),
		      DMI_MATCH(DMI_PRODUCT_NAME, "Precision 5520"),
		},
	},
	{
	 .callback = dmi_enable_rev_override,
	 .ident = "DELL Precision 3520",
	 .matches = {
		      DMI_MATCH(DMI_SYS_VENDOR, "Dell Inc."),
		      DMI_MATCH(DMI_PRODUCT_NAME, "Precision 3520"),
		},
	},
<<<<<<< HEAD

	/*
	 * The following Lenovo models have a broken workaround in the
	 * acpi_video backlight implementation to meet the Windows 8
	 * requirement of 101 backlight levels. Reverting to pre-Win8
	 * behavior fixes the problem.
	 */
	{
	.callback = dmi_disable_osi_win8,
	.ident = "Lenovo ThinkPad L430",
	.matches = {
		     DMI_MATCH(DMI_SYS_VENDOR, "LENOVO"),
		     DMI_MATCH(DMI_PRODUCT_VERSION, "ThinkPad L430"),
		},
	},
	{
	.callback = dmi_disable_osi_win8,
	.ident = "Lenovo ThinkPad T430",
	.matches = {
		     DMI_MATCH(DMI_SYS_VENDOR, "LENOVO"),
		     DMI_MATCH(DMI_PRODUCT_VERSION, "ThinkPad T430"),
		},
	},
	{
	.callback = dmi_disable_osi_win8,
	.ident = "Lenovo ThinkPad T430s",
	.matches = {
		     DMI_MATCH(DMI_SYS_VENDOR, "LENOVO"),
		     DMI_MATCH(DMI_PRODUCT_VERSION, "ThinkPad T430s"),
		},
	},
	{
	.callback = dmi_disable_osi_win8,
	.ident = "Lenovo ThinkPad T530",
	.matches = {
		     DMI_MATCH(DMI_SYS_VENDOR, "LENOVO"),
		     DMI_MATCH(DMI_PRODUCT_VERSION, "ThinkPad T530"),
		},
	},
	{
	.callback = dmi_disable_osi_win8,
	.ident = "Lenovo ThinkPad W530",
	.matches = {
		     DMI_MATCH(DMI_SYS_VENDOR, "LENOVO"),
		     DMI_MATCH(DMI_PRODUCT_VERSION, "ThinkPad W530"),
		},
	},
	{
	.callback = dmi_disable_osi_win8,
	.ident = "Lenovo ThinkPad X1 Carbon",
	.matches = {
		     DMI_MATCH(DMI_SYS_VENDOR, "LENOVO"),
		     DMI_MATCH(DMI_PRODUCT_VERSION, "ThinkPad X1 Carbon"),
		},
	},
	{
	.callback = dmi_disable_osi_win8,
	.ident = "Lenovo ThinkPad X230",
	.matches = {
		     DMI_MATCH(DMI_SYS_VENDOR, "LENOVO"),
		     DMI_MATCH(DMI_PRODUCT_VERSION, "ThinkPad X230"),
		},
	},
	{
	.callback = dmi_disable_osi_win8,
	.ident = "Lenovo ThinkPad Edge E330",
	.matches = {
		     DMI_MATCH(DMI_SYS_VENDOR, "LENOVO"),
		     DMI_MATCH(DMI_PRODUCT_VERSION, "ThinkPad Edge E330"),
		},
	},

	/*
	 * BIOS invocation of _OSI(Linux) is almost always a BIOS bug.
	 * Linux ignores it, except for the machines enumerated below.
	 */

=======
>>>>>>> temp
	/*
	 * Resolves a quirk with the Dell Latitude 3350 that
	 * causes the ethernet adapter to not function.
	 */
	{
	 .callback = dmi_enable_rev_override,
	 .ident = "DELL Latitude 3350",
	 .matches = {
		      DMI_MATCH(DMI_SYS_VENDOR, "Dell Inc."),
		      DMI_MATCH(DMI_PRODUCT_NAME, "Latitude 3350"),
		},
	},
	{
	 .callback = dmi_enable_rev_override,
	 .ident = "DELL Inspiron 7537",
	 .matches = {
		      DMI_MATCH(DMI_SYS_VENDOR, "Dell Inc."),
		      DMI_MATCH(DMI_PRODUCT_NAME, "Inspiron 7537"),
		},
	},
	/*
	 * Resolves a quirk with the Dell Latitude 3350 that
	 * causes the ethernet adapter to not function.
	 */
	{
	 .callback = dmi_enable_rev_override,
	 .ident = "DELL Latitude 3350",
	 .matches = {
		      DMI_MATCH(DMI_SYS_VENDOR, "Dell Inc."),
		      DMI_MATCH(DMI_PRODUCT_NAME, "Latitude 3350"),
		},
	},
	{
	 .callback = dmi_enable_rev_override,
	 .ident = "DELL Precision 5520",
	 .matches = {
		      DMI_MATCH(DMI_SYS_VENDOR, "Dell Inc."),
		      DMI_MATCH(DMI_PRODUCT_NAME, "Precision 5520"),
		},
	},
	{
	 .callback = dmi_enable_rev_override,
	 .ident = "DELL Precision 3520",
	 .matches = {
		      DMI_MATCH(DMI_SYS_VENDOR, "Dell Inc."),
		      DMI_MATCH(DMI_PRODUCT_NAME, "Precision 3520"),
		},
	},
#endif
	{}
};

#endif /* CONFIG_DMI */
