#
# Copyright 2017, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
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


.PHONY: copy_buildroot
copy_buildroot:
	@if [ ! -e "${BUILDROOT_DIR}" ] || [ "${supported}" == "false" ]; then \
		echo "Specify a BUILDROOT_DIR to ${BUILDROOT_DIR} buildroot"; \
		echo "Specify LINUX_FILE.  Only supported files are: linux-tk1-debian, \
linux-tk1-initrd";\
		exit 1; \
	else \
		echo "cp ${SRC_FILE} ${DEST_FILE}"; \
		cp ${SRC_FILE} ${DEST_FILE}; \
	fi
