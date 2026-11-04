#ifndef lrtemslib_h_
#define lrtemslib_h_

#include <stddef.h>

#include <rtems/score/heap.h>

/**
 * \brief Lua RTEMS allocator state.
 */
typedef struct luart_alloc_state {
  size_t heapSize;
  Heap_Control heap;
  void* heapptr;
} luart_alloc_state_t;

/**
 * \brief Initialize luart_alloc_state_t with sensible defaults.
 * \returns 0 on success.
 */
extern int luaRT_init_alloc_state(luart_alloc_state_t* state, size_t size);

extern void luaRT_free_alloc_state(luart_alloc_state_t* state);

/**
 * \brief Allocator handler for RTEMS. You may also provide your own!
 */
extern void* luaRT_alloc(void *ud, void *ptr, size_t osize, size_t nsize);

/**
 * \brief Returns the number of bytes used in the heap
 */
extern size_t luaRT_heap_usage(luart_alloc_state_t* state);

/**
 * \brief Returns the number of bytes free in the heap
 */
extern size_t luaRT_heap_free(luart_alloc_state_t* state);


#endif // lrtemslib_h_