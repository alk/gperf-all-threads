#define _GNU_SOURCE
#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <gperftools/profiler.h>

typedef int (*pthread_create_type)(pthread_t *restrict, const pthread_attr_t *restrict, void *(*)(void *), void *restrict);

static pthread_create_type original_pthread_create;

static __attribute__((constructor))
void __constructor()
{
	original_pthread_create = (pthread_create_type)(intptr_t)dlsym(RTLD_NEXT, "pthread_create");
	if (!original_pthread_create) {
		fprintf(stderr, "failed to locate original pthread_create: %s\n", dlerror());
		abort();
	}

	char *cpuprofile_path = getenv("CPUPROFILE");
	if (!cpuprofile_path || !*cpuprofile_path)
		fprintf(stderr, "warning: you're LD_PRELOAD-ing gperftools profiler, but not giving CPUPROFILE environment variable!\n");
}

struct trampoline_data {
	void *(*original_fn)(void *);
	void *original_arg;
};

static
void *trampoline(void *_arg)
{
	struct trampoline_data data = *(struct trampoline_data *)_arg;
	free(_arg);
	fprintf(stderr, "Activating profiler for newly created thread %p\n", (void *)pthread_self());
	ProfilerRegisterThread();
	return data.original_fn(data.original_arg);
}

int pthread_create(pthread_t *restrict thread, const pthread_attr_t *restrict attr, void *(*fn)(void *), void *restrict arg)
{
	struct trampoline_data *data;

	data = malloc(sizeof(struct trampoline_data));
	if (!data) {
		return ENOMEM;
	}
	data->original_fn = fn;
	data->original_arg = arg;

	int rv = original_pthread_create(thread, attr, trampoline, data);
	if (rv)
		free(data);
	return rv;
}
