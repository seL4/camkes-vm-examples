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
#include <sel4arm-vmm/devices/vgic.h>
#include <sel4arm-vmm/devices/vram.h>
#include <sel4arm-vmm/devices/vusb.h>
#include <sel4utils/irq_server.h>

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
extern irq_server_t _irq_server;
extern seL4_CPtr _fault_endpoint;

static const struct device *linux_pt_devices[] = {
    &dev_ps_chip_id,
    &dev_msh0,
    &dev_msh2,
#ifndef CONFIG_APP_LINUX_SECURE
    &dev_alive,
    &dev_sysreg,
    &dev_gpio_left,
    &dev_gpio_right,
    &dev_ps_pwm_timer,
    &dev_i2c4,
    &dev_usb2_ehci,
    &dev_usb2_ctrl,
    &dev_i2c1,
    &dev_i2c2,
    &dev_i2chdmi,
    &dev_usb2_ohci,
    &dev_uart0,
    &dev_uart1,
    //&dev_uart2, /* Console */
    &dev_uart3,
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
#endif
};

static const int linux_pt_irqs[] = {
    27, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
    50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67,
    77, 78, 79, 82, 85, 88, 89, 90, 92, 96, 97, 104, 107, 109, 126, 152,
    215, 216, 217, 232,
#if !defined(CONFIG_APP_SEL4ARMVMM_HAVE_VUSB)
    103
#endif
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

    pwm_vmsig(0);
    vm_sem_wait();

    return 0;

//    pwm_linux_action(1);
    return -1;
#if 0
    entry = install_linux_kernel(vm, pwr_token->linux_bin);
    dtb_addr = install_linux_dtb(vm, pwr_token->device_tree);
    if (entry == NULL || dtb_addr == 0) {
        printf("Failed to reload linux\n");
        return -1;
    }
    err = vm_set_bootargs(vm, entry, MACH_TYPE, dtb_addr);
    if (err) {
        printf("Failed to set boot args\n");
        return -1;
    }
    err = vm_start(vm);
    if (err) {
        printf("Failed to restart linux\n");
        return -1;
    }
    printf("VM restarted\n");
    return 0;
#endif
}

