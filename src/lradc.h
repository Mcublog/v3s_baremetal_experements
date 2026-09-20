#ifndef V3S_LRADC_H
#define V3S_LRADC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /* LRADC (KEYADC0), глава 4.11 даташита */
    #define LRADC_BASE 0x01C22800u

    #define LRADC_CTRL (*(volatile uint32_t *)(LRADC_BASE + 0x00))
    #define LRADC_INTC (*(volatile uint32_t *)(LRADC_BASE + 0x04))
    #define LRADC_INTS (*(volatile uint32_t *)(LRADC_BASE + 0x08))
    #define LRADC_DATA0 (*(volatile uint32_t *)(LRADC_BASE + 0x0C))

    #define LRADC_CTRL_EN (1u << 0)             /* LRADC enable */
    #define LRADC_CTRL_HOLD_EN (1u << 6)        /* LRADC sample hold enable (default 1) */
    #define LRADC_DATA0_MASK 0x3Fu              /* 6-bit результат */

    void lradc_init(void);
    uint16_t lradc_read_raw(void);
    uint16_t lradc_read_mv(void);

#ifdef __cplusplus
}
#endif

#endif