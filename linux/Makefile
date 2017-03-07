#
# Copyright 2017, Data61
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# ABN 41 687 119 230.
# This software may be distributed and modified according to the terms of
# the BSD 2-Clause license. Note that NO WARRANTY is provided.
# See "LICENSE_BSD2.txt" for details.
# @TAG(DATA61_BSD)
#

supported= false

ifeq ($(LINUX_FILE), linux-tk1-debian)
supported:=true
SRC_FILE := $(BUILDROOT_DIR)/output/build/linux-custom/arch/arm/boot/Image
DEST_FILE := linux-tk1-debian
endif
ifeq ($(LINUX_FILE), linux-tk1-initrd)
supported:=true
SRC_FILE := $(BUILDROOT_DIR)/output/build/linux-custom/arch/arm/boot/Image
DEST_FILE := linux-tk1-initrd
endif
ifeq ($(LINUX_FILE), linux-tk1-dtb-nonsecured)
supported:=true
SRC_FILE := $(BUILDROOT_DIR)/output/images/tegra124-jetson-tk1-sel4vm.dtb
DEST_FILE := linux-tk1-dtb-nonsecured
endif
ifeq ($(LINUX_FILE), linux-tk1-dtb-secure)
supported:=true
SRC_FILE := $(BUILDROOT_DIR)/output/images/tegra124-jetson-tk1-sel4vm-secure.dtb
DEST_FILE := linux-tk1-dtb-secure
endif
ifneq (${supported},true)
$(error Specify LINUX_FILE.  Only supported files are: linux-tk1-debian, \
		linux-tk1-initrd, linux-tk1-dtb-nonsecured, linux-tk1-dtb-secure)
endif


.PHONY: copy_buildroot
copy_buildroot:
	@if [ ! -e "${BUILDROOT_DIR}" ]; then \
		echo "Specify a BUILDROOT_DIR to ${BUILDROOT_DIR} buildroot"; \
		exit 1; \
	else \
		echo "cp ${SRC_FILE} ${DEST_FILE}"; \
		cp ${SRC_FILE} ${DEST_FILE}; \
	fi
