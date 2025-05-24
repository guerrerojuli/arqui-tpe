#include <registers.h>
#include <syscalls.h>

static uint64_t (*intHandlers[])(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rcx, uint64_t r8, uint64_t r9) = {sys_read, sys_write};

static void printDebugInfo(uint64_t int_id) {
    char a[] = "Llegue a dispatcher";
    sys_write(1, a, sizeof(a));
    char buffer[21];
    int len = 0;
    uint64_t n = int_id;
    if (n == 0) {
        buffer[len++] = '0';
    } else {
        char temp[21];
        int temp_len = 0;
        while (n > 0 && temp_len < 20) {
            temp[temp_len++] = '0' + (n % 10);
            n /= 10;
        }
        for (int i = temp_len - 1; i >= 0; i--) {
            buffer[len++] = temp[i];
        }
    }
    sys_write(1, buffer, len);
}

uint64_t intDispatcher(const registers_t *registers) {
    if (registers->rax >= sizeof(intHandlers) / sizeof(intHandlers[0]))
        return 0;

    return intHandlers[registers->rax](registers->rdi, registers->rsi, registers->rdx, registers->rcx, registers->r8, registers->r9);
}