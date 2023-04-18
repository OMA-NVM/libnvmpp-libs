/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Hugo Lefeuvre <hugo.lefeuvre@neclab.eu>
 *
 * Copyright (c) 2020, NEC Europe Ltd., NEC Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <uk/allocnv.h>
#include <uk/alloc_impl.h>
#include <uk/arch/limits.h> /* for __PAGE_SIZE */
#include <string.h> /* for memset */
#include <allocnv.h>
#include <stdlib.h>

struct pmalloc_struct pmsl[5] __attribute__((persistent)) = {0};
struct uk_alloc *a __attribute__((persistent)) = (void*) 1337;
struct allocnv *b __attribute__((persistent)) = (void*) 1337;
size_t metalen __attribute__((persistent)) = 1337;

static void *uk_allocnv_malloc(struct uk_alloc *a, size_t size)
{
	struct allocnv *b;

	b = (struct allocnv *)&a->priv;
	return ta_alloc(b, size);
}

static void uk_allocnv_free(struct uk_alloc *a, void *ptr)
{
	struct allocnv *b;

	b = (struct allocnv *)&a->priv;
	ta_free(b, ptr);
}

void pmalloc_print_stats() {
  for(size_t i = 0; i < sizeof(pmsl) / sizeof(pmsl[0]); i++) {
    uk_pr_info("pmsl entry %d: ptr: %p, handle: %d\n", i, pmsl[i].ptr, pmsl[i].handle);
  }

}

void *pmalloc(size_t handle, size_t len) {
  // Walk through the whole array to check whether there is an entry with that handle
  for(size_t i = 0; i < sizeof(pmsl) / sizeof(pmsl[0]); i++) {
    if(pmsl[i].handle == handle) {
  pmalloc_print_stats();
      return pmsl[i].ptr;
    }
  }
  // We went through the whole array but did not find the handle, search an empty slot (handle = 0 and ptr = 0)
  for(size_t i = 0; i < sizeof(pmsl) / sizeof(pmsl[0]); i++) {
    if(pmsl[i].handle == 0 && pmsl[i].ptr == (void*) 0) {
      pmsl[i].handle = handle;
      pmsl[i].ptr = malloc(len); // This could be 0
      // Zero out the memory, so that the constructor can check for that.
      memset(pmsl[i].ptr, 0, len);
  pmalloc_print_stats();
      return pmsl[i].ptr;
    }
  }
  // The handle was not known yet nor was it possible to allocate a new slot for it, return 0
  pmalloc_print_stats();
  return 0;
}

void pfree(size_t handle) {
  for(size_t i = 0; i < sizeof(pmsl) / sizeof(pmsl[0]); i++) {
    if(pmsl[i].handle == handle) {
      pmsl[i].handle = 0;
      free(pmsl[i].ptr);
      pmsl[i].ptr = 0;
    }
  }
  pmalloc_print_stats();
    return;
}

struct uk_alloc *uk_allocnv_init(void *base, size_t len)
{
        if( a == (void*) 1337 || b == (void*) 1337 || metalen == 1337 ) { /* check all three values */
          if (len <= __PAGE_SIZE) return NULL;

          /* Allocate space for allocator descriptor */
          metalen = sizeof(*a) + sizeof(*b);

          /* enough space for allocator available? */
          if (metalen > len) {
                  uk_pr_err("Not enough space for allocator: %"__PRIsz
                            " B required but only %"__PRIuptr" B usable\n",
                            metalen, len);
                  return NULL;
          }

          /* store allocator metadata on the heap, just before the memory pool */
          a = (struct uk_alloc *)base;
          b = (struct allocnv *)&a->priv;
          memset(a, 0, metalen);

          uk_pr_info("Initialize allocnv allocator @ 0x%" __PRIuptr ", len %"
                          __PRIsz"\n", (uintptr_t)a, len);

          ta_init(b, base + metalen, base + len, CONFIG_LIBALLOCNV_HEAP_BLOCKS,
                  CONFIG_LIBALLOCNV_SPLIT_THRESH,
                  CONFIG_LIBALLOCNV_ALIGNMENT);

        } else {
	  uk_pr_info("Skipped allocnv inittialization code, running self check...\n");
          if(ta_check(b)) {
            uk_pr_info("self check successful\n");
          } else {
            uk_pr_info("self check failed\n");
          }
        }
        uk_pr_info("Registering malloc-calls\n");
        uk_alloc_init_malloc_ifmalloc(a, uk_allocnv_malloc, uk_allocnv_free, NULL /* addmem */);
        pmalloc_print_stats();

	return a;
}

