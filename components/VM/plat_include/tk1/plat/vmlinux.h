/*
 * Copyright 2017, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(DATA61_BSD)
 */
#ifndef VMLINUX_TK1_H
#define VMLINUX_TK1_H

#include <sel4arm-vmm/vm.h>

#define INTERRUPT_VTIMER                27
#define INTERRUPT_TMR1                  32
#define INTERRUPT_TMR2                  33
#define INTERRUPT_RTC                   34
#define INTERRUPT_CEC                   35
#define INTERRUPT_SHR_SEM_INBOX_FULL    36
#define INTERRUPT_SHR_SEM_INBOX_EMPTY   37
#define INTERRUPT_SHR_SEM_OUTBOX_FULL   38
#define INTERRUPT_SHR_SEM_OUTBOX_EMPTY  39
#define INTERRUPT_VDE_UCQ               40
#define INTERRUPT_VDE_SYNC_TOKEN        41
#define INTERRUPT_VDE_BSEV              42
#define INTERRUPT_VDE_BSEA              43
#define INTERRUPT_VDE_SXE               44
#define INTERRUPT_SATA_RX_STAT          45
#define INTERRUPT_SDMMC1                46
#define INTERRUPT_SDMMC2                47
#define INTERRUPT_VDE                   49
#define INTERRUPT_AVP_UCQ               50
#define INTERRUPT_SDMMC3                51
#define INTERRUPT_USB                   52
#define INTERRUPT_USB2                  53
#define INTERRUPT_SATA_CTL              55
#define INTERRUPT_VCP                   57
#define INTERRUPT_APB_DMA_CPU           58
#define INTERRUPT_AHB_DMA_CPU           59
#define INTERRUPT_ARB_SEM_GNT_COP       60
#define INTERRUPT_ARB_SEM_GNT_CPU       61
#define INTERRUPT_OWR                   62
#define INTERRUPT_SDMMC4                63
#define INTERRUPT_GPIO1                 64
#define INTERRUPT_GPIO2                 65
#define INTERRUPT_GPIO3                 66
#define INTERRUPT_GPIO4                 67
#define INTERRUPT_UARTA                 68
#define INTERRUPT_UARTB                 69
#define INTERRUPT_I2C                   70
#define INTERRUPT_USB3_HOST             71
#define INTERRUPT_USB3_HOST_SMI         72
#define INTERRUPT_TMR3                  73
#define INTERRUPT_TMR4                  74
#define INTERRUPT_USB3_HOST_PME         75
#define INTERRUPT_USB3_DEV_HOST         76
#define INTERRUPT_ACTMON                77
#define INTERRUPT_UARTC                 78
#define INTERRUPT_HSI                   79
#define INTERRUPT_THERMAL               80
#define INTERRUPT_XUSB_PADCTL           81
#define INTERRUPT_TSEC                  82
#define INTERRUPT_EDP                   83
#define INTERRUPT_VFIR                  84
#define INTERRUPT_I2C5                  85
#define INTERRUPT_STAT_MON              86
#define INTERRUPT_GPIO5                 87
#define INTERRUPT_USB3_DEV_SMI          88
#define INTERRUPT_USB3_DEV_PME          89
#define INTERRUPT_SE                    90
#define INTERRUPT_SPI1                  91
#define INTERRUPT_APB_DMA_COP           92
#define INTERRUPT_AHB_DMA_COP           93
#define INTERRUPT_CLDVFS                94
#define INTERRUPT_I2C6                  95
#define INTERRUPT_HOST1X_SYNCPT_COP     96
#define INTERRUPT_HOST1X_SYNCPT_CPU     97
#define INTERRUPT_HOST1X_GEN_COP        98
#define INTERRUPT_HOST1X_GEN_CPU        99
#define INTERRUPT_MSENC                 100
#define INTERRUPT_VI                    101
#define INTERRUPT_ISPB                  102
#define INTERRUPT_ISP                   103
#define INTERRUPT_VIC                   104
#define INTERRUPT_DISPLAY               105
#define INTERRUPT_DISPLAYB              106
#define INTERRUPT_HDMI                  107
#define INTERRUPT_SOR                   108
//#define INTERRUPT_MC                  109
#define INTERRUPT_EMC                   110
#define INTERRUPT_SPI6                  111
#define INTERRUPT_HDA                   113
#define INTERRUPT_SPI2                  114
#define INTERRUPT_SPI3                  115
#define INTERRUPT_I2C2                  116
#define INTERRUPT_PMU_EXT               118
#define INTERRUPT_GPIO6                 119
#define INTERRUPT_GPIO7                 121
#define INTERRUPT_UARTD                 122
#define INTERRUPT_I2C3                  124
#define INTERRUPT_SW                    127
#define INTERRUPT_SNOR                  128
#define INTERRUPT_USB3                  129
#define INTERRUPT_PCIE_INT              130
#define INTERRUPT_PCIE_MSI              131
#define INTERRUPT_PCIE_WAKE             132
#define INTERRUPT_AVP_CACHE             133
#define INTERRUPT_AUDIO_CLUSTER         135
#define INTERRUPT_APB_DMA_CH0           136
#define INTERRUPT_APB_DMA_CH1           137
#define INTERRUPT_APB_DMA_CH2           138
#define INTERRUPT_APB_DMA_CH3           139
#define INTERRUPT_APB_DMA_CH4           140
#define INTERRUPT_APB_DMA_CH5           141
#define INTERRUPT_APB_DMA_CH6           142
#define INTERRUPT_APB_DMA_CH7           143
#define INTERRUPT_APB_DMA_CH8           144
#define INTERRUPT_APB_DMA_CH9           145
#define INTERRUPT_APB_DMA_CH10          146
#define INTERRUPT_APB_DMA_CH11          147
#define INTERRUPT_APB_DMA_CH12          148
#define INTERRUPT_APB_DMA_CH13          149
#define INTERRUPT_APB_DMA_CH14          150
#define INTERRUPT_APB_DMA_CH15          151
#define INTERRUPT_I2C4                  152
#define INTERRUPT_TMR5                  153
#define INTERRUPT_HIER_GROUP1_COP       154
#define INTERRUPT_WDT_CPU               155
#define INTERRUPT_WDT_AVP               156
#define INTERRUPT_GPIO8                 157
#define INTERRUPT_CAR                   158
#define INTERRUPT_HIER_GROUP1_CPU       159
#define INTERRUPT_APB_DMA_CH16          160
#define INTERRUPT_APB_DMA_CH17          161
#define INTERRUPT_APB_DMA_CH18          162
#define INTERRUPT_APB_DMA_CH19          163
#define INTERRUPT_APB_DMA_CH20          164
#define INTERRUPT_APB_DMA_CH21          165
#define INTERRUPT_APB_DMA_CH22          166
#define INTERRUPT_APB_DMA_CH23          167
#define INTERRUPT_APB_DMA_CH24          168
#define INTERRUPT_APB_DMA_CH25          169
#define INTERRUPT_APB_DMA_CH26          170
#define INTERRUPT_APB_DMA_CH27          171
#define INTERRUPT_APB_DMA_CH28          172
#define INTERRUPT_APB_DMA_CH29          173
#define INTERRUPT_APB_DMA_CH30          174
#define INTERRUPT_APB_DMA_CH31          175
#define INTERRUPT_CPU0_PMU              176
#define INTERRUPT_CPU1_PMU              177
#define INTERRUPT_CPU2_PMU              178
#define INTERRUPT_CPU3_PMU              179
#define INTERRUPT_SDMMC1_SYS            180
#define INTERRUPT_SDMMC2_SYS            181
#define INTERRUPT_SDMMC3_SYS            182
#define INTERRUPT_SDMMC4_SYS            183
#define INTERRUPT_TMR6                  184
#define INTERRUPT_TMR7                  185
#define INTERRUPT_TMR8                  186
#define INTERRUPT_TMR9                  187
#define INTERRUPT_TMR0                  188
#define INTERRUPT_GPU                   189
#define INTERRUPT_GPU_NONSTALL          190
#define ARDPAUX                         191

