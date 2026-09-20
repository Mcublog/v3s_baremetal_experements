#include "delay.h"
#include "uart.h"
#include "ephy.h"
#include "lradc.h"

#include <stdint.h>

static void put_hex4(unsigned v)
{
    v &= 0xFu;
    uart_putchar(v < 10 ? (char)('0' + v) : (char)('A' + v - 10));
}

static void put_hex16(uint16_t v)
{
    put_hex4(v >> 12);
    put_hex4(v >> 8);
    put_hex4(v >> 4);
    put_hex4(v);
}

static void put_dec16(uint16_t v)
{
    char buf[5];
    uint8_t i = 0;

    if (v == 0)
    {
        uart_putchar('0');
        return;
    }
    while (v > 0 && i < sizeof(buf))
    {
        buf[i++] = (char)('0' + v % 10);
        v /= 10;
    }
    while (i > 0)
        uart_putchar(buf[--i]);
}

static void dump_keyadc0(void)
{
    uint16_t raw = lradc_read_raw();

    uart_puts("  KEYADC0: 0x");
    put_hex4(raw);
    uart_puts(" (");
    put_dec16(lradc_read_mv());
    uart_puts(" mV)\r\n");
}

static void dump_mii_regs(void)
{
    uint16_t phyid1 = ephy_mii_read(EPHY_REG_PHYID1);
    uint16_t phyid2 = ephy_mii_read(EPHY_REG_PHYID2);
    uint16_t bmsr   = ephy_bmsr();

    uart_puts("  PHYID  : ");
    put_hex16(phyid1);
    uart_putchar('.');
    put_hex16(phyid2);
    uart_puts("\r\n");

    uart_puts("  BMSR   : ");
    put_hex16(bmsr);
    uart_puts("\r\n");

    uart_puts("  Link   : ");
    uart_puts((bmsr & EPHY_BMSR_LINK_STATUS) ? "UP" : "DOWN");
    uart_puts(bmsr & EPHY_BMSR_100BASE_TX_FD ? " (100FD)\r\n" : "\r\n");

    uart_puts("  Vendor (0x10..0x1F): ");
    {
        uint8_t i;
        for (i = 0x10; i <= 0x1F; i++)
        {
            put_hex16(ephy_mii_read(i));
            uart_putchar(' ');
        }
    }
    uart_puts("\r\n");
}

void main(void)
{
    uart_init();
    delay_init();

    uart_puts("Hello from V3s!\r\n");
    uart_puts("EPHY LED demo: ETH_LED_LINK / ETH_LED_SPD.\r\n");

    ephy_init();
    uart_puts("EPHY powered up (internal PHY, addr 0x01), LED_POL=low-active.\r\n");

    lradc_init();
    uart_puts("LRADC (KEYADC0) enabled, 250 Hz sample, no button ~3.0V.\r\n");

    while (1)
    {
        uart_puts("[0] KEYADC0 value (buttons: SELECT/START/VOL+/VOL- lower the level)\r\n");
        dump_keyadc0();

        uart_puts("[1] PHY/MII status\r\n");
        dump_mii_regs();

        uart_puts("[2] LED_POL low-active  -> LEDs on low level\r\n");
        ephy_set_led_polarity(1);
        delay_ms(500);

        uart_puts("[3] LED_POL high-active -> LEDs inverted (blink on idle)\r\n");
        ephy_set_led_polarity(0);
        delay_ms(500);
        ephy_set_led_polarity(1);

        uart_puts("[4] EPHY shutdown (LEDs off)\r\n");
        ephy_set_power(0);
        delay_ms(500);

        uart_puts("[5] EPHY power up\r\n");
        ephy_set_power(1);
        delay_ms(500);
    }
}