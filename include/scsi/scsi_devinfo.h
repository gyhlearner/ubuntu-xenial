/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _SCSI_SCSI_DEVINFO_H
#define _SCSI_SCSI_DEVINFO_H
/*
 * Flags for SCSI devices that need special treatment
 */
<<<<<<< HEAD
#define BLIST_NOLUN     	0x001	/* Only scan LUN 0 */
#define BLIST_FORCELUN  	0x002	/* Known to have LUNs, force scanning,
					   deprecated: Use max_luns=N */
#define BLIST_BORKEN    	0x004	/* Flag for broken handshaking */
#define BLIST_KEY       	0x008	/* unlock by special command */
#define BLIST_SINGLELUN 	0x010	/* Do not use LUNs in parallel */
#define BLIST_NOTQ		0x020	/* Buggy Tagged Command Queuing */
#define BLIST_SPARSELUN 	0x040	/* Non consecutive LUN numbering */
#define BLIST_MAX5LUN		0x080	/* Avoid LUNS >= 5 */
#define BLIST_ISROM     	0x100	/* Treat as (removable) CD-ROM */
#define BLIST_LARGELUN		0x200	/* LUNs past 7 on a SCSI-2 device */
#define BLIST_INQUIRY_36	0x400	/* override additional length field */
#define BLIST_INQUIRY_58	0x800	/* ... for broken inquiry responses */
#define BLIST_NOSTARTONADD	0x1000	/* do not do automatic start on add */
#define BLIST_MS_SKIP_PAGE_08	0x2000	/* do not send ms page 0x08 */
#define BLIST_MS_SKIP_PAGE_3F	0x4000	/* do not send ms page 0x3f */
#define BLIST_USE_10_BYTE_MS	0x8000	/* use 10 byte ms before 6 byte ms */
#define BLIST_MS_192_BYTES_FOR_3F	0x10000	/*  192 byte ms page 0x3f request */
#define BLIST_REPORTLUN2	0x20000	/* try REPORT_LUNS even for SCSI-2 devs
 					   (if HBA supports more than 8 LUNs) */
#define BLIST_NOREPORTLUN	0x40000	/* don't try REPORT_LUNS scan (SCSI-3 devs) */
#define BLIST_NOT_LOCKABLE	0x80000	/* don't use PREVENT-ALLOW commands */
#define BLIST_NO_ULD_ATTACH	0x100000 /* device is actually for RAID config */
#define BLIST_SELECT_NO_ATN	0x200000 /* select without ATN */
#define BLIST_RETRY_HWERROR	0x400000 /* retry HARDWARE_ERROR */
#define BLIST_MAX_512		0x800000 /* maximum 512 sector cdb length */
#define BLIST_ATTACH_PQ3	0x1000000 /* Scan: Attach to PQ3 devices */
#define BLIST_NO_DIF		0x2000000 /* Disable T10 PI (DIF) */
#define BLIST_SKIP_VPD_PAGES	0x4000000 /* Ignore SBC-3 VPD pages */
#define BLIST_SCSI3LUN		0x8000000 /* Scan more than 256 LUNs
					     for sequential scan */
#define BLIST_TRY_VPD_PAGES	0x10000000 /* Attempt to read VPD pages */
#define BLIST_NO_RSOC		0x20000000 /* don't try to issue RSOC */
#define BLIST_MAX_1024		0x40000000 /* maximum 1024 sector cdb length */
#define BLIST_SYNC_ALUA		0x80000000 /* Synchronous ALUA commands */
=======

/* Only scan LUN 0 */
#define BLIST_NOLUN		((__force blist_flags_t)(1 << 0))
/* Known to have LUNs, force scanning.
 * DEPRECATED: Use max_luns=N */
#define BLIST_FORCELUN		((__force blist_flags_t)(1 << 1))
/* Flag for broken handshaking */
#define BLIST_BORKEN		((__force blist_flags_t)(1 << 2))
/* unlock by special command */
#define BLIST_KEY		((__force blist_flags_t)(1 << 3))
/* Do not use LUNs in parallel */
#define BLIST_SINGLELUN		((__force blist_flags_t)(1 << 4))
/* Buggy Tagged Command Queuing */
#define BLIST_NOTQ		((__force blist_flags_t)(1 << 5))
/* Non consecutive LUN numbering */
#define BLIST_SPARSELUN		((__force blist_flags_t)(1 << 6))
/* Avoid LUNS >= 5 */
#define BLIST_MAX5LUN		((__force blist_flags_t)(1 << 7))
/* Treat as (removable) CD-ROM */
#define BLIST_ISROM		((__force blist_flags_t)(1 << 8))
/* LUNs past 7 on a SCSI-2 device */
#define BLIST_LARGELUN		((__force blist_flags_t)(1 << 9))
/* override additional length field */
#define BLIST_INQUIRY_36	((__force blist_flags_t)(1 << 10))
/* do not do automatic start on add */
#define BLIST_NOSTARTONADD	((__force blist_flags_t)(1 << 12))
/* try REPORT_LUNS even for SCSI-2 devs (if HBA supports more than 8 LUNs) */
#define BLIST_REPORTLUN2	((__force blist_flags_t)(1 << 17))
/* don't try REPORT_LUNS scan (SCSI-3 devs) */
#define BLIST_NOREPORTLUN	((__force blist_flags_t)(1 << 18))
/* don't use PREVENT-ALLOW commands */
#define BLIST_NOT_LOCKABLE	((__force blist_flags_t)(1 << 19))
/* device is actually for RAID config */
#define BLIST_NO_ULD_ATTACH	((__force blist_flags_t)(1 << 20))
/* select without ATN */
#define BLIST_SELECT_NO_ATN	((__force blist_flags_t)(1 << 21))
/* retry HARDWARE_ERROR */
#define BLIST_RETRY_HWERROR	((__force blist_flags_t)(1 << 22))
/* maximum 512 sector cdb length */
#define BLIST_MAX_512		((__force blist_flags_t)(1 << 23))
/* Disable T10 PI (DIF) */
#define BLIST_NO_DIF		((__force blist_flags_t)(1 << 25))
/* Ignore SBC-3 VPD pages */
#define BLIST_SKIP_VPD_PAGES	((__force blist_flags_t)(1 << 26))
/* Attempt to read VPD pages */
#define BLIST_TRY_VPD_PAGES	((__force blist_flags_t)(1 << 28))
/* don't try to issue RSOC */
#define BLIST_NO_RSOC		((__force blist_flags_t)(1 << 29))
/* maximum 1024 sector cdb length */
#define BLIST_MAX_1024		((__force blist_flags_t)(1 << 30))
/* Use UNMAP limit for WRITE SAME */
#define BLIST_UNMAP_LIMIT_WS	((__force blist_flags_t)(1 << 31))
>>>>>>> temp

#endif
