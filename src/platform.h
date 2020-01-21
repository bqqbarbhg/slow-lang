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

// -- Mutex

typedef struct sw_mutex sw_mutex;
sw_mutex *sw_mutex_alloc();
void sw_mutex_free(sw_mutex *m);
void sw_mutex_lock(sw_mutex *m);
void sw_mutex_unlock(sw_mutex *m);
