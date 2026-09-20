#!/usr/bin/env python3
# Захват дампа NAND через U-Boot по UART (/dev/ttyUSB0).
# 1) nand read 0x41000000 0 0x1000000   (первые 16 МБ, ECC-декод драйвером)
# 2) setenv baudrate 1500000; saveenv   (разгон консоли)
# 3) md.b 0x41000000 0x1000000          (вывод hex-дампа)
# Результат: dump/nand_hex.raw

import os, re, select, subprocess, sys, time

PORT = os.environ.get("TTY", "/dev/ttyUSB0")
ADDR = os.environ.get("NAND_ADDR", "0x41000000")
OFF  = os.environ.get("NAND_OFF",  "0")
SIZE = os.environ.get("NAND_SIZE", "0x1000000")
BAUD_LO = 115200
BAUD_HI = 1500000
RAWFILE = "dump/nand_hex.raw"
TAIL_MAX = 1 << 16

PROMPT_RE = re.compile(rb"(sunxi#|=>)\s*$")

class Tail:
    """Держит только последние TAIL_MAX байт вывода и умеет дописывать лог."""
    def __init__(self):
        self._buf = bytearray()
    def push(self, data, log=None):
        self._buf += data
        if len(self._buf) > TAIL_MAX:
            del self._buf[:-TAIL_MAX]
        if log:
            log.write(data)
            log.flush()
    def getvalue(self):
        return bytes(self._buf)


def stty(baud):
    subprocess.run(["stty", "-F", PORT, str(baud), "cs8", "-cstopb", "-parenb",
                    "-ixon", "-ixoff", "-echo", "-echoe", "-echok", "-icanon",
                    "-opost", "-isig"], check=True)


def drain(fd):
    while select.select([fd], [], [], 0.1)[0]:
        try:
            os.read(fd, 4096)
        except OSError:
            break


def wait_for(fd, buf, timeout, log=None, progress=False, logpath=None):
    """Читает порт до появления prompt. Возвращает True, если prompt найден."""
    deadline = time.time() + timeout
    last_tick = time.time()
    while time.time() < deadline:
        if PROMPT_RE.search(buf.getvalue()):
            return True
        if select.select([fd], [], [], 0.2)[0]:
            try:
                chunk = os.read(fd, 65536)
            except OSError:
                chunk = b""
            if chunk:
                buf.push(chunk, log=log)
        if progress and time.time() - last_tick >= 20:
            last_tick = time.time()
            sz = os.path.getsize(logpath) if logpath else buf.getvalue().__len__()
            print(f"  ... {time.time()-deadline+timeout:6.1f} c, уловлено {sz} байт")
    return False


def send(fd, cmd):
    os.write(fd, cmd.encode() + b"\r")


def main():
    os.makedirs("dump", exist_ok=True)
    stty(BAUD_LO)
    fd = os.open(PORT, os.O_RDWR | os.O_NOCTTY | os.O_NDELAY)
    try:
        drain(fd)
        buf = Tail()

        # 1) Проверка prompt
        send(fd, "\r")
        if not wait_for(fd, buf, 5):
            print("Нет U-Boot prompt! Вывод:", buf.getvalue()[-200:])
            sys.exit(2)
        print("U-Boot prompt OK")

        # 2) nand info
        buf = Tail()
        send(fd, "nand info")
        wait_for(fd, buf, 10)
        print(buf.getvalue())

        # 3) nand read (ECC-декод)
        buf = Tail()
        send(fd, f"nand read {ADDR} {OFF} {SIZE}")
        wait_for(fd, buf, 120)
        out = buf.getvalue()
        print("read:", out[-180:])
        if b"Error" in out:
            sys.exit(3)

        # 4) Разгон консоли (применяется при рестарте U-Boot)
        buf = Tail()
        send(fd, "setenv bootdelay -1; setenv baudrate %d; saveenv; reset" % BAUD_HI)
        time.sleep(1.0)
        try:
            stty(BAUD_HI)
        except subprocess.CalledProcessError as e:
            print("Не удалось переключить baud:", e)
            stty(BAUD_LO)
            sys.exit(4)
        print("baud switched to", BAUD_HI, "- жду перезагрузки U-Boot...")
        if not wait_for(fd, buf, 120):
            print("Prompt после рестарта не появился:", buf.getvalue()[-200:])
            sys.exit(5)
        print("U-Boot prompt на", BAUD_HI, "OK")

        # 5) md.b — hex-дамп в dump/nand_hex.raw
        buf = Tail()
        send(fd, "\r")
        wait_for(fd, buf, 2)
        with open(RAWFILE, "wb") as log:
            send(fd, f"md.b {ADDR} {SIZE}")
            start = time.time()
            done = wait_for(fd, buf, 1800, log=log, progress=True, logpath=RAWFILE)
            elapsed = time.time() - start
        if not done:
            print("md.b не завершился за 900 c")
            sys.exit(6)
        total = os.path.getsize(RAWFILE)
        print(f"md.b завершён за {elapsed:.1f} c, лог {total} байт -> {RAWFILE}")

        # 6) Возврат скорости и bootdelay
        send(fd, "setenv bootdelay 1; setenv baudrate %d; saveenv" % BAUD_LO)
        time.sleep(1.0)
        try:
            stty(BAUD_LO)
        except subprocess.CalledProcessError:
            pass
        print("baud returned to", BAUD_LO)
    finally:
        try:
            os.close(fd)
        except OSError:
            pass


if __name__ == "__main__":
    main()