static int
pwmsig_device_fault_handler(struct device* d UNUSED, vm_t* vm, fault_t* fault){
    uint32_t data = fault_get_data(fault);
    ignore_fault(fault);
//    printf("IN VM, GOT PWM SIGNAL 0x%x\n", data);
//    fflush(stdout);
    pwm_vmsig(data);
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


#if defined CONFIG_APP_SEL4ARMVMM_HAVE_VUSB

static vusb_device_t* _vusb;
static usb_host_t _hcd;

static void
usb_irq_handler(struct irq_data* irq_data)
{
    usb_host_t* hcd = (usb_host_t*)irq_data->token;
    usb_hcd_handle_irq(hcd);
    irq_data_ack_irq(irq_data);
}

static int
install_vusb(vm_t* vm)
{
    irq_server_t irq_server;
    ps_io_ops_t* io_ops;
    vusb_device_t* vusb;
    usb_host_t* hcd;
    struct irq_data* irq_data;
    seL4_CPtr vmm_ep;
    int err;
    irq_server = _irq_server;
    io_ops = vm->io_ops;
    hcd = &_hcd;
    vmm_ep = _fault_endpoint;

    /* Initialise the physical host controller */
    err = usb_host_init(USB_HOST_DEFAULT, io_ops, hcd);
    assert(!err);
    if (err) {
        return -1;
    }

    /* Route physical IRQs */
    irq_data = irq_server_register_irq(irq_server, 103, usb_irq_handler, hcd);
    if (!irq_data) {
        return -1;
    }
    /* Install the virtual device */
    vusb = vm_install_vusb(vm, hcd, VUSB_ADDRESS, VUSB_IRQ, vmm_ep, VUSB_NINDEX,
                           VUSB_NBADGE);
    assert(vusb != NULL);
    if (vusb == NULL) {
        return -1;
    }
    _vusb = vusb;

    return 0;
}

void
vusb_notify(void)
{
    vm_vusb_notify(_vusb);
}

#else /* CONFIG_APP_SEL4ARMVMM_HAVE_VUSB */

static int
install_vusb(vm_t* vm)
{
    /* Linux phy initialisation seems to be buggy when booting over TFTPBOOT
     * Initialise the phy for Linux here */
    usb_host_t hcd;
    usb_host_init(USB_HOST_DEFAULT, vm->io_ops, &hcd);
    return 0;
}

void
vusb_notify(void)
{
}

#endif /* CONFIG_APP_SEL4ARMVMM_HAVE_VUSB */

void
configure_gpio(vm_t *vm)
{
#ifdef CONFIG_APP_LINUX_SECURE
    /* Don't provide any access to GPIO/MUX */
#else /* CONFIG_APP_LINUX_SECURE */
    /* Provide GPIOS */
    struct gpio_device* gpio_dev;
    gpio_dev = vm_install_ac_gpio(vm, VACDEV_DEFAULT_ALLOW, VACDEV_REPORT_AND_MASK);
    assert(gpio_dev);
#if 0
    vm_gpio_restrict(gpio_dev, GPIOID(GPA1, 0));
    vm_gpio_restrict(gpio_dev, GPIOID(GPA1, 1));
    vm_gpio_restrict(gpio_dev, GPIOID(GPA1, 2));
    vm_gpio_restrict(gpio_dev, GPIOID(GPA1, 3));
    vm_gpio_restrict(gpio_dev, GPIOID(GPA1, 4));
    vm_gpio_restrict(gpio_dev, GPIOID(GPA1, 5));
    vm_gpio_restrict(gpio_dev, GPIOID(GPA1, 6));
    vm_gpio_restrict(gpio_dev, GPIOID(GPA1, 7));
#endif
#endif /* CONFIG_APP_LINUX_SECURE */
}
void
configure_clocks(vm_t *vm)
{
    struct clock_device* clock_dev;
#ifdef CONFIG_APP_LINUX_SECURE
    clock_dev = vm_install_ac_clock(vm, VACDEV_DEFAULT_DENY, VACDEV_REPORT_AND_MASK);
    assert(clock_dev);
    vm_clock_provide(clock_dev, CLK_MMC0);
    vm_clock_provide(clock_dev, CLK_MMC2);
    vm_clock_provide(clock_dev, CLK_SCLKVPLL);
    vm_clock_provide(clock_dev, CLK_SCLKGPLL);
    vm_clock_provide(clock_dev, CLK_SCLKCPLL);
#else /* CONFIG_APP_LINUX_SECURE */
    clock_dev = vm_install_ac_clock(vm, VACDEV_DEFAULT_ALLOW, VACDEV_REPORT_AND_MASK);
    assert(clock_dev);
    vm_clock_restrict(clock_dev, CLK_UART0);
    vm_clock_restrict(clock_dev, CLK_UART1);
    vm_clock_restrict(clock_dev, CLK_UART2);
    vm_clock_restrict(clock_dev, CLK_UART3);
    vm_clock_restrict(clock_dev, CLK_I2C0);
    vm_clock_restrict(clock_dev, CLK_SPI1);
#endif /* CONFIG_APP_LINUX_SECURE */
}

static int
install_linux_devices(vm_t* vm)
{
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

#if CONFIG_APP_LINUX_SECURE
    /* Add hooks for specific power management hooks */
    err = vm_install_vpower(vm, &vm_shutdown_cb, &pwr_token, &vm_reboot_cb, &pwr_token);
    assert(!err);
    /* Install virtual USB */
    err = install_vusb(vm);
    assert(!err);
#if 0
    /* Install SDHC controller with DMA restricted */
    err = vm_install_nodma_sdhc2(vm);
    assert(!err);
#endif
#endif /* CONFIG_APP_LINUX_SECURE */
    configure_gpio(vm);
    configure_clocks(vm);

    err = vm_install_ac_uart(vm, &dev_vconsole);
    assert(!err);

    /* Device for signalling to the VM */
    err = vm_add_device(vm, &pwmsig_dev);
    assert(!err);

    /* Install pass through devices */
    for (i = 0; i < sizeof(linux_pt_devices) / sizeof(*linux_pt_devices); i++) {
        err = vm_install_passthrough_device(vm, linux_pt_devices[i]);
    }

    return 0;
}

static void
do_irq_server_ack(void* token)
{
    struct irq_data* irq_data = (struct irq_data*)token;
    irq_data_ack_irq(irq_data);
}

static void
irq_handler(struct irq_data* irq_data)
{
    virq_handle_t virq;
    int err;
    virq = (virq_handle_t)irq_data->token;
    err = vm_inject_IRQ(virq);
    assert(!err);
}

static void
vcombiner_irq_handler(struct irq_data* irq)
{
    vm_t* vm;
    assert(irq);
    vm = (vm_t*)irq->token;
    vm_combiner_irq_handler(vm, irq->irq);
    irq_data_ack_irq(irq);
}

static int
route_irqs(vm_t* vm, irq_server_t irq_server)
{
    int i;
    for (i = 0; i < ARRAY_SIZE(linux_pt_irqs); i++) {
        irq_t irq = linux_pt_irqs[i];
        struct irq_data* irq_data;
        virq_handle_t virq;
        void (*handler)(struct irq_data*);
        if (irq >= 32 && irq <= 63) {
            /* IRQ combiner IRQs must be handled by the combiner directly */
            handler = &vcombiner_irq_handler;
        } else {
            handler = &irq_handler;
        }
        irq_data = irq_server_register_irq(irq_server, irq, handler, NULL);
        if (!irq_data) {
            continue;
        }
        virq = vm_virq_new(vm, irq, &do_irq_server_ack, irq_data);
        if (virq == NULL) {
            continue;
        }
        irq_data->token = (void*)virq;
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
    /* Route IRQs */
    err = route_irqs(vm, _irq_server);
    if (err) {
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

