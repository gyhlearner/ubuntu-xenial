KBUILD_EXTMOD=${srctree}/ubuntu/vbox
# $Id: Makefile.module.kms $
## @file
# VirtualBox Guest Additions Module Makefile.
#
# (For 2.6.x this file must be 'Makefile'!)
#

#
<<<<<<< HEAD
# Copyright (C) 2006-2010 Oracle Corporation
=======
# Copyright (C) 2006-2017 Oracle Corporation
>>>>>>> temp
#
# This file is part of VirtualBox Open Source Edition (OSE), as
# available from http://www.virtualbox.org. This file is free software;
# you can redistribute it and/or modify it under the terms of the GNU
# General Public License (GPL) as published by the Free Software
# Foundation, in version 2 as it comes in the "COPYING" file of the
# VirtualBox OSE distribution. VirtualBox OSE is distributed in the
# hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
#

# Linux kbuild sets this to our source directory if we are called from there
obj ?= $(CURDIR)
include $(obj)/Makefile.include.header

<<<<<<< HEAD
MOD_NAME   = vboxvideo

ifeq ($(filter 1.% 2.% 3.0.% 3.1.% 3.2.% 3.3.% 3.4.% 3.5.% 3.6.% 3.7.% \
3.8.% 3.9.% 3.10.%,$(KERNELRELEASE)),)
MOD_OBJS   = HGSMIBase.o HGSMICommon.o HGSMIMemAlloc.o heapoffset.o \
             Modesetting.o vbox_drv.o vbox_fb.o vbox_irq.o vbox_main.o \
             vbox_mode.o vbox_ttm.o VBVABase.o
else
MOD_OBJS   = vbox_dummy.o
endif

ifneq ($(wildcard $(KBUILD_EXTMOD)/vboxvideo),)
 MANGLING := $(KBUILD_EXTMOD)/vboxvideo/include/VBox/VBoxGuestMangling.h
else
 MANGLING := $(KBUILD_EXTMOD)/include/VBox/VBoxGuestMangling.h
endif
MOD_CFLAGS = -Wno-declaration-after-statement -fshort-wchar -include $(MANGLING)
MOD_INCL   = $(addprefix -I$(KBUILD_EXTMOD),/ /include)
# What on earth is this?
MOD_INCL  += $(addprefix -I$(KBUILD_EXTMOD)/vboxvideo,/ /include)
MOD_INCL  += -Iinclude/drm
MOD_DEFS  := -DRT_OS_LINUX -DIN_RING0 -DIN_RT_R0 \
	     -DIN_SUP_R0 -DVBOX -DVBOX_WITH_HGCM -DLOG_TO_BACKDOOR -DIN_MODULE \
	     -DIN_GUEST_R0
# our module does not export any symbol
MOD_DEFS  += -DRT_NO_EXPORT_SYMBOL
ifeq ($(BUILD_TARGET_ARCH),amd64)
 MOD_DEFS += -DRT_ARCH_AMD64 -DVBOX_WITH_64_BITS_GUESTS
else
 MOD_DEFS += -DRT_ARCH_X86
endif
MOD_CLEAN  = . linux r0drv r0drv/linux

include $(obj)/Makefile.include.footer
=======
BUILD =

# We want to build on Linux 3.11 and later and on all EL 7 kernels.
ifneq ($(filter-out 1.% 2.% 3.0.% 3.1.% 3.2.% 3.3.% 3.4.% 3.5.% 3.6.% 3.7.% \
                    3.8.% 3.9.% 3.10.%,$(KERN_VER)),)
 BUILD = 1
endif
ifeq ($(filter-out %.el7.x86_64,$(KERN_VER)),)
 BUILD = 1
endif

ifneq ($(BUILD),)

MOD_NAME   = vboxvideo
MOD_OBJS   = hgsmi_base.o \
             modesetting.o vbox_drv.o vbox_fb.o vbox_irq.o vbox_main.o \
             vbox_mode.o vbox_ttm.o vbva_base.o vbox_prime.o vbox_hgsmi.o
MOD_INCL   = -I$(KBUILD_EXTMOD) -Iinclude/drm

include $(obj)/Makefile.include.footer

else  # ! wildcard $(KERN_INCL)/drm/drm_rect.h

 all:
 install:
 clean:

endif  # ! wildcard $(KERN_INCL)/drm/drm_rect.h
>>>>>>> temp
