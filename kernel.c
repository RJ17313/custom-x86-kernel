#include <stdint.h>

typedef uint32_t size_t;

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((volatile uint16_t*) 0xB8000)
#define MAX_BUFFER 256

void update_cursor(size_t row, size_t col);
void terminal_clear(void);
void terminal_put_char(char c);
void terminal_scroll(void);

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

uint8_t keyboard_has_input(void) {
    return inb(0x64) & 1;
}

uint8_t keyboard_read_scancode(void) {
    while (!keyboard_has_input());
    return inb(0x60);
}

static const char scancode_to_ascii[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
     0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
     0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
   '*',   0, ' '
};

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) return 0;
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

char keyboard_get_char(void) {
    while (1) {
        uint8_t scancode = keyboard_read_scancode();

        if (scancode & 0x80) {
            continue;
        }

        if (scancode < sizeof(scancode_to_ascii)) {
            char c = scancode_to_ascii[scancode];
            if (c != 0) return c;
        }
    }
}

enum vga_color
{
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;

// volatile uint16_t *const VGA_BUFER = (volatile uint16_t *)0xB8000;

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | (bg << 4);
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t)uc | (uint16_t)color << 8;
}

void terminal_clear(void) {
    terminal_row = 0;
    terminal_column = 0;

    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            VGA_MEMORY[index] = vga_entry(' ', terminal_color);
        }
    }

    update_cursor(0, 0);
}

void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    terminal_clear();
}

void terminal_put_char(char c) {
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
        }
        update_cursor(terminal_row, terminal_column);
        return;
    }

    const size_t index = terminal_row * VGA_WIDTH + terminal_column;
    VGA_MEMORY[index] = vga_entry(c, terminal_color);

    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
        }
    }

    update_cursor(terminal_row, terminal_column);
}

void terminal_write_string(const char* data) {
    for (size_t i = 0; data[i] != '\0'; i++) {
        terminal_put_char(data[i]);
    }
}

void terminal_scroll(void) {
    for (size_t y = 0; y <VGA_HEIGHT -1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            size_t src_index = (y + 1) * VGA_WIDTH +x;
            size_t dest_index = y * VGA_WIDTH + x;
            VGA_MEMORY[dest_index] = VGA_MEMORY[src_index];
        }
    }

    size_t last_row = VGA_HEIGHT - 1;
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        size_t index = last_row * VGA_WIDTH +x;
        VGA_MEMORY[index] = vga_entry(' ', terminal_color);
    }

    terminal_row = VGA_HEIGHT - 1;
}

void update_cursor(size_t row, size_t col) {
    uint16_t pos = row * VGA_WIDTH + col;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void process_command(const char* buffer) {
    terminal_write_string("\n");

    if (strcmp(buffer, "help") == 0) {
        terminal_write_string("Available commands:\n");
        terminal_write_string("  help  - Display this menu\n");
        terminal_write_string("  clear - Clear the screen\n");
        terminal_write_string("  echo - Print a string to the terminal\n");
        terminal_write_string("  ver   - Show kernel version\n");
    }
    else if (strcmp(buffer, "clear") == 0) {
        terminal_clear();
        terminal_write_string("> ");
        return;
    }
    else if (strcmp(buffer, "ver") == 0) {
        terminal_write_string("Ryan's Kernel v0.1\n");
    }
    else if (strncmp(buffer, "echo ", 5) == 0) {
        terminal_write_string(buffer + 5);
        terminal_write_string("\n\n");
        
    }
    else if (strcmp(buffer, "echo") == 0) {
        terminal_write_string("\n");
    }
    else if (buffer[0] != '\0') {
        terminal_write_string("Unknown Command: ");
        terminal_write_string(buffer);
        terminal_write_string("\n");
    }

    terminal_write_string("> ");
}

void kernel_main(void)
{
    terminal_initialize();

    terminal_write_string("=================================\n");
    terminal_write_string("     RYAN'S CUSTOM KERNEL\n");
    terminal_write_string("=================================\n\n");
    terminal_write_string("Type 'help' for available commands. \n\n");

    char cmd_buffer[MAX_BUFFER];
    uint32_t buf_index = 0;

    while (1) {
        char c = keyboard_get_char();

        if (c == '\n') {
            cmd_buffer[buf_index] = '\0';
            process_command(cmd_buffer);
            buf_index = 0;
        }
        else if (c == '\b') {
            if (buf_index > 0) {
                buf_index--;

                if (terminal_column > 0) {
                    terminal_column--;
                    const size_t index = terminal_row * VGA_WIDTH + terminal_column;
                    VGA_MEMORY[index] = vga_entry(' ', terminal_color);
                    update_cursor(terminal_row, terminal_column);
                }
            }
        }
        else if (buf_index < MAX_BUFFER - 1) {
            cmd_buffer[buf_index++] = c;
            terminal_put_char(c);
        }
    }

}
