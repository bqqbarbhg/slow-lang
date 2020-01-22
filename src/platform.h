#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <intrin.h>

// -- Language

#define sw_arraycount(arr) (sizeof(arr) / sizeof(*(arr)))

#define sw_forceinline __forceinline

#define sw_for(type, name, data, num) for (type *name = (data), *name##_end = name + (num); name != name##_end; name++)

#define sw_assert(cond) do { if (!(cond)) __debugbreak(); } while (0)

// -- Atomics

#define sw_atomic_inc_32(dst) (uint32_t)_InterlockedIncrement((volatile long*)(dst))
#define sw_atomic_dec_32(dst) (uint32_t)_InterlockedDecrement((volatile long*)(dst))
#define sw_atomic_cas_32(dst, cmp, value) (uint32_t)_InterlockedCompareExchange(((volatile long*)dst), (long)(value), (long)(cmp))
#define sw_atomic_swap_32(dst, value) (uint32_t)_InterlockedExchange((volatile long*)(dst), (long)(value))

#define sw_atomic_swap_ptr(dst, value) _InterlockedExchangePointer((void*volatile*)(dst), (void*)(value))
#define sw_atomic_cas_ptr(dst, cmp, value) _InterlockedCompareExchangePointer(((void*volatile*)dst), (void*)(value), (void*)(cmp))

// -- Allocation

#define sw_alloc(type) (type*)malloc(sizeof(type))
#define sw_alloc_n(type) (type*)malloc(sizeof(type) * (n))
#define sw_free(ptr) free(ptr)

void sw_buf_realloc(void **data, size_t *p_cap, size_t num, size_t size);

static sw_forceinline void
sw_buf_grow_size(void **p_data, size_t *p_cap, size_t num, size_t size)
{
	if (num <= *p_cap) return;
	buf_realloc(p_data, p_cap, num, size);
}

#define sw_buf_grow(p_buf, p_cap, num) sw_buf_grow_size((void**)(p_buf), (p_cap), (num), sizeof(**(p_buf)))

// -- Mutex

typedef struct sw_os_mutex sw_os_mutex;
typedef struct sw_mutex {
	void *state;
} sw_mutex;

void sw_mutex_lock(sw_mutex *m);
void sw_mutex_unlock(sw_mutex *m);

// -- Threads

typedef void (*sw_os_thread_entry)(void *);
typedef struct sw_os_thread sw_os_thread;
typedef uint32_t sw_os_thread_id;

sw_os_thread *sw_start_os_thread(sw_os_thread_entry *entry, void *user);
void sw_join_os_thread(sw_os_thread *thread);
void sw_detach_os_thread(sw_os_thread *thread);
sw_os_thread_id sw_get_os_thread_id();
