#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

// -- Language

#define sw_threadlocal __declspec(thread)
#define sw_forceinline __forceinline

#define sw_for(type, name, data, num) for (type *name = (data), *name##_end = name + (num); name != name##_end; name++)

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

typedef struct sw_mutex sw_mutex;

sw_mutex *sw_mutex_alloc();
void sw_mutex_free(sw_mutex *m);
void sw_mutex_lock(sw_mutex *m);
void sw_mutex_unlock(sw_mutex *m);

// -- Threads

typedef void (*sw_os_thread_entry)(void *);
typedef struct sw_os_thread sw_os_thread;

sw_os_thread *sw_start_os_thread(sw_os_thread_entry *entry, void *user);
void sw_join_os_thread(sw_os_thread *thread);
void sw_detach_os_thread(sw_os_thread *thread);
