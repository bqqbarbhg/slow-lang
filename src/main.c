#include "vm0.h"

sw_vm *g_vm;

#define NUM_THREADS 4

void thread_func(void *user)
{
	sw_thread *thread = sw_get_thread(g_vm);

	sw_exec(thread);

	sw_free_thread(thread);
}

int main(int argc, char **argv)
{
	g_vm = sw_make_vm();
	sw_os_thread *os_threads[NUM_THREADS];
	sw_for(sw_os_thread*, p_thread, os_threads, NUM_THREADS) {
		*p_thread = sw_start_os_thread(&thread_func, NULL);
	}

	sw_thread *thread = sw_get_thread(g_vm);

	sw_exec(thread);
	sw_collect_garbage(g_vm);

	sw_for(sw_os_thread*, p_thread, os_threads, NUM_THREADS) {
		sw_join_os_thread(*p_thread);
	}

	return 0;
}
