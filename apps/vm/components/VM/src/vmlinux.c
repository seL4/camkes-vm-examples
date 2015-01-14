/*
 * Copyright 2014, NICTA
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(NICTA_BSD)
 */

#include "vmlinux.h"

#include <string.h>

#include <vka/capops.h>

#include <sel4arm-vmm/vm.h>
#include <sel4arm-vmm/images.h>
#include <sel4arm-vmm/exynos/devices.h>

#include <cpio/cpio.h>

#define LINUX_RAM_BASE    0x40000000
#define LINUX_RAM_SIZE    0x40000000
#define ATAGS_ADDR        (LINUX_RAM_BASE + 0x100)
#define DTB_ADDR          (LINUX_RAM_BASE + 0x0F000000)

#define MACH_TYPE_EXYNOS5410 4151
#define MACH_TYPE_SPECIAL    ~0
#define MACH_TYPE            MACH_TYPE_SPECIAL

extern char _cpio_archive[];

extern vka_t _vka;
extern vspace_t _vspace;

static const struct device *linux_pt_devices[] = {
    &dev_ps_pwm_timer,
    &dev_ps_chip_id,
    &dev_i2c1,
    &dev_i2c2,
    &dev_i2c4,
    &dev_i2chdmi,
    &dev_usb2_ohci,
    &dev_usb2_ehci,
    &dev_usb2_ctrl,
    &dev_ps_msh0,
    &dev_ps_msh2,
    &dev_uart0,
/*    &dev_uart1,*/
    //&dev_uart2, /* Console */
/*    &dev_uart3,*/
    &dev_ps_tx_mixer,
    &dev_ps_hdmi0,
    &dev_ps_hdmi1,
    &dev_ps_hdmi2,
    &dev_ps_hdmi3,
    &dev_ps_hdmi4,
    &dev_ps_hdmi5,
    &dev_ps_hdmi6,
    &dev_ps_pdma0,
    &dev_ps_pdma1,
    &dev_ps_mdma0,
    &dev_ps_mdma1,
};

struct pwr_token {
    const char* linux_bin;
    const char* device_tree;
} pwr_token;

static void* install_linux_kernel(vm_t* vm, const char* kernel_name);
static uint32_t install_linux_dtb(vm_t* vm, const char* dtb_name);

static int
vm_shutdown_cb(vm_t* vm, void* token)
{
    printf("Received shutdown from linux\n");
    return -1;
}

static int
vm_reboot_cb(vm_t* vm, void* token)
{
    struct pwr_token* pwr_token = (struct pwr_token*)token;
    uint32_t dtb_addr;
    void* entry;
    int err;
    printf("Received reboot from linux\n");

//    pwm_linux_action(1);
    return -1;
#if 0
    entry = install_linux_kernel(vm, pwr_token->linux_bin);
    dtb_addr = install_linux_dtb(vm, pwr_token->device_tree);
    if(entry == NULL || dtb_addr == 0){
        printf("Failed to reload linux\n");
        return -1;
    }
    err = vm_set_bootargs(vm, entry, MACH_TYPE, dtb_addr);
    if(err){
        printf("Failed to set boot args\n");
        return -1;
    }
    err = vm_start(vm);
    if(err){
        printf("Failed to restart linux\n");
        return -1;
    }
    printf("VM restarted\n");
    return 0;
#endif
}

static int
pwmsig_device_fault_handler(struct device* d UNUSED, vm_t* vm, fault_t* fault){
    uint32_t data = fault->data;
    ignore_fault(fault);
    printf("IN VM, GOT PWM SIGNAL 0x%x\n", data);
    fflush(stdout);
    pwm_vmsig(fault->data);
    return 0;
}

struct device pwmsig_dev = {
        .devid = DEV_CUSTOM,
        .name = "NICTAcopter signal",
        .pstart = 0x30000000,
        .size = 0x1000,
        .handle_page_fault = &pwmsig_device_fault_handler,
        .priv = NULL,
    };


