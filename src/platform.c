#include "platform.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

// -- Mutex

struct sw_mutex {
	CRITICAL_SECTION cs;
};

sw_mutex *sw_mutex_alloc()
{
	sw_mutex *m = sw_alloc(sw_mutex);
	if (!m) return NULL;
	InitializeCriticalSection(&m->cs);
	return m;
}

void sw_mutex_free(sw_mutex *m)
{
	if (!m) return;
	DeleteCriticalSection(&m->cs);
	sw_free(m);
}

void sw_mutex_lock(sw_mutex *m)
{
	EnterCriticalSection(&m->cs);
}

void sw_mutex_unlock(sw_mutex *m)
{
	LeaveCriticalSection(&m->cs);
}
