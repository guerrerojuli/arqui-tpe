#include <syscalls.h>
#include <videoDriver.h>

uint64_t sys_read(uint64_t fd, char *buf, uint64_t count) {
  return 0;
}

uint64_t sys_write(uint64_t fd, const char *buf, uint64_t count) {
  switch (fd) {
    case 1:
      write_to_video_text_buffer(buf, count, 0xFFFFFF);
      return count;
    case 2:
      write_to_video_text_buffer(buf, count, 0xFF0000);
      return count;
    default:
      return 0;
  }
}