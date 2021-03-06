#ifndef _CORE_H
#define _CORE_H

#include <stdlib.h>
#include <stdint.h>
#include "types.h"

#define DEF_BUILTIN_FN(name) \
static obj name##_(obj, obj*); \
obj name = (obj)&(malp_builtin_fn){ \
	.type = BuiltinFn, \
	.fn = name##_, \
}; \
static obj name##_

void core_load_vars(obj env);

obj cons_sym;

obj concat_sym;

obj quote_sym;

obj STAR1_sym;

obj STAR2_sym;

obj STAR3_sym;

obj malp_core_numberQUESTION;

obj malp_core_PLUS;

obj malp_core_MINUS;

obj malp_core_STAR;

obj malp_core_SLASH;

obj malp_core_prMINUSstr;

obj malp_core_str;

obj malp_core_prn;

obj malp_core_print;

obj malp_core_println;

obj malp_core_list;

obj malp_core_listQUESTION;

obj malp_core_emptyQUESTION;

obj malp_core_count;

obj malp_core_EQUAL;

obj malp_core_LT;

obj malp_core_LE;

obj malp_core_GT;

obj malp_core_GE;

obj malp_core_readMINUSstring;

obj malp_core_slurp;

obj malp_core_atom;

obj malp_core_atomQUESTION;

obj malp_core_deref;

obj malp_core_resetIMPURE;

obj malp_core_swapIMPURE;

obj malp_core_first;

obj malp_core_rest;

obj malp_core_cons;

obj malp_core_concat;

obj malp_core_nth;

obj malp_core_apply;

obj malp_core_map;

obj malp_core_nilQUESTION;

obj malp_core_trueQUESTION;

obj malp_core_falseQUESTION;

obj malp_core_symbolQUESTION;

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
