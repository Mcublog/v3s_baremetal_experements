#!/usr/bin/env bash
# Дамп onboard SPI NOR (W25Q128, 16 МБ) платы V3s через FEL.
# Usage: fel_dump_flash.sh [--len 0x1000000] [--offset 0] [--name ИМЯ]

set -euo pipefail

SUNXI_FEL="${SUNXI_FEL:-sunxi-fel}"
PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DUMP_DIR="${PROJECT_ROOT}/dump"
LEN="0x1000000"
OFFSET="0"
NAME=""

while [ $# -gt 0 ]; do
    case "$1" in
        --len)   LEN="$2";    shift 2 ;;
        --offset) OFFSET="$2"; shift 2 ;;
        --name)  NAME="$2";   shift 2 ;;
        -h|--help)
            echo "Usage: $0 [--len 0x1000000] [--offset 0] [--name ИМЯ]"
            exit 0
            ;;
        *) echo "!!! Неизвестный аргумент: $1" >&2; exit 1 ;;
    esac
done

if ! command -v "$SUNXI_FEL" >/dev/null 2>&1; then
    echo "!!! sunxi-fel не найден. Установи sunxi-tools." >&2
    exit 1
fi

if ! "$SUNXI_FEL" ver >/dev/null 2>&1; then
    echo "!!! Устройство не в FEL-режиме (нужно: удерживать S6 + RESET)." >&2
    exit 1
fi

mkdir -p "$DUMP_DIR"

if [ -z "$NAME" ]; then
    STAMP=$(date +%Y%m%d_%H%M%S)
    NAME="flash_${STAMP}"
fi

OUT_FILE="${DUMP_DIR}/${NAME}.bin"

echo "=== Информация о SPI flash ==="
"$SUNXI_FEL" spiflash-info

echo
echo "=== Чтение flash: offset=$OFFSET len=$LEN ==="
"$SUNXI_FEL" spiflash-read "$OFFSET" "$LEN" "$OUT_FILE"

echo
echo "=== Готово ==="
echo "Файл: $(readlink -f "$OUT_FILE")"
echo "Размер: $(stat -c %s "$OUT_FILE") байт ($LEN ожидается)"
echo "Первые 32 байта:"
xxd -l 32 "$OUT_FILE" 2>/dev/null || od -A x -t x1z -N 32 "$OUT_FILE"