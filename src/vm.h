#if 0
#pragma once

#include "platform.h"

typedef struct sw_vm sw_vm;
typedef struct sw_type sw_type;
typedef struct sw_object sw_object;
typedef struct sw_obj_list sw_obj_list;
typedef struct sw_obj_pool sw_obj_pool;
typedef struct sw_gc_slots sw_gc_slots;

extern sw_threadlocal sw_vm *sw_thread_vm;

// -- VM

struct sw_obj_pool {
	sw_mutex *mutex;
	sw_obj_list *queue;
	sw_obj_list *free;
};

struct sw_vm {
	sw_obj_pool gray_pool;
	sw_obj_pool alloc_pool;
	sw_obj_pool prev_alloc_pool;
};

// -- Type

struct sw_type {
	size_t size;
	const sw_gc_slots *gc_slots;
};

// -- Object

struct sw_object {
	const sw_type *type;
	char gray, black, deep;
};

sw_object *sw_alloc_object();

void sw_mark_gray(const sw_object *obj);

static sw_forceinline void
sw_write_slot(sw_object *parent, sw_object **slot, const sw_object *c_value)
{
	sw_object *value = (sw_object*)c_value;
	*slot = value;
	if (value && parent->black && !value->gray) {
		sw_mark_gray(value);
	}
}

void sw_finish_gc();
#endif
