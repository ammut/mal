#ifndef _GC_H
#define _GC_H

#include "types.h"

#define INITIAL_GC_THRESHOLD 512

typedef struct sGcRoot {
	MalValue *root;
	size_t object_count;
	size_t object_count_max;
	size_t object_count_max_suspension;

	MalValue *repl_point;

#if 0
	pthread_t *gc_thread;
	pthread_cont_t *wakey_wakey;
	pthread_mutex_t *lock;
#endif

} GcRoot;

GcRoot new_gc;

void gc_run(GcRoot *gc_root);

void gc_set_root(MalValue *new, GcRoot*);

void gc_pause(GcRoot*);

void gc_resume(GcRoot*);

#endif
