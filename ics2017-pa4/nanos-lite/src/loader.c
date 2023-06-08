#include "common.h"
#include "fs.h"
#define DEFAULT_ENTRY ((void *)0x8048000)

extern void ramdisk_read(void *, uint32_t, uint32_t);
extern size_t get_ramdisk_size();
extern void* new_page(void);
int fs_open(const char*, int, int);
size_t fs_filesz(int);
ssize_t fs_read(int, void *, size_t);
int fs_close(int);


uintptr_t loader(_Protect *as, const char *filename) {
  int fd = fs_open(filename, 0, 0);
  int f_size = fs_filesz(fd);
  Log("Load %d bytes file, named %s, fd %d", f_size, filename, fd);
  void *pa = DEFAULT_ENTRY;
  void *va = DEFAULT_ENTRY;
  while (f_size > 0){
    pa = new_page();
    Log("Map va to pa: 0x%08x to 0x%08x", va, pa);
    _map(as, va, pa);
    fs_read(fd, pa, PGSIZE);
    va += PGSIZE;
    f_size -= PGSIZE;
  }
  fs_close(fd);
  //ramdisk_read(DEFAULT_ENTRY, 0, get_ramdisk_size());
  return (uintptr_t)DEFAULT_ENTRY;
}
