#include <time.h>
#include <registers.h>

static void (*intHandlers[])(const registers_t *) = {timer_handler};

void irqDispatcher(uint64_t irq, const registers_t *registers) {
    if (irq >= sizeof(intHandlers) / sizeof(intHandlers[0]))
        return;

    intHandlers[irq](registers);
}