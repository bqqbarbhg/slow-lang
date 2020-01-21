#pragma once

#include "platform.h"

typedef struct sw_vm sw_vm;
typedef struct sw_thread sw_thread;

struct sw_thread {
	sw_vm *vm;
	sw_mutex *mutex;
};

struct sw_vm {
	int halt;

	sw_mutex *thread_mutex;
	sw_thread **threads;
	size_t num_threads, cap_threads;
};

sw_vm *sw_make_vm();

sw_thread *sw_make_thread(sw_vm *vm);
void sw_free_thread(sw_thread *thread);

void sw_exec(sw_thread *thread);

void sw_halt(sw_vm *vm, sw_thread *thread);

static sw_forceinline void
sw_checkpoint(sw_vm *vm, sw_thread *thread)
{
	if (vm->halt) sw_halt(vm, t);
}

void sw_collect_garbage(sw_vm *vm);
