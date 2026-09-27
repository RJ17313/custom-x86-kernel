CC = gcc
AS = as
LD = ld

CFLAGS = -m32 --freestanding -O2 -Wall -Wextra -std=gnu99
ASFLAGS = --32
LDFLAGS = -m elf_i386 -T linker.ld

KERNEL = customOS.bin
OBJS = boot.o kernel.o

all: $(KERNEL)

boot.o: boot.s
	$(AS) $(ASFLAGS) boot.s -o boot.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

$(KERNEL): $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o $(KERNEL) $(OBJS)

run: $(KERNEL)
	qemu-system-i386 -kernel $(KERNEL) -append ""

clean:
	rm -rf *.o $(KERNEL)

.PHONY: all run clean