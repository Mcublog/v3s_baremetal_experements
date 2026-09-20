#include "delay.h"

#define TMR0_CTL    0x01C20C10
#define TMR0_INT    0x01C20C14
#define TMR0_CNT    0x01C20C18

#define TMR_CTL_ENABLE     (1U << 0)
#define TMR_CTL_RELOAD     (1U << 1)
#define TMR_CTL_CLK_OSC24M (1U << 2)

#define OSC24M_HZ    24000000U

static inline uint32_t reg32_read(uint32_t addr)
{
    return *(volatile uint32_t *)(uintptr_t)addr;
}

static inline void reg32_write(uint32_t addr, uint32_t val)
{
    *(volatile uint32_t *)(uintptr_t)addr = val;
}

void delay_init(void)
{
    reg32_write(TMR0_INT, 0xFFFFFFFF);
    reg32_write(TMR0_CTL, TMR_CTL_ENABLE | TMR_CTL_RELOAD | TMR_CTL_CLK_OSC24M);
}

void delay_ms(uint32_t ms)
{
    uint32_t start, ticks, target;

    if (ms == 0)
        return;

    start = ~reg32_read(TMR0_CNT);
    ticks = ms * (OSC24M_HZ / 1000U);
    target = start + ticks;

    while ((int32_t)(target - (~reg32_read(TMR0_CNT))) > 0)
        ;
}