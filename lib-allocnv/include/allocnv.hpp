#include <stddef.h>
#include <allocnv.h>
void* operator new(__attribute__((unused)) unsigned int len, void *addr) {
  return addr;
}

template <class T, typename... Ts> T *cine(size_t handle, Ts... ts) {
  void *ptr = pmalloc(handle, sizeof(T));
  if(*((int*) ptr) == 0) { // first byte in an initialized object must not be 0
    return (T*) new(ptr) T(ts...); // this calls the constructor
  }
  return (T*) ptr;
}

template <class T> void die(size_t handle) {
  // get the ptr to the handle, or shortly allocate memory as collateral damage
  T *ptr = (T*) pmalloc(handle, sizeof(T));
  if(*((int*) ptr) != 0) {
    // we have an allocated and initialized object, call the destructor
    (*ptr).~T();
  }
  // free the used memory
  pfree(handle);
  return;
}

