#include <stdbool.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

struct pmalloc_struct {
  void *ptr;
  size_t handle;
};

typedef struct Block Block;

struct Block {
    void *addr;
    Block *next;
    size_t size;
};

typedef struct {
    Block *free;   // first free block
    Block *used;   // first used block
    Block *fresh;  // first available blank block
    size_t top;    // top free addr
    Block *blocks;
} Heap;

struct allocnv {
    Heap *heap;
    void *heap_limit;
    size_t heap_split_thresh;
    size_t heap_alignment;
    size_t heap_max_blocks;
};

void *pmalloc(size_t handler, size_t len);
void pfree(size_t handler);


void ta_init(struct allocnv *a, const void *base, const void *limit,
             const size_t heap_blocks, const size_t split_thresh,
             const size_t alignment);

void *ta_alloc(struct allocnv *a, size_t num);
void *ta_calloc(struct allocnv *a, size_t num, size_t size);
void  ta_free(struct allocnv *a, void *ptr);
int   ta_posix_memalign(struct allocnv *a, void **memptr, size_t align,
			size_t size);

size_t ta_num_free(struct allocnv *a);
size_t ta_num_used(struct allocnv *a);
size_t ta_num_fresh(struct allocnv *a);
bool   ta_check(struct allocnv *a);
#ifdef __cplusplus
} /* extern C*/
#endif
