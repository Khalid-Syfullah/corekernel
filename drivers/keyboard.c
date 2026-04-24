#include "keyboard.h"
#include <irq.h>
#include <io.h>

#define KB_DATA_PORT    0x60
#define KB_STATUS_PORT  0x64

/* US QWERTY scancode set 1 — lowercase */
static const char sc_ascii[] = {
    0,    0,   '1', '2', '3', '4', '5', '6',  /* 0x00-0x07 */
   '7',  '8', '9', '0', '-', '=',  '\b', '\t', /* 0x08-0x0F */
   'q',  'w', 'e', 'r', 't', 'y', 'u', 'i',   /* 0x10-0x17 */
   'o',  'p', '[', ']', '\n',  0,  'a', 's',   /* 0x18-0x1F */
   'd',  'f', 'g', 'h', 'j', 'k', 'l', ';',   /* 0x20-0x27 */
   '\'', '`',  0,  '\\','z', 'x', 'c', 'v',   /* 0x28-0x2F */
   'b',  'n', 'm', ',', '.', '/',  0,   '*',   /* 0x30-0x37 */
    0,   ' ',  0,   0,   0,   0,   0,   0,     /* 0x38-0x3F */
};

/* Shifted version */
static const char sc_shift[] = {
    0,    0,   '!', '@', '#', '$', '%', '^',
   '&',  '*', '(', ')', '_', '+',  '\b', '\t',
   'Q',  'W', 'E', 'R', 'T', 'Y', 'U', 'I',
   'O',  'P', '{', '}', '\n',  0,  'A', 'S',
   'D',  'F', 'G', 'H', 'J', 'K', 'L', ':',
   '"',  '~',  0,  '|', 'Z', 'X', 'C', 'V',
   'B',  'N', 'M', '<', '>', '?',  0,   '*',
    0,   ' ',  0,   0,   0,   0,   0,   0,
};

static char    kb_buf[KB_BUFFER_SIZE];
static uint8_t kb_head = 0;
static uint8_t kb_tail = 0;
static bool    shift_down = false;
static bool    caps_lock  = false;

static void kb_buf_push(char c) {
    uint8_t next = (kb_head + 1) % KB_BUFFER_SIZE;
    if (next != kb_tail) {
        kb_buf[kb_head] = c;
        kb_head = next;
    }
}

static void keyboard_callback(registers_t *regs) {
    (void)regs;
    uint8_t scancode = inb(KB_DATA_PORT);

    bool released = (scancode & 0x80) != 0;
    uint8_t sc    = scancode & 0x7F;

    /* Track shift keys */
    if (sc == 0x2A || sc == 0x36) { shift_down = !released; return; }
    /* Caps lock toggle on press */
    if (sc == 0x3A && !released)  { caps_lock = !caps_lock; return; }
    /* Ignore releases and scancodes out of table */
    if (released || sc >= sizeof(sc_ascii)) return;

    char c = shift_down ? sc_shift[sc] : sc_ascii[sc];
    if (c == 0) return;

    /* Caps lock affects only letters */
    if (caps_lock && c >= 'a' && c <= 'z') c -= 32;
    if (caps_lock && c >= 'A' && c <= 'Z') c += 32;

    kb_buf_push(c);
}

void keyboard_init(void) {
    irq_register_handler(1, keyboard_callback);
}

bool keyboard_buffer_empty(void) {
    return kb_head == kb_tail;
}

char keyboard_poll(void) {
    if (kb_head == kb_tail) return 0;
    char c = kb_buf[kb_tail];
    kb_tail = (kb_tail + 1) % KB_BUFFER_SIZE;
    return c;
}

char keyboard_getchar(void) {
    while (keyboard_buffer_empty())
        __asm__ volatile("hlt");
    return keyboard_poll();
}
