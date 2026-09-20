#!/usr/bin/env python3
# Конвертация hex-дампа md.b (.raw) в бинарный файл.
# Usage: hexdump2bin.py <файл.raw> <выход.bin>
# Строки вида: 41000000: 00 11 22 ... 33   |

import re, sys

LINE_RE = re.compile(r"^([0-9a-fA-F]{8}):((?:[ \t][0-9a-fA-F]{2})+)")


def main():
    if len(sys.argv) != 3:
        print("Usage: hexdump2bin.py <raw> <out.bin>", file=sys.stderr)
        sys.exit(1)
    raw, out = sys.argv[1], sys.argv[2]

    total = 0
    with open(out, "wb") as f:
        with open(raw) as src:
            for line in src:
                m = LINE_RE.match(line)
                if not m:
                    continue
                addr = int(m.group(1), 16)
                if total != addr:
                    if total == 0:
                        print(f"старт адрес 0x{addr:x}, пропускаю префикс", file=sys.stderr)
                        total = addr
                    else:
                        print(f"пропуск: жду 0x{total:x}, встретил 0x{addr:x}", file=sys.stderr)
                hexs = m.group(2).split()
                blob = bytes(int(h, 16) for h in hexs)
                f.write(blob)
                total += len(blob)

    print(f"Записано {total} байт -> {out}")


if __name__ == "__main__":
    main()