#include <registers.h>
#include <syscalls.h>

static uint64_t (*intHandlers[])(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rcx, uint64_t r8, uint64_t r9) = {sys_read, sys_write};

uint64_t intDispatcher(uint64_t int_id, const registers_t *registers) {
    if (int_id >= sizeof(intHandlers) / sizeof(intHandlers[0]))
        return 0;

    return intHandlers[int_id](registers->rdi, registers->rsi, registers->rdx, registers->rcx, registers->r8, registers->r9);
}