static const int linux_pt_irqs[] = {
INTERRUPT_VTIMER               ,
INTERRUPT_USB2                 ,
INTERRUPT_SDMMC4               ,
INTERRUPT_UARTD                ,
INTERRUPT_USB3,
#ifdef CONFIG_TK1_INSECURE
INTERRUPT_TMR1                 ,
INTERRUPT_TMR2                 ,
INTERRUPT_RTC                  ,
INTERRUPT_CEC                  ,
INTERRUPT_SHR_SEM_INBOX_FULL   ,
INTERRUPT_SHR_SEM_INBOX_EMPTY  ,
INTERRUPT_SHR_SEM_OUTBOX_FULL  ,
INTERRUPT_SHR_SEM_OUTBOX_EMPTY ,
INTERRUPT_VDE_UCQ              ,
INTERRUPT_VDE_SYNC_TOKEN       ,
INTERRUPT_VDE_BSEV             ,
INTERRUPT_VDE_BSEA             ,
INTERRUPT_VDE_SXE              ,
INTERRUPT_SATA_RX_STAT         ,
INTERRUPT_SDMMC1               ,
INTERRUPT_SDMMC2               ,
INTERRUPT_VDE                  ,
INTERRUPT_AVP_UCQ              ,
INTERRUPT_SDMMC3               ,
INTERRUPT_USB                  ,
INTERRUPT_SATA_CTL             ,
INTERRUPT_VCP                  ,
INTERRUPT_APB_DMA_CPU          ,
INTERRUPT_AHB_DMA_CPU          ,
INTERRUPT_ARB_SEM_GNT_COP      ,
INTERRUPT_ARB_SEM_GNT_CPU      ,
INTERRUPT_OWR                  ,
INTERRUPT_GPIO1                ,
INTERRUPT_GPIO2                ,
INTERRUPT_GPIO3                ,
INTERRUPT_GPIO4                ,
INTERRUPT_UARTA                ,
INTERRUPT_UARTB                ,
INTERRUPT_I2C                  ,
INTERRUPT_USB3_HOST            ,
INTERRUPT_USB3_HOST_SMI        ,
INTERRUPT_TMR3                 ,
INTERRUPT_TMR4                 ,
INTERRUPT_USB3_HOST_PME        ,
INTERRUPT_USB3_DEV_HOST        ,
INTERRUPT_ACTMON               ,
INTERRUPT_UARTC                ,
INTERRUPT_HSI                  ,
INTERRUPT_THERMAL              ,
INTERRUPT_XUSB_PADCTL          ,
INTERRUPT_TSEC                 ,
INTERRUPT_EDP                  ,
INTERRUPT_VFIR                 ,
INTERRUPT_I2C5                 ,
INTERRUPT_STAT_MON             ,
INTERRUPT_GPIO5                ,
INTERRUPT_USB3_DEV_SMI         ,
INTERRUPT_USB3_DEV_PME         ,
INTERRUPT_SE                   ,
INTERRUPT_SPI1                 ,
INTERRUPT_APB_DMA_COP          ,
INTERRUPT_AHB_DMA_COP          ,
INTERRUPT_CLDVFS               ,
INTERRUPT_I2C6                 ,
INTERRUPT_HOST1X_SYNCPT_COP    ,
INTERRUPT_HOST1X_SYNCPT_CPU    ,
INTERRUPT_HOST1X_GEN_COP       ,
INTERRUPT_HOST1X_GEN_CPU       ,
INTERRUPT_MSENC                ,
INTERRUPT_VI                   ,
INTERRUPT_ISPB                 ,
INTERRUPT_ISP                  ,
INTERRUPT_VIC                  ,
INTERRUPT_DISPLAY              ,
INTERRUPT_DISPLAYB             ,
INTERRUPT_HDMI                 ,
INTERRUPT_SOR                  ,
INTERRUPT_EMC                  ,
INTERRUPT_SPI6                 ,
INTERRUPT_HDA                  ,
INTERRUPT_SPI2                 ,
INTERRUPT_SPI3                 ,
INTERRUPT_I2C2                 ,
INTERRUPT_PMU_EXT              ,
INTERRUPT_GPIO6                ,
INTERRUPT_GPIO7                ,
INTERRUPT_I2C3                 ,
INTERRUPT_SW                   ,
INTERRUPT_SNOR                 ,
INTERRUPT_PCIE_INT             ,
INTERRUPT_PCIE_MSI             ,
INTERRUPT_PCIE_WAKE            ,
INTERRUPT_AVP_CACHE            ,
INTERRUPT_AUDIO_CLUSTER        ,
INTERRUPT_APB_DMA_CH0          ,
INTERRUPT_APB_DMA_CH1          ,
INTERRUPT_APB_DMA_CH2          ,
INTERRUPT_APB_DMA_CH3          ,
INTERRUPT_APB_DMA_CH4          ,
INTERRUPT_APB_DMA_CH5          ,
INTERRUPT_APB_DMA_CH6          ,
INTERRUPT_APB_DMA_CH7          ,
INTERRUPT_APB_DMA_CH8          ,
INTERRUPT_APB_DMA_CH9          ,
INTERRUPT_APB_DMA_CH10         ,
INTERRUPT_APB_DMA_CH11         ,
INTERRUPT_APB_DMA_CH12         ,
INTERRUPT_APB_DMA_CH13         ,
INTERRUPT_APB_DMA_CH14         ,
INTERRUPT_APB_DMA_CH15         ,
INTERRUPT_I2C4                 ,
INTERRUPT_TMR5                 ,
INTERRUPT_HIER_GROUP1_COP      ,
INTERRUPT_WDT_CPU              ,
INTERRUPT_WDT_AVP              ,
INTERRUPT_GPIO8                ,
INTERRUPT_CAR                  ,
INTERRUPT_HIER_GROUP1_CPU      ,
INTERRUPT_APB_DMA_CH16         ,
INTERRUPT_APB_DMA_CH17         ,
INTERRUPT_APB_DMA_CH18         ,
INTERRUPT_APB_DMA_CH19         ,
INTERRUPT_APB_DMA_CH20         ,
INTERRUPT_APB_DMA_CH21         ,
INTERRUPT_APB_DMA_CH22         ,
INTERRUPT_APB_DMA_CH23         ,
INTERRUPT_APB_DMA_CH24         ,
INTERRUPT_APB_DMA_CH25         ,
INTERRUPT_APB_DMA_CH26         ,
INTERRUPT_APB_DMA_CH27         ,
INTERRUPT_APB_DMA_CH28         ,
INTERRUPT_APB_DMA_CH29         ,
INTERRUPT_APB_DMA_CH30         ,
INTERRUPT_APB_DMA_CH31         ,
INTERRUPT_CPU0_PMU             ,
INTERRUPT_CPU1_PMU             ,
INTERRUPT_CPU2_PMU             ,
INTERRUPT_CPU3_PMU             ,
INTERRUPT_SDMMC1_SYS           ,
INTERRUPT_SDMMC2_SYS           ,
INTERRUPT_SDMMC3_SYS           ,
INTERRUPT_SDMMC4_SYS           ,
INTERRUPT_TMR6                 ,
INTERRUPT_TMR7                 ,
INTERRUPT_TMR8                 ,
INTERRUPT_TMR9                 ,
INTERRUPT_TMR0                 ,
INTERRUPT_GPU                  ,
INTERRUPT_GPU_NONSTALL         ,
ARDPAUX                        ,
#endif
};


#define LINUX_RAM_PADDR_BASE 0xb0000000
#define LINUX_RAM_BASE    0x80000000
#define LINUX_RAM_SIZE    0x40000000
#define PLAT_RAM_END      0xffffffff
/* the offset between actual physcial memory and guest physical memory */
#define LINUX_RAM_OFFSET  (LINUX_RAM_PADDR_BASE - LINUX_RAM_BASE)
#define DTB_ADDR          (LINUX_RAM_BASE + 0x02000000)
#define INITRD_MAX_SIZE   0x1900000 //25 MB
#define INITRD_ADDR       (DTB_ADDR - INITRD_MAX_SIZE) //0x80700000

#endif /* VMLINUX_H */

