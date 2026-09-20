#!/usr/bin/env bash
# Пишет строку в UART0 платы напрямую через FEL (без загрузки кода в SRAM).
# Проверено на Allwinner V3s (CherryPi PC V3S), UART0: PB8(TX)/PB9(RX), 115200 8N1.
#
# Usage: fel_uart_hello.sh ["текст"]   (по умолчанию: "hello from fel")

set -euo pipefail

MSG="${1:-hello from fel}"
SUNXI_FEL="${SUNXI_FEL:-sunxi-fel}"

if ! command -v "$SUNXI_FEL" >/dev/null 2>&1; then
    echo "!!! sunxi-fel не найден. Установи sunxi-tools." >&2
    exit 1
fi

if ! "$SUNXI_FEL" ver >/dev/null 2>&1; then
    echo "!!! Устройство не в FEL-режиме (нужно: удерживать S6 + RESET)." >&2
    exit 1
fi

# --- Инициализация UART0 ---
# CCU.APB2_GATING:    0x01C2006C bit16 = bus_uart0_en
# CCU.APB2_RST:       0x01C202D8 bit16 = uart0_reset (1 = снят)
# PIO PB_CFG2:        PB8=3 (UART0_TX), PB9=3 (UART0_RX)
# UART0:              LCR=DLAB(8N1) => DLL=13, DLH=0 (115200 при APB2=24 МГц)

cur_pio=$("$SUNXI_FEL" readl 0x01C20828)
pio_cfg=$(( (cur_pio & ~0xFF) | 0x33 ))

ARGS=(
    writel 0x01C2006C 0x00010000
    writel 0x01C202D8 0x00010000
    writel 0x01C20828 "0x$(printf '%08X' "$pio_cfg")"
    writel 0x01C2800C 0x83
    writel 0x01C28000 0x0D
    writel 0x01C28004 0x00
    writel 0x01C2800C 0x03
)

# --- Отправка строки (посимвольно в THR) ---
for (( i = 0; i < ${#MSG}; i++ )); do
    ch="${MSG:i:1}"
    printf -v code '%d' "'$ch"
    ARGS+=( writel 0x01C28000 "$code" )
done

# Переход строки (CR LF) — как в прошивке hello
ARGS+=( writel 0x01C28000 13 writel 0x01C28000 10 )

"$SUNXI_FEL" "${ARGS[@]}"

printf 'Sent: %s\n' "$MSG"