#if 0
#include "vm.h"

sw_threadlocal sw_vm *sw_thread_vm;

// -- Garbage collection

struct sw_gc_slots {
	uint32_t num_shallow;
	uint32_t num_deep;
	uint32_t offsets[];
};

#define OBJ_LIST_SIZE 64
struct sw_obj_list {
	sw_object *objects[OBJ_LIST_SIZE];
	size_t num;
	sw_obj_list *next;
};

static sw_threadlocal sw_obj_list *t_gray_list;
static sw_threadlocal sw_obj_list *t_alloc_list;

static sw_obj_list *allocate_obj_list(sw_obj_pool *pool)
{
	sw_obj_list *list = NULL;

	// Try to acquire a gray list from the free queue
	sw_mutex_lock(pool->mutex);
	list = pool->free;
	if (list != NULL) {
		pool->free = list->next;
	}
	sw_mutex_unlock(pool->mutex);

	// Allocate a new one if none was found
	if (!list) {
		list = sw_alloc(sw_obj_list);
	}

	return list;
}

static void enqueue_obj_list(sw_obj_pool *pool, sw_obj_list *list)
{
	sw_mutex_lock(pool->mutex);
	list->next = pool->queue;
	pool->queue = list;
	sw_mutex_unlock(pool->mutex);
}

static sw_obj_list *dequeue_obj_list(sw_obj_pool *pool)
{
	sw_mutex_lock(&pool->mutex);
	sw_obj_list *list = pool->queue;
	if (list) {
		pool->queue = list->next;
	}
	sw_mutex_unlock(&pool->mutex);
	return list;
}

static sw_obj_list *free_obj_list(sw_obj_pool *pool, sw_obj_list *list)
{
	list->num = 0;
	sw_mutex_lock(&pool->mutex);
	list->next = pool->free;
	pool->free = list;
	sw_mutex_unlock(&pool->mutex);
}

sw_object *sw_alloc_object()
{
	sw_object *obj = sw_alloc(sw_object);

	sw_vm *vm = sw_thread_vm;
	sw_obj_list *list = t_alloc_list;

	// Get an allocated object list from the VM if necessary
	if (!list) {
		list = allocate_obj_list(&vm->alloc_pool);
		t_alloc_list = list;
	}

	size_t index = list->num++;
	list->objects[index] = obj;

	// Return the gray list to the other threads if it's full
	if (index + 1 == OBJ_LIST_SIZE) {
		enqueue_obj_list(&vm->alloc_pool, list);
		t_alloc_list = NULL;
	}
}

void sw_mark_gray(const sw_object *c_obj)
{
	sw_object *obj = (sw_object*)c_obj;
	obj->gray = 1;

	// Early return if the object is shallow
	if (!obj->deep) {
		obj->black = 1;
		return;
	}

	sw_vm *vm = sw_thread_vm;

	// Queue deep objects to a gray list
	sw_obj_list *list = t_gray_list;

	// Get a gray list if the thread doesn't currently have one
	if (!list) {
		list = allocate_obj_list(&vm->gray_pool);
		t_gray_list = list;
	}

	size_t index = list->num++;
	list->objects[index] = obj;

	// Return the gray list to the other threads if it's full
	if (index + 1 == OBJ_LIST_SIZE) {
		enqueue_obj_list(&vm->gray_pool, list);
		t_gray_list = NULL;
	}
}

static void mark_object_slots(sw_object *obj)
{
	const sw_gc_slots *slots = obj->type->gc_slots;
	const uint32_t *so = slots->offsets;

	// Shallow objects can be marked directly to black
	for (const uint32_t *end = so + slots->num_shallow; so != end; so++) {
		sw_object *slot_obj = ((sw_object**)obj)[*so];
		slot_obj->gray = 1;
		slot_obj->black = 1;
	}

	// Queue deep objects for further marking
	for (const uint32_t *end = so + slots->num_deep; so != end; so++) {
		sw_object *slot_obj = ((sw_object**)obj)[*so];
		if (!slot_obj->gray && !slot_obj->black) {
			sw_mark_gray(slot_obj);
		}
	}
}

static int process_gray_list(sw_obj_list *list)
{
	sw_for(sw_object*, p_obj, list->objects, list->num) {
		sw_object *obj = *p_obj;
		mark_object_slots(obj);
	}
}

static int process_next_gray_list(sw_vm *vm)
{
	// Pop a gray list from the VM queue
	sw_obj_list *list = dequeue_obj_list(&vm->gray_pool);
	if (list == NULL) return 0;

	// Process the contained objects
	process_gray_list(list);

	// Return the list for re-use
	free_obj_list(&vm->gray_pool, list);

	return 1;
}

static void sweep_garbage(sw_vm *vm)
{
	vm->prev_alloc_pool.queue = vm->alloc_pool.queue;
	vm->alloc_pool.queue = NULL;

	for (;;) {
		sw_obj_list *src = dequeue_obj_list(&vm->prev_alloc_pool);
		if (!src) break;
		sw_obj_list *dst = allocate_obj_list(&vm->alloc_pool);
		sw_for(sw_object*, p_obj, src->objects, src->num) {
			sw_object *obj = *p_obj;
			if (!obj->gray && !obj->black) {
				sw_free(obj);
				continue;
			}
			obj->gray = 0;
			obj->black = 0;

			size_t index = dst->num++;
			dst->objects[index] = obj;
			if (index + 1 == OBJ_LIST_SIZE) {
				enqueue_obj_list(&vm->alloc_pool, dst);
				dst = allocate_obj_list(&vm->alloc_pool);
			}
			free_obj_list(&vm->alloc_pool, src);
		}
		enqueue_obj_list(&vm->alloc_pool, dst);
	}
}

void sw_finish_gc()
{
	sw_vm *vm = sw_thread_vm;

	// Queue alloc list
	if (t_alloc_list) {
		enqueue_obj_list(&vm->alloc_pool, t_alloc_list);
	}

	// TODO: This is super slow for long linked lists...
	do {

		// Return thread-local gray list
		sw_object *list = t_gray_list;
		if (list) {
			enqueue_obj_list(&vm->gray_pool, list);
			t_gray_list = NULL;
		}

	} while (process_gray_list(vm));

	return 1;
}
#endif
