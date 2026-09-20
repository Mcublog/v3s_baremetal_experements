#ifndef V3S_EPHY_H
#define V3S_EPHY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /* System Control (SYS_CFG): EMAC-EPHY Clock Register, offset 0x30 */
    #define SYS_CFG_BASE 0x01C00000u
    #define SYS_CFG_EMAC_EPHY_CLK (*(volatile uint32_t *)(SYS_CFG_BASE + 0x30))

    #define EPHY_CLK_XMII_SEL_INTERNAL (0u << 27)   /* Internal SMI and MII */
    #define EPHY_CLK_EPHY_MODE_NORMAL (0u << 25)
    #define EPHY_CLK_PHY_ADDR_MASK 0x1Fu
    #define EPHY_CLK_PHY_ADDR_SHIFT 20u
    #define EPHY_CLK_CLK_SEL_24M (1u << 18)
    #define EPHY_CLK_LED_POL (1u << 17)             /* 0: high active, 1: low active */
    #define EPHY_CLK_SHUTDOWN (1u << 16)            /* 0: power up, 1: shutdown */
    #define EPHY_CLK_PHY_SELECT_INTERNAL (1u << 15)
    #define EPHY_CLK_RMII_EN (1u << 13)

    /* EMAC MDIO (MII) */
    #define EMAC_BASE 0x01C30000u
    #define EMAC_MCMD (*(volatile uint32_t *)(EMAC_BASE + 0x10))
    #define EMAC_MADR (*(volatile uint32_t *)(EMAC_BASE + 0x14))
    #define EMAC_MWTD (*(volatile uint32_t *)(EMAC_BASE + 0x18))
    #define EMAC_MRDD (*(volatile uint32_t *)(EMAC_BASE + 0x1C))

    #define EMAC_MCMD_PHY_BUSY (1u << 0)

    /* Внутренний EPHY: адрес PHY и стандартные MII-регистры */
    #define EPHY_PHY_ADDR 1u
    #define EPHY_REG_BMSR 1u
    #define EPHY_REG_PHYID1 2u
    #define EPHY_REG_PHYID2 3u
    #define EPHY_BMSR_LINK_STATUS (1u << 2)
    #define EPHY_BMSR_100BASE_TX_FD (1u << 14)

    void ephy_init(void);
    uint16_t ephy_mii_read(uint8_t reg);
    void ephy_mii_write(uint8_t reg, uint16_t val);
    void ephy_set_power(int on);
    void ephy_set_led_polarity(int low_active);
    uint16_t ephy_bmsr(void);

#ifdef __cplusplus
}
#endif

#endif