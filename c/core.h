#ifndef _CORE_H
#define _CORE_H

#include <stdlib.h>
#include <stdint.h>
#include "types.h"

#define DEF_BUILTIN_FN(name) \
static obj name##_(obj, int*); \
obj name = (obj)&(malp_fn){ \
	.type = Fn, \
	.fn = name##_, \
}; \
static obj name##_

malp_symtab *builtins;

void init_builtins();

obj numberQUESTION;

obj PLUS;

obj MINUS;

obj STAR;

obj SLASH;

#if 0

int32_t hash_string(malp_string str, size_t len) {
	int32_t h = 0;

	size_t loops = len / 4;
	for (int i = 0; loops - i > 0; ++i) {
		int32_t ;
	}
}

int32_t hash_number(malp_number *num) {
	int32_t l = num->value->INT & 0xffffffff,
		u = num->value->INT >> (8 * sizeof num->value->INT);
	return u ^ l;
}

int32_t hash_symbol(malp_symbol sym) {
	return hash_string(sym);
}

#endif

#endif
