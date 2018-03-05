#include <assert.h>
#include <pthread.h>
#include <stdio.h>

#include "gc.h"
#include "types.h"

GcRoot new_gc = {
	.root = NULL,
	.object_count = 0,
	.object_count_max = INITIAL_GC_THRESHOLD
};

void gc_set_root(MalValue *v, GcRoot *gc) {
	if(gc->object_count > gc->object_count_max) gc_run(gc);

	v->gc_next = gc->root;
	v->gc_mark = 0;
	gc->root = v;
	++gc->object_count;
}

static void markSingle(MalValue *object) {
	if (NULL == object || object->gc_mark) return;
	switch (object->type) {
	case List: {
		do {
			object->gc_mark = 1;
			markSingle(((malp_list*)object->value)->first);
		} while ((object = ((malp_list*)object->value)->rest));
		break;
	}
	case Number:
	case Symbol:
	case String:
		object->gc_mark = 1;
		break;
	case Nil:
	case True:
	case False:
		break;
	default:
		assert(!"not implemented yet");
		break;
	}
}

static void mark(GcRoot *gc) {
	// go through env table, start marking...
	markSingle(gc->repl_point);
}

static void sweep(GcRoot *gc) {
	MalValue **object = &gc->root;
	while (*object) {
		if ((*object)->gc_mark) {
			(*object)->gc_mark = 0;
			object = &(*object)->gc_next;
		} else {
			MalValue *obsolete = *object;
			*object = obsolete->gc_next;
			destroy_mal_value(obsolete);
			--gc->object_count;
		}
	}
}

void gc_run(GcRoot *gc) {
	puts("collecting trash...");
	mark(gc); // feed the env in here
	sweep(gc);

	gc->object_count_max = 2 * gc->object_count;
}

void gc_pause(GcRoot *gc) {
	if (!gc->object_count_max_suspension) {
		gc->object_count_max_suspension = gc->object_count_max;
		gc->object_count_max = SIZE_MAX;
	}
}

void gc_resume(GcRoot *gc) {
	if (gc->object_count_max_suspension) {
		gc->object_count_max = gc->object_count_max_suspension;
		gc->object_count_max_suspension = 0;
	}
}

#if 0
static void gc_run_async(void *env) {
	pthread_mutex_t wakey_mutex;

	pthread_mutex_lock(&wakey_mutex);
	while (0) ;
	MalValue *start = env->root;
	mark(start);

	pthread_mutex_lock(env->lock);
	mark(start);

	sweep(&env->root);

	pthread_mutex_unlock(env->lock);
}


void gc_async_init(Env *env) {
	pthread_create(env->gc_thread, NULL, gc_run, env);

	while (env);
}

void gc_wake(Env *env) {}


void gc_join() {}
#endif
