#
# Makefile for the Linux kernel ubuntu supplied third-party device drivers.
#

#
# NOTE: to allow drivers to be added and removed without causing merge
# collisions you should add new entries in the middle of the six lines
# of ## at the bottom of the list.  Always add three lines of ## above
# your new entry and maintain the six lines below.
#

##
##
##
<<<<<<< HEAD
obj-$(CONFIG_DM_RAID45)		+= dm-raid4-5/
##
##
##
ifneq ($(filter $(ARCH), i386 x86_64),)
=======
##
##
##
ifeq ($(ARCH),x86)
>>>>>>> temp
obj-y                         += vbox/
endif
##
##
##
<<<<<<< HEAD
obj-$(CONFIG_DRM_I915_BPO)	+= i915/
##
##
##
=======
>>>>>>> temp
obj-$(CONFIG_HIO)             += hio/
##
##
##
<<<<<<< HEAD
ifeq ($(filter $(ARCH), s390),)
=======
ifeq ($(ARCH),x86)
>>>>>>> temp
obj-y				+= xr-usb-serial/
endif
##
##
##
<<<<<<< HEAD
obj-$(CONFIG_WLAN_VENDOR_RSI) += rsi/
##
##
##
obj-$(CONFIG_OPENNSL) += opennsl/
##
##
##
##
obj-$(CONFIG_BNXT_BPO) += bnxt/
##
##
##
obj-$(CONFIG_CAN_HMS_USB) += ixxat/
##
##
##
=======
>>>>>>> temp
##
##
##

# This is a stupid trick to get kbuild to create ubuntu/built-in.o
obj- += foo.o
