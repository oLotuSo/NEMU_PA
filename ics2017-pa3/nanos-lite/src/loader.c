#include "common.h"
#include "fs.h"
#define DEFAULT_ENTRY ((void *)0x4000000)

extern void ramdisk_read(void *, uint32_t, uint32_t);
extern size_t get_ramdisk_size();
int fs_open(const char*, int, int);
size_t fs_filesz(int);
ssize_t fs_read(int, void *, size_t);
int fs_close(int);

uintptr_t loader(_Protect *as, const char *filename) {
  int fd = fs_open(filename, 0, 0);
  size_t f_size = fs_filesz(fd);
  Log("Load %d bytes file, named %s, fd %d", f_size, filename, fd);
  fs_read(fd, DEFAULT_ENTRY, f_size);
  fs_close(fd);
  //ramdisk_read(DEFAULT_ENTRY, 0, get_ramdisk_size());
  return (uintptr_t)DEFAULT_ENTRY;
}
