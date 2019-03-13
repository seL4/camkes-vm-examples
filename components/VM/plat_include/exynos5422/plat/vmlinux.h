/*
 * Copyright 2019, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(DATA61_BSD)
 */
#pragma once

#include <sel4arm-vmm/vm.h>

#define LINUX_RAM_BASE    0x40000000
#define LINUX_RAM_PADDR_BASE LINUX_RAM_BASE
#define LINUX_RAM_SIZE    0x20000000
#define LINUX_RAM_OFFSET  0
#define DTB_ADDR          (LINUX_RAM_BASE + 0x0F000000)
#define INITRD_MAX_SIZE   0x1900000 //25 MB
#define INITRD_ADDR       (DTB_ADDR - INITRD_MAX_SIZE) //0x4D700000

static const int linux_pt_irqs[] = {
    27, // VTCNT
    32, // INTERRUPT CONTROLLER
    33, // INTERRUPT CONTROLLER
    34, // INTERRUPT CONTROLLER
    35, // INTERRUPT CONTROLLER
    36, // INTERRUPT CONTROLLER
    37, // INTERRUPT CONTROLLER
    38, // INTERRUPT CONTROLLER
    39, // INTERRUPT CONTROLLER
    40, // INTERRUPT CONTROLLER
    41, // INTERRUPT CONTROLLER
    42, // INTERRUPT CONTROLLER
    43, // INTERRUPT CONTROLLER
    44, // INTERRUPT CONTROLLER
    45, // INTERRUPT CONTROLLER
    46, // INTERRUPT CONTROLLER
    47, // INTERRUPT CONTROLLER
    48, // INTERRUPT CONTROLLER
    49, // INTERRUPT CONTROLLER
    50, // INTERRUPT CONTROLLER
    51, // INTERRUPT CONTROLLER
    52, // INTERRUPT CONTROLLER
    53, // INTERRUPT CONTROLLER
    54, // INTERRUPT CONTROLLER
    55, // INTERRUPT CONTROLLER
    56, // INTERRUPT CONTROLLER
    57, // INTERRUPT CONTROLLER
    58, // INTERRUPT CONTROLLER
    59, // INTERRUPT CONTROLLER
    60, // INTERRUPT CONTROLLER
    61, // INTERRUPT CONTROLLER
    62, // INTERRUPT CONTROLLER
    63, // INTERRUPT CONTROLLER
    64, // PINCTRL WAKEUP
    65, // MDMA
    66, // PDMA
    67, // PDMA
    68, // PWM
    69, // PWM
    70, // PWM
    71, // PWM
    72, // PWM
    74, // WATCHDOG
    75, // RTC
    76, // RTC
    77, // GPIO RIGHT
    78, // GPIO LEFT
    79, // PINCTRL GPZ
    82, // PINCTRL3
    83, // UART0
    84, // UART1
    85, // UART2
    86, // UART3
    88, // I2C0
    89, // I2C1
    90, // I2C2
    91, // I2C3
    92, // I2C4
    93, // I2C5
    94, // I2C6
    95, // I2C7
    103, // USB
    106, // G3D MMU
    107, // SDMMC0
    109, // SDMMC2
    110, // GPIO TOP
    116, // ROTATOR
    117, // VIDEO SCALER
    118, // VIDEO SCALER
    138, // ADC
    142, // ADMA
    144, // SSS
    146, // CEC
    192, // KFC CPU0
    193, // KFC CPU1
    194, // KFC CPU2
    195, // KFC CPU3
    201, // SYSMMU JPEG2x
    218, // SYSMMU R MSCL1
    220, // SYSMMU R MSCL2
};
