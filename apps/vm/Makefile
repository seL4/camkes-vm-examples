#
# Copyright 2014, NICTA
#
# This software may be distributed and modified according to the terms of
# the BSD 2-Clause license. Note that NO WARRANTY is provided.
# See "LICENSE_BSD2.txt" for details.
#
# @TAG(NICTA_BSD)
#

TARGETS := $(notdir ${SOURCE_DIR}).cdl
ADL := vm.camkes

pilot_CFILES := \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/components/pilot/src/*.c)) \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/components/pilot/src/aeslib/*.c))

pilot_HFILES := \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/components/pilot/src/aeslib/*.h))

uart_CFILES := \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/components/uart/src/*.c))

gpio_HFILES := \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/include/*.h))
gpio_CFILES := \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/components/gpio/src/*.c))

pwm_CFILES := \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/components/pwm/src/*.c))

can_HFILES := \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/include/*.h)) \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/components/can/include/*.h))
can_CFILES := \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/components/can/src/*.c))

spi_HFILES := \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/components/spi/include/*.h))
spi_CFILES := \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/components/spi/src/*.c))

clk_HFILES := \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/components/clk/include/*.h))
clk_CFILES := \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/components/clk/src/*.c))

timer_CFILES := \
    $(patsubst ${SOURCE_DIR}/%,%,$(wildcard ${SOURCE_DIR}/components/timer/src/*.c))

uart_LIBS := platsupport
gpio_LIBS := platsupport
pwm_LIBS  := platsupport
spi_LIBS  := platsupport
clk_LIBS  := platsupport
timer_LIBS  := platsupport

VM_CFILES   := $(patsubst $(SOURCE_DIR)/%,%,$(wildcard $(SOURCE_DIR)/components/VM/src/*.c))
VM_OFILES   := archive.o
VM_HFILES := $(patsubst ${SOURCE_DIR}/%,%,$(wildcard $(SOURCE_DIR)/components/VM/src/*.h))
VM_LIBS :=  sel4 sel4camkes sel4muslccamkes sel4vka sel4allocman \
            platsupport sel4platsupport sel4vspace elf \
            sel4utils sel4simple utils  sel4simple-default cpio \
            sel4arm-vmm sel4sync sel4debug sel4dma usbdrivers

include ${PWD}/tools/camkes/camkes.mk

ifeq (${CONFIG_VM_ROOTFS_MMCBLK0P2},y)
ROOTFS := mmcblk0p2
else ifeq (${CONFIG_VM_ROOTFS_MMCBLK1P2},y)
ROOTFS := mmcblk1p2
else
$(error Unknown root filesystem)
endif

ifeq (${CONFIG_VM_VUSB},y)
DEVICE_TREE_SRC := ${SOURCE_DIR}/linux/linux-secure-vusb-dtb
else
DEVICE_TREE_SRC := ${SOURCE_DIR}/linux/linux-secure-dtb
endif

$(STAGE_DIR)/linux/linux-dtb:
	mkdir -p $(dir $@)
	sed "s/root=\/dev\/mmcblk1p2/root=\/dev\/${ROOTFS}/g" $(DEVICE_TREE_SRC) > $@

COMPONENTS := ${SOURCE_DIR}/linux/linux $(STAGE_DIR)/linux/linux-dtb

${BUILD_DIR}/src/vm/static/archive.o: ${COMPONENTS}
	$(Q)mkdir -p $(dir $@)
	${COMMON_PATH}/files_to_obj.sh $@ _cpio_archive $^

