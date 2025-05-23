#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <stdint.h>

uint64_t sys_read(uint64_t fd, char *buf, uint64_t count);

uint64_t sys_write(uint64_t fd, const char *buf, uint64_t count);

#endif