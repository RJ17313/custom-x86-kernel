# custom-x86-kernel

> Bare-metal C experiments at `0xB8000`. A custom 32-bit x86 CLI kernel running live in QEMU.

A lightweight, freestanding 32-bit x86 kernel built from scratch using C and x86 Assembly. Runs in IA-32 protected mode without standard C library (`libc`) dependencies. I built it because I have been learning C and have gotten very interested with low level systems. I may add more for fun later on.

---

## Features

* **Direct VRAM Mapping**: Framebuffer text rendering via direct memory writes to `0xB8000`.
* **Hardware Scrolling & Cursor Sync**: Syncs physical cursor position via CRT controller ports (`0x3D4`/`0x3D5`) and shifts VRAM memory buffers on screen overflow.
* **PS/2 Keyboard Input**: Polled scan-code decoder mapped to IBM Code Page 437 / ASCII translations.
* **Custom CLI Shell**: Custom freestanding string manipulation utilities (`strcmp`, `strncmp`) parsing interactive commands.
* **Zero Dependencies**: Built with `-ffreestanding` and a custom linker script—no runtime overhead or external libraries.

---

## Built-in Shell Commands

| Command | Description |
|---|---|
| `help` | Displays available kernel commands |
| `clear` | Clears the VGA framebuffer and resets cursor position |
| `ver` | Displays kernel version info |
| `echo <msg>` | Prints the provided string back to the terminal |

---

## Tech Stack & Architecture

* **Architecture**: 32-bit x86 (IA-32 Protected Mode)
* **Language**: C, x86 NASM / GAS Assembly
* **Build System**: `make`, `i686-elf-gcc` / `gcc -m32`, `ld`
* **Emulation**: QEMU (`qemu-system-i386`)

---

## Building & Running

### Prerequisites
Make sure you have `gcc` (with 32-bit multilib support or `i686-elf-gcc`), `nasm`, `make`, and `qemu-system-i386` installed.

### Run in QEMU
```bash
# Clone the repository
git clone https://github.com/RJ17313/custom-x86-kernel.git
cd custom-x86-kernel

# Build and launch in QEMU
make run
