# V3s Bare-Metal Hello

Bare-metal проект для Allwinner V3s: без операционной системы и bootloader,
написанный с нуля «Hello from V3s!» на UART0 плюс таймерные задержки.
Код компилируется в ARM (Cortex-A7), загружается через USB (FEL) в SRAM
и работает без оболочки, Linux и загрузочных образов.

- Язык: C11 + asm (ARMv7-A), arm-none-eabi-gcc + CMake/Ninja
- Загрузка: `sunxi-fel` (USB FEL mode)
- Вывод: UART0, 115200 8N1 (PB8/PB9)
- Таймер: sun4i-совместимый TMR0, тактируется OSC24M

## Платформа — LC-PI-V3S (CherryPi PC V3S)

Описание платы: http://wiki-en.lcmaker.com/index.php?title=LC-PI-V3S

- Плата: LCPI-PC-V3S_V1.2, SoC Allwinner V3s
- CPU: ARM Cortex-A7, до 1.2 GHz
- RAM: встроенная 64 МБ DDR2 (SoC)
- Накопитель: по факту SPI NAND MX35LF1GE4 (128 МБ);
  в вики упомянут «128Mbit SPI Nor Flash» — на плате установлен именно NAND
- WiFi: ESP8089; Ethernet: 100M (встроенный EPHY)
- USB OTG (micro-USB), MIPI CSI 24 pin, LCD 480x272 (40 pin)
- Аудио 3.5 мм, микрофон, слот TF-карты
- Родная прошивка: PhoenixSuit (режим прошивки — зажать S6, нажать RESET).
  Консоль: U0T/U0R/GND, 115200, логин `root`, пароль пустой
- Канальный терминал и дампы: все команды в этом репозитории рассчитаны на UART0

Замечание по загрузке: на плате проверено, что образ `LCPI-v3s-linux-2020-07-14.img`
(`IMAGEWTY`, контейнер для PhoenixSuit) на SD-карту залить как загрузочный нельзя —
это raw-образ для прошивки в NAND (подробнее в разделе «Исследование прошивки»).

## Загрузка через FEL

1. Подать питание, удерживая **S6**, затем нажать **RESET** и отпустить S6 —
   плата войдёт в FEL (USB устройство `1f3a:efe8`).
2. Проверить: `sunxi-fel version / lsusb`.
3. Прошивку (256-байтный binary) загружаем начиная с адреса `0x2000`:
   `sunxi-fel write 0x2000 hello.bin; sunxi-fel execute 0x2000`.

Важно: `sunxi-fel execute` передаёт управление как A32-функцию — код обязан быть
скомпилирован в **ARM**, а не Thumb (поэтому в проекте флаг `-marm`, а в
`startup.S` директива `.arm`). В Thumb-режиме плата «повисает» до `main`
(первый байт интерпретируется как A32-инструкция).

## Структура проекта

```
├── CMakeLists.txt            # сборка hello.elf/hello.bin + цели flash/reset
├── link.ld                   # SRAM 0x2000..0x8000, стек 0x4000
├── cmake/toolchain-arm.cmake # кросс-тулчейн arm-none-eabi
├── src/
│   ├── startup.S             # .arm, установка стека, bl main
│   ├── main.c                # uart_init -> "Hello from V3s!" -> Tick...
│   ├── uart.c / uart.h       # UART0: CCU gate/reset, PB8/PB9 mux3, 115200
│   ├── delay.c / delay.h     # delay_ms на базе TMR0 (OSC24M)
│   ├── ephy.c / ephy.h       # управление встроенным EPHY (ETH_LED, MII-доступ)
│   ├── lradc.c / lradc.h     # LRADC (KEYADC0): 6-бит АЦП кнопок, вывод raw и mV
│   ├── ccu.h                 # регистры тактирования
│   └── pio.h                 # регистры портов PB
├── tools/
│   ├── fel_uart_hello.sh     # "hello from fel" через FEL, без запуска кода
│   ├── fel_dump_flash.sh     # дамп SPI через spiflash-read
│   ├── uboot_dump.py         # дамп разделов NAND через U-Boot console UART
│   └── hexdump2bin.py        # raw hex-лог md.b -> бинарный файл
├── dump/                     # результаты дампов (в .gitignore)
└── docs/                     # местные заметки и спецификации
```

## Сборка и запуск

Требуется: CMake >= 3.16, Ninja, arm-none-eabi-gcc, sunxi-fel.

```sh
cmake -B build
ninja -C build            # соберёт build/hello.bin
ninja -C build flash      # загрузит hello.bin через FEL (сначала войти в FEL!)
ninja -C build reset      # wdreset: перезагрузка V3s, возврат в FEL
```

Терминал: UART0 на **PB8 (TX)** и **PB9 (RX)**, 115200 8N1:

```
Hello from V3s!
UART0 @ 115200 baud, bare metal, no bootloader.
Tick...
Tick...
```

## Как это работает

1. `startup.S`: `.arm`, указывает `sp = 0x4000` и вызывает `main`.
2. `uart_init()`:
   - включает такты: `CCU bus_gating2 |= APB1_PIO`, `bus_gating3 |= APB2_UART0`,
     `bus_soft_rst4 |= APB2_UART0`;
   - PB8/PB9 на mux3 (`PIO PB_CFG2 = 0x777733`);
   - UART0 (0x01C28000): LCR DLAB, DLL=13 (115200 при APB2=24 МГц), LCR=0x03, FCR=0x07.
