#include "lradc.h"

void lradc_init(void)
{
    /* Дефолтное значение 0x01000168 уже задаёт: нормальный режим (KEY_MODE=00),
       HOLD_EN=1, сэмплирование 250 Гц (SAMPLE_RATE=00).
       Остаётся только включить сам преобразователь. */
    LRADC_CTRL |= LRADC_CTRL_EN;
}

uint16_t lradc_read_raw(void)
{
    return (uint16_t)(LRADC_DATA0 & LRADC_DATA0_MASK);
}

uint16_t lradc_read_mv(void)
{
    /* 6-бит шкала 0~3.0 В: напряжение = raw * 3000 / 64 */
    return (uint16_t)(((uint32_t)lradc_read_raw() * 3000u) / 64u);
}