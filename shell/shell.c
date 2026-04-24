#include "shell.h"
#include <vga.h>
#include <keyboard.h>
#include <timer.h>
#include <printf.h>
#include <string.h>
#include <pmm.h>
#include <heap.h>
#include <paging.h>

#define CMD_BUF_SIZE 256
#define HISTORY_SIZE 16
#define PROMPT       "corekernel> "

static char cmd_buf[CMD_BUF_SIZE];
static char history[HISTORY_SIZE][CMD_BUF_SIZE];
static int  history_count = 0;
static int  history_pos   = 0;

/* ---- Built-in commands ---- */

static void cmd_help(void) {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    kprintf("\n  CoreKernel Built-in Commands\n");
    kprintf("  ============================\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    kprintf("  help      - Show this help\n");
    kprintf("  clear     - Clear the screen\n");
    kprintf("  uname     - Kernel information\n");
    kprintf("  uptime    - System uptime\n");
    kprintf("  meminfo   - Memory statistics\n");
    kprintf("  cpuinfo   - CPU information\n");
    kprintf("  reboot    - Reboot the system\n");
    kprintf("  halt      - Halt the system\n");
    kprintf("  echo <x>  - Print arguments\n");
    kprintf("  color <n> - Change text color (0-15)\n");
    kprintf("  crash     - Trigger divide-by-zero (test ISR)\n");
    kprintf("  pfault    - Trigger page fault (test handler)\n");
    kprintf("  mtest     - Test kmalloc/kfree\n");
    kprintf("\n");
}

static void cmd_uname(void) {
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    kprintf("CoreKernel 0.1.0 x86 (i686) SMP PREEMPT\n");
    kprintf("Compiled: " __DATE__ " " __TIME__ "\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

static void cmd_uptime(void) {
    uint32_t secs  = timer_get_seconds();
    uint32_t mins  = secs / 60;
    uint32_t hours = mins / 60;
    kprintf("Uptime: %u hours, %u minutes, %u seconds (%u ticks)\n",
            hours, mins % 60, secs % 60, timer_get_ticks());
}

static void cmd_meminfo(void) {
    uint32_t total = pmm_total_pages() * 4;
    uint32_t free  = pmm_free_pages()  * 4;
    uint32_t used  = pmm_used_pages()  * 4;
    vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    kprintf("Physical Memory\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    kprintf("  Total : %6u KiB  (%u MiB)\n", total, total / 1024);
    kprintf("  Used  : %6u KiB  (%u MiB)\n", used,  used  / 1024);
    kprintf("  Free  : %6u KiB  (%u MiB)\n", free,  free  / 1024);
    kprintf("Kernel Heap\n");
    kprintf("  Used  : %u bytes\n",      heap_used());
    kprintf("  Free  : %u bytes\n",      heap_available());
}

static void cmd_cpuinfo(void) {
    uint32_t eax, ebx, ecx, edx;
    char vendor[13] = {0};
    __asm__ volatile("cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(0));
    ((uint32_t *)vendor)[0] = ebx;
    ((uint32_t *)vendor)[1] = edx;
    ((uint32_t *)vendor)[2] = ecx;
    vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    kprintf("CPU Information\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    kprintf("  Vendor   : %s\n", vendor);
    kprintf("  Max CPUID: %u\n", eax);
    __asm__ volatile("cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(1));
    kprintf("  Family   : %u  Model: %u  Stepping: %u\n",
            (eax >> 8) & 0xF, (eax >> 4) & 0xF, eax & 0xF);
    kprintf("  Features : %s%s%s%s\n",
            edx & (1<<0)  ? "FPU "  : "",
            edx & (1<<23) ? "MMX "  : "",
            edx & (1<<25) ? "SSE "  : "",
            edx & (1<<26) ? "SSE2 " : "");
}

static void cmd_reboot(void) {
    kprintf("Rebooting...\n");
    timer_sleep(500);
    /* Pulse the keyboard controller reset line */
    uint8_t good = 0x02;
    while (good & 0x02)
        good = (uint8_t)({ uint8_t r; __asm__ volatile("inb $0x64,%0":"=a"(r)); r; });
    __asm__ volatile("outb %0,$0x64" : : "a"((uint8_t)0xFE));
    __asm__ volatile("cli; hlt");
}

static void cmd_halt(void) {
    vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
    kprintf("System halted. Power off safely.\n");
    __asm__ volatile("cli");
    while (1) __asm__ volatile("hlt");
}

static void cmd_echo(const char *args) {
    kprintf("%s\n", args ? args : "");
}

static void cmd_color(const char *args) {
    if (!args) { kprintf("Usage: color <0-15>\n"); return; }
    int c = (int)strtol(args, NULL, 10);
    if (c < 0 || c > 15) { kprintf("Color must be 0-15\n"); return; }
    vga_set_color((vga_color_t)c, VGA_COLOR_BLACK);
    kprintf("Color changed to %d\n", c);
}

static void cmd_crash(void) {
    kprintf("Triggering divide-by-zero...\n");
    volatile int x = 0;
    volatile int y = 1 / x;
    (void)y;
}

static void cmd_pfault(void) {
    kprintf("Triggering page fault at 0xDEADBEEF...\n");
    volatile uint32_t *ptr = (volatile uint32_t *)0xDEADBEEF;
    volatile uint32_t  val = *ptr;
    (void)val;
}

static void cmd_mtest(void) {
    kprintf("Testing heap allocator...\n");
    void *a = kmalloc(64);
    void *b = kmalloc(128);
    void *c = kmalloc(256);
    kprintf("  kmalloc(64)  = %p\n", a);
    kprintf("  kmalloc(128) = %p\n", b);
    kprintf("  kmalloc(256) = %p\n", c);
    kfree(b);
    void *d = kmalloc(100);
    kprintf("  kfree(b), kmalloc(100) = %p  (reused: %s)\n",
            d, d == b ? "yes" : "no");
    kfree(a); kfree(c); kfree(d);
    kprintf("  All freed. Heap used: %u bytes\n", heap_used());
    kprintf("  PASS\n");
}

/* ---- Input ---- */

static void readline(char *buf, int max) {
    int pos = 0;
    buf[0] = '\0';
    while (1) {
        char c = keyboard_getchar();
        if (c == '\n') {
            vga_put_char('\n');
            buf[pos] = '\0';
            return;
        }
        if (c == '\b' && pos > 0) {
            pos--;
            buf[pos] = '\0';
            vga_put_char('\b');
        } else if (c >= ' ' && pos < max - 1) {
            buf[pos++] = c;
            buf[pos]   = '\0';
            vga_put_char(c);
        }
    }
}

static void history_add(const char *cmd) {
    if (!*cmd) return;
    strncpy(history[history_count % HISTORY_SIZE], cmd, CMD_BUF_SIZE - 1);
    history_count++;
    history_pos = history_count;
}

/* ---- Dispatch ---- */

static void execute(char *line) {
    /* Trim leading whitespace */
    while (*line == ' ') line++;
    if (!*line) return;

    history_add(line);

    char *cmd  = strtok(line, " ");
    char *args = strtok(NULL, "");
    if (args) while (*args == ' ') args++;

    if      (!strcmp(cmd, "help"))    cmd_help();
    else if (!strcmp(cmd, "clear"))   { vga_clear(); }
    else if (!strcmp(cmd, "uname"))   cmd_uname();
    else if (!strcmp(cmd, "uptime"))  cmd_uptime();
    else if (!strcmp(cmd, "meminfo")) cmd_meminfo();
    else if (!strcmp(cmd, "cpuinfo")) cmd_cpuinfo();
    else if (!strcmp(cmd, "reboot"))  cmd_reboot();
    else if (!strcmp(cmd, "halt"))    cmd_halt();
    else if (!strcmp(cmd, "echo"))    cmd_echo(args);
    else if (!strcmp(cmd, "color"))   cmd_color(args);
    else if (!strcmp(cmd, "crash"))   cmd_crash();
    else if (!strcmp(cmd, "pfault"))  cmd_pfault();
    else if (!strcmp(cmd, "mtest"))   cmd_mtest();
    else {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        kprintf("Unknown command: '%s'  (type 'help' for list)\n", cmd);
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    }
}

void shell_run(void) {
    while (1) {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        kprintf(PROMPT);
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        readline(cmd_buf, CMD_BUF_SIZE);
        execute(cmd_buf);
    }
}