3. `delay_ms()`: TMR0 (`0x01C20C10/14/18`, sun4i-стиль, всегда от OSC24M):
   `INT=0xFFFFFFFF`, `CTL = EN|RELOAD|CLK_OSC24M`, счётчик уменьшается от 0xFFFFFFFF,
   1 мс = 24000 тиков; сравнение со знаком устойчиво к переполнению.
4. `ephy_init()`: управление светодиодами Ethernet (ETH_LED_LINK/ETH_LED_SPD).
   Это **не GPIO** — контакты 77/78 V3s (EPHY_LINK_LED/EPHY_SPD_LED) это выходы
   встроенного PHY. `ephy.c` тактирует EMAC/EPHY (CCU gate/reset), включает EPHY
   (`SYS_CFG EMAC_EPHY_CLK_REG 0x01C00030`: SHUTDOWN=0, PHY_SELECT=1, CLK=24 МГц,
   PHY addr 1) и даёт MII-доступ через EMAC MDIO (`0x01C30010/14/18/1C`).
   Светодиоды отражают линк (EPHY_LINK_LED) и скорость 10/100 (EPHY_SPD_LED);
   полярность задаёт бит LED_POL, питание PHY — бит SHUTDOWN.
5. `lradc_init()` / `dump_keyadc0()`: чтение KEYADC0 (LRADC, глава 4.11 даташита).
   LRADC тактируется от APB, отдельного gate/reset в CCU **нет** — достаточно
   `LRADC_CTRL (0x01C22800) |= LRADC_EN`. 6-бит результат в `LRADC_DATA0`
   (`bits 5:0`), шкала 0~3.0 В (LSB ≈ 47 мВ): `mV = raw * 3000 / 64`.
   Без нажатия на входе ~3.0 В (raw ≈ 0x3F); кнопки через делители дают меньше:
   SELECT ≈ 0.2 В, START ≈ 0.4 В, VOL+ ≈ 0.6 В, VOL- ≈ 0.8 В (разводка платы).

Отладочные находки, учтённые в коде:
- `sunxi-fel` v1.4.2 не умеет `setbits/clrsetbits` — только `readl/writel`
  (RMW делается руками, см. `fel_uart_hello.sh`).
- при `-Os` опасное сворачивание арифметики указателей давало неверный адрес
  таймера (0x03841000 вместо 0x01C20C00) — в `delay.c` используются абсолютные
  адреса через `(volatile uint32_t *)(uintptr_t)`.

## Инструменты

- `tools/fel_uart_hello.sh` — вывести `hello from fel` на UART0 через FEL
  (прямые `write`, без запуска своей программы).
- `tools/fel_dump_flash.sh` — дамп SPI-флэша через `sunxi-fel spiflash-*`,
  результат ложится в `dump/`.
- `tools/uboot_dump.py` — автоматизированный дамп NAND через консоль U-Boot:
  `sunxi_flash read <addr> <part> <bytes>` + `md.b` по UART (115200),
  `setenv baudrate 1500000` на лету **не применяется** этим U-Boot.
- `tools/hexdump2bin.py <hex.raw> <out.bin>` — пересборка hex-дампа `md.b`
  обратно в бинарник.

## Исследование прошивки (dump/)

Собранные дампы и выводы (плата LC-PI-V3S, SPI NAND 128 МБ):
- `dump/flash_20260920_113302.bin` (16 МБ) — сырой `spiflash-read` NOR-протоколом
  против NAND-чипа: «мусор», без строк и сигнатур (есть ECC/scrambler NAND).
- `dump/nand_bootloader_1MB.bin` (1 МБ) — раздел `bootloader` прочитан через
  U-Boot (`sunxi_flash read ... bootloader 0x100000` + `md.b`): это FAT16
  boot-resource (JMP `E9`, OEM `FAT16`, метка `Volumn`, `55 AA`) — той же природы,
  что и в заводском образе. Самих eGON/U-Boot в разделе нет.
- Официальный фабричный образ `LCPI-v3s-linux-2020-07-14.img` (88 МБ):
  контейнер `IMAGEWTY` для PhoenixSuit (запись в NAND); содержит копии
  `eGON.BT0` (boot0 v4.0.0) и `U-Boot 2011.09-rc1 (Jul 14 2020) Allwinner
  Technology` — совпадает со штатным boot-логом платы. Как SD-образ не
  загружается (нет boot0 на SD-смещениях, нет MBR).

Партиции штатной прошивки (из fastboot-лога):
`bootloader` 0x100000/0x800000, `env` 0x900000/0x80000, `boot` 0x980000/0x800000,
`system` 0x1180000/0x5a00000, `UDISK` 0x6b80000/0.

## Ссылки

- LC-PI-V3S (вики LCTech): http://wiki-en.lcmaker.com/index.php?title=LC-PI-V3S
- Готовый SDK/прошивка: `CherryPi_V3S-Build-2020-07-14` (см. раздел «Source code
  compilation» на вики)