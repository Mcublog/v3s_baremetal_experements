#include "ephy.h"
#include "ccu.h"

void ephy_init(void)
{
    /* Тактирование и сброс EMAC + EPHY */
    CCU->bus_gating0 |= CCU_GATING_REG0_EMAC;
    CCU->bus_gating4 |= CCU_GATING_REG4_EPHY;
    CCU->bus_soft_rst0 |= CCU_SOFT_RST_REG0_EMAC;
    CCU->bus_soft_rst2 |= CCU_SOFT_RST_REG2_EPHY;

    /* Встроенный EPHY, 24 МГц, нормальный режим, PHY addr 1.
       LED_POL=1: активный низкий — так разведено на плате (LED на 3V3 через 51 Ом).
       SHUTDOWN=0: EPHY power up. */
    SYS_CFG_EMAC_EPHY_CLK = EPHY_CLK_XMII_SEL_INTERNAL
                            | EPHY_CLK_EPHY_MODE_NORMAL
                            | (EPHY_CLK_PHY_ADDR_MASK & EPHY_PHY_ADDR) << EPHY_CLK_PHY_ADDR_SHIFT
                            | EPHY_CLK_CLK_SEL_24M
                            | EPHY_CLK_LED_POL
                            | EPHY_CLK_PHY_SELECT_INTERNAL
                            | EPHY_CLK_RMII_EN;
}

static uint16_t mii_transfer(uint8_t reg, uint16_t val, int write)
{
    EMAC_MADR = ((uint32_t)EPHY_PHY_ADDR << 8) | reg;
    if (write)
        EMAC_MWTD = val;
    else
        EMAC_MCMD = EMAC_MCMD_PHY_BUSY;

    while (EMAC_MCMD & EMAC_MCMD_PHY_BUSY)
        ;

    if (!write)
        return (uint16_t)EMAC_MRDD;
    return 0;
}

uint16_t ephy_mii_read(uint8_t reg)
{
    return mii_transfer(reg, 0, 0);
}

void ephy_mii_write(uint8_t reg, uint16_t val)
{
    mii_transfer(reg, val, 1);
}

void ephy_set_power(int on)
{
    if (on)
        SYS_CFG_EMAC_EPHY_CLK &= ~EPHY_CLK_SHUTDOWN;
    else
        SYS_CFG_EMAC_EPHY_CLK |= EPHY_CLK_SHUTDOWN;
}

void ephy_set_led_polarity(int low_active)
{
    if (low_active)
        SYS_CFG_EMAC_EPHY_CLK |= EPHY_CLK_LED_POL;
    else
        SYS_CFG_EMAC_EPHY_CLK &= ~EPHY_CLK_LED_POL;
}

uint16_t ephy_bmsr(void)
{
    return ephy_mii_read(EPHY_REG_BMSR);
}