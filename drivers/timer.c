#include "timer.h"
#include <irq.h>
#include <io.h>

#define PIT_CHANNEL0  0x40
#define PIT_COMMAND   0x43

static volatile uint32_t ticks = 0;

static void timer_callback(registers_t *regs) {
    (void)regs;
    ticks++;
}

void timer_init(uint32_t hz) {
    irq_register_handler(0, timer_callback);

    uint32_t divisor = PIT_HZ / hz;
    outb(PIT_COMMAND, 0x36);                     /* channel 0, lo/hi, mode 3 */
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
}

uint32_t timer_get_ticks(void) {
    return ticks;
}

uint32_t timer_get_seconds(void) {
    return ticks / TIMER_FREQ_HZ;
}

void timer_sleep(uint32_t ms) {
    uint32_t target = ticks + (ms * TIMER_FREQ_HZ / 1000) + 1;
    while (ticks < target) __asm__ volatile("hlt");
}
