#include <stddef.h>
#include <tinyalloc.h>
void* operator new(unsigned int len) {
  return malloc(len);
}

