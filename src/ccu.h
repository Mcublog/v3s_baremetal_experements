#ifndef V3S_CCU_H
#define V3S_CCU_H

#include <stdint.h>

#define CCU_BASE 0x01C20000u

typedef struct {
    volatile uint32_t pll_cpu;              /* 0x00  */
    uint32_t _reserved[7];                  /* 0x04..0x1F */
    volatile uint32_t pll_audio;            /* 0x20  */
    volatile uint32_t pll_video;            /* 0x24  */
    volatile uint32_t pll_ve;               /* 0x28  */
    volatile uint32_t pll_ddr;              /* 0x2C  */
    volatile uint32_t pll_periph;           /* 0x30  */
    uint32_t _reserved2;                    /* 0x34  */
    volatile uint32_t pll_csi;              /* 0x38  */
    volatile uint32_t pll_isp;              /* 0x3C  */
    volatile uint32_t pll_audio_div;        /* 0x40  */
    volatile uint32_t pll_video_div;        /* 0x44  */
    uint32_t _reserved3[6];                 /* 0x48..0x5F */
    volatile uint32_t bus_gating0;          /* 0x60  */
    volatile uint32_t bus_gating1;          /* 0x64  */
    volatile uint32_t bus_gating2;          /* 0x68  APB1: codec/PIO/I2S0 */
    volatile uint32_t bus_gating3;          /* 0x6C  APB2: I2C0/I2C1/UART0/1/2 */
    volatile uint32_t bus_gating4;          /* 0x70  EPHY: bit0 */
    uint32_t _reserved4[147];               /* 0x74..0x2BF */
    volatile uint32_t bus_soft_rst0;        /* 0x2C0 */
    volatile uint32_t bus_soft_rst1;        /* 0x2C4 */
    volatile uint32_t bus_soft_rst2;        /* 0x2C8 */
    uint32_t _reserved5;                    /* 0x2CC */
    volatile uint32_t bus_soft_rst3;        /* 0x2D0  codec */
    uint32_t _reserved6;                    /* 0x2D4 */
    volatile uint32_t bus_soft_rst4;        /* 0x2D8  I2C/UART resets */
} ccu_regs_t;

#define CCU ((volatile ccu_regs_t *)CCU_BASE)

#define CCU_GATING_APB1_PIO (1u << 5)
#define CCU_GATING_APB2_UART0 (1u << 16)
#define CCU_GATING_REG0_EMAC (1u << 17)
#define CCU_GATING_REG4_EPHY (1u << 0)
#define CCU_SOFT_RST_APB2_UART0 (1u << 16)
#define CCU_SOFT_RST_REG0_EMAC (1u << 17)
#define CCU_SOFT_RST_REG2_EPHY (1u << 2)

#endif