static int
install_linux_devices(vm_t* vm)
{
    struct gpio_device* gpio_dev;
    struct clock_device* clock_dev;
    int err;
    int i;
    /* Install virtual devices */
    err = vm_install_vgic(vm);
    assert(!err);
    err = vm_install_ram_range(vm, LINUX_RAM_BASE, LINUX_RAM_SIZE);
    assert(!err);
    err = vm_install_vcombiner(vm);
    assert(!err);
    err = vm_install_vmct(vm);
    assert(!err);
    err = vm_install_vpower(vm, &vm_shutdown_cb, &pwr_token, &vm_reboot_cb, &pwr_token);
    assert(!err);
    err = vm_install_vsysreg(vm);
    assert(!err);

    gpio_dev = vm_install_ac_gpio(vm, VACDEV_DEFAULT_ALLOW, VACDEV_REPORT_AND_MASK);
    assert(gpio_dev);
    clock_dev = vm_install_ac_clock(vm, VACDEV_DEFAULT_ALLOW, VACDEV_REPORT_AND_MASK);
    assert(clock_dev);
    vm_clock_restrict(clock_dev, CLK_UART0);
    vm_clock_restrict(clock_dev, CLK_UART1);
    vm_clock_restrict(clock_dev, CLK_UART3);
    vm_clock_restrict(clock_dev, CLK_I2C0);
    vm_clock_restrict(clock_dev, CLK_SPI1);

    err = vm_install_passthrough_device(vm, &dev_vconsole);
    assert(!err);

    /* Device for signalling to the VM */
    err = vm_add_device(vm, &pwmsig_dev);
    assert(!err);

    /* Install pass through devices */
    for (i = 0; i < sizeof(linux_pt_devices) / sizeof(*linux_pt_devices); i++) {
        err = vm_install_passthrough_device(vm, linux_pt_devices[i]);
//        assert(!err);
    }

    return 0;
}

static uint32_t
install_linux_dtb(vm_t* vm, const char* dtb_name)
{
    void* file;
    unsigned long size;
    uint32_t dtb_addr;

    /* Retrieve the file data */
    file = cpio_get_file(_cpio_archive, dtb_name, &size);
    if (file == NULL) {
        printf("Error: Linux dtb file \'%s\' not found\n", dtb_name);
        return 0;
    }
    if (image_get_type(file) != IMG_DTB) {
        printf("Error: \'%s\' is not a device tree\n", dtb_name);
        return 0;
    }

    /* Copy the tree to the VM */
    dtb_addr = DTB_ADDR;
    if (vm_copyout(vm, file, dtb_addr, size)) {
        printf("Error: Failed to load device tree \'%s\'\n", dtb_name);
        return 0;
    } else {
        return dtb_addr;
    }
}

static void*
install_linux_kernel(vm_t* vm, const char* kernel_name)
{
    void* file;
    unsigned long size;
    uintptr_t entry;

    /* Retrieve the file data */
    file = cpio_get_file(_cpio_archive, kernel_name, &size);
    if (file == NULL) {
        printf("Error: Unable to find kernel image \'%s\'\n", kernel_name);
        return NULL;
    }

    /* Determine the load address */
    switch (image_get_type(file)) {
    case IMG_BIN:
        entry = LINUX_RAM_BASE + 0x8000;
        break;
    case IMG_ZIMAGE:
        entry = zImage_get_load_address(file, LINUX_RAM_BASE);
        break;
    default:
        printf("Error: Unknown Linux image format for \'%s\'\n", kernel_name);
        return NULL;
    }

    /* Load the image */
    if (vm_copyout(vm, file, entry, size)) {
        printf("Error: Failed to load \'%s\'\n", kernel_name);
        return NULL;
    } else {
        return (void*)entry;
    }
}

int
load_linux(vm_t* vm, const char* kernel_name, const char* dtb_name)
{
    void* entry;
    uint32_t dtb;
    int err;

    pwr_token.linux_bin = kernel_name;
    pwr_token.device_tree = dtb_name;

    /* Install devices */
    err = install_linux_devices(vm);
    if (err) {
        printf("Error: Failed to install Linux devices\n");
        return -1;
    }
    /* Load kernel */
    entry = install_linux_kernel(vm, kernel_name);
    if (!entry) {
        return -1;
    }
    /* Load device tree */
    dtb = install_linux_dtb(vm, dtb_name);
    if (!dtb) {
        return -1;
    }
    /* Set boot arguments */
    err = vm_set_bootargs(vm, entry, MACH_TYPE, dtb);
    if (err) {
        printf("Error: Failed to set boot arguments\n");
        return -1;
    }

    return 0;
}

