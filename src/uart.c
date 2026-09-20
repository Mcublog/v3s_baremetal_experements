#include "uart.h"
#include "ccu.h"
#include "pio.h"

#include <stdint.h>

#define UART0_BASE 0x01C28000u

typedef struct {
    volatile uint32_t rbr_thr_dll;  /* 0x00 RBR/THR/DLL */
    volatile uint32_t ier_dlh;      /* 0x04 IER/DLH */
    volatile uint32_t iir_fcr;      /* 0x08 IIR/FCR */
    volatile uint32_t lcr;          /* 0x0C */
    volatile uint32_t mcr;          /* 0x10 */
    volatile uint32_t lsr;          /* 0x14 */
    volatile uint32_t msr;          /* 0x18 */
    volatile uint32_t sched;        /* 0x1C */
} uart_regs_t;

#define UART0 ((volatile uart_regs_t *)UART0_BASE)

#define LCR_DLAB 0x80
#define LCR_8N1  0x03
#define LSR_THRE (1u << 5)

/* Делитель для 115200 при тактировании 24 МГц: 24e6 / (16 * 115200) ~ 13.02 */
#define UART_DIVISOR 13

void uart_init(void)
{
    CCU->bus_gating2 |= CCU_GATING_APB1_PIO;
    CCU->bus_gating3 |= CCU_GATING_APB2_UART0;
    CCU->bus_soft_rst4 |= CCU_SOFT_RST_APB2_UART0;

    PIO_PB_CFG2 = (PIO_PB_CFG2 & ~PIO_PB89_MASK) | PIO_PB89_UART0;

    UART0->lcr = LCR_DLAB;
    UART0->rbr_thr_dll = UART_DIVISOR;
    UART0->ier_dlh = 0;
    UART0->lcr = LCR_8N1;
    UART0->iir_fcr = 0x07;
}

void uart_putchar(char c)
{
    while (!(UART0->lsr & LSR_THRE))
        ;
    UART0->rbr_thr_dll = (uint32_t)(uint8_t)c;
}

void uart_puts(const char *s)
{
    while (*s)
        uart_putchar(*s++);
}