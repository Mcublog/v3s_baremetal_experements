#ifndef V3S_PIO_H
#define V3S_PIO_H

#include <stdint.h>

#define PIO_BASE 0x01C20800u

/* Базы банков: PA 0x00, PB 0x20, PC 0x40, PD 0x60, PE 0x80, PF 0xA0, PG 0xC0 */
#define PIO_PB_BASE (PIO_BASE + 0x20)

/* PB_CFG2 (0x28 относительно PB): PB8..PB11, по 4 бита на пин */
#define PIO_PB_CFG2 (*(volatile uint32_t *)(PIO_BASE + 0x28))

#define PIO_PB8_SHIFT 0
#define PIO_PB9_SHIFT 4
#define PIO_PIN_MASK 0x0Fu

/* На V3s функция "uart0": PB8=TX, PB9=RX (mux 3) */
#define PIO_FN_UART0 3

#define PIO_PB89_MASK \
    ((PIO_PIN_MASK << PIO_PB8_SHIFT) | (PIO_PIN_MASK << PIO_PB9_SHIFT))
#define PIO_PB89_UART0 \
    ((PIO_FN_UART0 << PIO_PB8_SHIFT) | (PIO_FN_UART0 << PIO_PB9_SHIFT))

#endif