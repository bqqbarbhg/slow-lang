#include "vm0.h"

sw_vm *sw_make_vm()
{
	sw_vm *vm = sw_alloc(sw_vm);
	vm->threads = NULL;
	vm->num_threads = 0;
	vm->cap_threads = 0;
	vm->halt = 0;
	vm->thread_mutex = sw_mutex_alloc();
	return vm;
}

void sw_free_vm(sw_vm *vm)
{
	// TODO: Halt threads as check
	sw_for(sw_thread*, p_thread, vm->threads, vm->num_threads) {
		sw_thread *thread = *p_thread;
		sw_mutex_free(thread->mutex);
		sw_free(thread);
	}
	sw_mutex_free(vm->thread_mutex);
	sw_free(vm->threads);
	sw_free(vm);
}

sw_thread *sw_get_thread(sw_vm *vm)
{
	sw_os_thread_id os_thread = sw_get_os_thread_id();
	sw_thread *thread = NULL;

	// Try to find an existing thread for the OS thread ID
	sw_mutex_lock(vm->thread_mutex);
	sw_for(sw_thread*, p_thread, vm->threads, vm->num_threads) {
		sw_thread *t = *p_thread;
		if (t->os_thread == os_thread) {
			thread = t;
			break;
		}
	}
	sw_mutex_unlock(vm->thread_mutex);

	if (!thread) {
		thread = sw_alloc(sw_thread);
		thread->vm = vm;
		thread->exec_mutex = sw_mutex_alloc();
		thread->halt_mutex = sw_mutex_alloc();

		// Add thread to the list of all threads
		sw_mutex_lock(vm->thread_mutex);
		sw_buf_grow(&vm->threads, &vm->cap_threads, ++vm->num_threads);
		vm->threads[vm->num_threads - 1] = thread;
		sw_mutex_unlock(vm->thread_mutex);
	}

	return thread;
}

void sw_free_thread(sw_thread *thread)
{
	sw_vm *vm = thread->vm;

	// Swap out the thread from the list
	sw_mutex_lock(vm->thread_mutex);
	size_t last_ix = --vm->num_threads;
	sw_for(sw_thread*, p_thread, vm->threads, last_ix) {
		if (*p_thread == thread) {
			*p_thread = vm->threads[last_ix];
			break;
		}
	}
	sw_mutex_unlock(vm->thread_mutex);

	// Free the pointer
	sw_mutex_free(thread->exec_mutex);
	sw_mutex_free(thread->halt_mutex);
	sw_free(thread);
}

void sw_exec(sw_thread *thread)
{
	sw_assert(thread->os_thread == sw_get_os_thread_id());

	sw_mutex_lock(thread->mutex);

	// TODO: Execute code

	sw_mutex_unlock(thread->mutex);
}

void sw_halt(sw_vm *vm, sw_thread *thread)
{
	sw_mutex_unlock(thread->mutex);

	// TODO: Help with garbage collection

	sw_mutex_lock(thread->mutex);
}

void sw_collect_garbage(sw_vm *vm)
{
	// Don't allow creating any more threads
	sw_mutex_lock(vm->thread_mutex);

	// Set halt flag to unlock thread mutexes
	vm->halt = 1;

	// Wait for all active threads to stop executing
	sw_for(sw_thread *, p_thread, vm->threads, vm->num_threads) {
		sw_thread *thread = *p_thread;
		sw_mutex_lock(thread->mutex);
	}

	// TODO: Collect garbage

	vm->halt = 0;

	sw_for(sw_thread *, p_thread, vm->threads, vm->num_threads) {
		sw_thread *thread = *p_thread;
		sw_mutex_unlock(thread->mutex);
	}

	sw_mutex_unlock(vm->thread_mutex);
}
