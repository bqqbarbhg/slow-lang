#include "platform.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

// -- Alloaction

void sw_buf_realloc(void **data, size_t *p_cap, size_t num, size_t size)
{
	uint32_t cap = *p_cap;
	cap = cap ? cap * 2 : 64 / (uint32_t)size;
	if (cap < num) cap = num;
	*p_cap = cap;
	*data = realloc(*data, cap * size);
}

// -- Mutex

struct sw_os_mutex {
	CRITICAL_SECTION cs;
};

sw_os_mutex *sw_os_mutex_alloc()
{
	sw_os_mutex *m = sw_alloc(sw_os_mutex);
	if (!m) return NULL;
	InitializeCriticalSection(&m->cs);
	return m;
}

void sw_os_mutex_free(sw_os_mutex *m)
{
	if (!m) return;
	DeleteCriticalSection(&m->cs);
	sw_free(m);
}

void sw_os_mutex_lock(sw_os_mutex *m)
{
	EnterCriticalSection(&m->cs);
}

void sw_os_mutex_unlock(sw_os_mutex *m)
{
	LeaveCriticalSection(&m->cs);
}

static char simple_lock[1];

void sw_mutex_lock(sw_mutex *m)
{
	void *prev = sw_atomic_swap_ptr(&m->state, simple_lock);
	if (prev == NULL) return;

	sw_os_mutex *m = sw_os_mutex_alloc();
	prev = sw_atomic_swap_ptr(&m->state, m);
	if (prev == NULL) {
	}

}

void sw_mutex_unlock(sw_mutex *m)
{
	uint32_t prev = sw_atomic_dec_32(&m->state);
	if (prev == 1) return;
}

// -- Threads

struct sw_os_thread {
	sw_os_thread_entry entry;
	void *user;
	HANDLE handle;
};

static DWORD WINAPI sw_win32_thread_proc(LPVOID arg)
{
	sw_os_thread *thread = (sw_os_thread*)arg;
	thread->entry(thread->user);
	return 0;
}

sw_os_thread *sw_start_os_thread(sw_os_thread_entry *entry, void *user)
{
	sw_os_thread *thread = sw_alloc(sw_os_thread);
	thread->entry = entry;
	thread->user = user;
	thread->handle = CreateThread(NULL, 0, &sw_win32_thread_proc, thread, 0, NULL);
	return thread;
}

void sw_join_os_thread(sw_os_thread *thread)
{
	WaitForSingleObject(thread->handle, INFINITE);
	CloseHandle(thread->handle);
	sw_free(thread);
}

void sw_detach_os_thread(sw_os_thread *thread)
{
	CloseHandle(thread->handle);
	sw_free(thread);
}

sw_os_thread_id sw_get_os_thread_id()
{
	return GetCurrentThreadId();
}
