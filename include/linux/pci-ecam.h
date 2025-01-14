/*
 * Copyright 2016 Broadcom
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation (the "GPL").
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License version 2 (GPLv2) for more details.
 *
 * You should have received a copy of the GNU General Public License
 * version 2 (GPLv2) along with this source code.
 */
#ifndef DRIVERS_PCI_ECAM_H
#define DRIVERS_PCI_ECAM_H

<<<<<<< HEAD
=======
#include <linux/pci.h>
>>>>>>> temp
#include <linux/kernel.h>
#include <linux/platform_device.h>

/*
 * struct to hold pci ops and bus shift of the config window
 * for a PCI controller.
 */
struct pci_config_window;
struct pci_ecam_ops {
	unsigned int			bus_shift;
	struct pci_ops			pci_ops;
	int				(*init)(struct pci_config_window *);
};

/*
 * struct to hold the mappings of a config space window. This
 * is expected to be used as sysdata for PCI controllers that
 * use ECAM.
 */
struct pci_config_window {
	struct resource			res;
	struct resource			busr;
	void				*priv;
	struct pci_ecam_ops		*ops;
	union {
		void __iomem		*win;	/* 64-bit single mapping */
		void __iomem		**winp; /* 32-bit per-bus mapping */
	};
	struct device			*parent;/* ECAM res was from this dev */
};

/* create and free pci_config_window */
struct pci_config_window *pci_ecam_create(struct device *dev,
		struct resource *cfgres, struct resource *busr,
		struct pci_ecam_ops *ops);
void pci_ecam_free(struct pci_config_window *cfg);

/* map_bus when ->sysdata is an instance of pci_config_window */
void __iomem *pci_ecam_map_bus(struct pci_bus *bus, unsigned int devfn,
			       int where);
/* default ECAM ops */
extern struct pci_ecam_ops pci_generic_ecam_ops;

<<<<<<< HEAD
#ifdef CONFIG_PCI_HOST_GENERIC
=======
#if defined(CONFIG_ACPI) && defined(CONFIG_PCI_QUIRKS)
extern struct pci_ecam_ops pci_32b_ops;		/* 32-bit accesses only */
extern struct pci_ecam_ops hisi_pcie_ops;	/* HiSilicon */
extern struct pci_ecam_ops thunder_pem_ecam_ops; /* Cavium ThunderX 1.x & 2.x */
extern struct pci_ecam_ops pci_thunder_ecam_ops; /* Cavium ThunderX 1.x */
extern struct pci_ecam_ops xgene_v1_pcie_ecam_ops; /* APM X-Gene PCIe v1 */
extern struct pci_ecam_ops xgene_v2_pcie_ecam_ops; /* APM X-Gene PCIe v2.x */
#endif

#ifdef CONFIG_PCI_HOST_COMMON
>>>>>>> temp
/* for DT-based PCI controllers that support ECAM */
int pci_host_common_probe(struct platform_device *pdev,
			  struct pci_ecam_ops *ops);
#endif
#endif
