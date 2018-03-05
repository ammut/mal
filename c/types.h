#ifndef _TYPES_H
#define _TYPES_H

#include <inttypes.h>
#include <stdlib.h>

typedef union obj_s *obj;

// TYPE

enum malp_type {
	List,
	Int,
	Ratio,
	Real,
	Symbol,
	Nil,
	True,
	False,
	String,
	Fn,
	Env,
	Error,
	Keyword,
	Vector,
	HashMap,
	Atom,
	Forward,
};

typedef int malp_type;

// LIST

typedef struct malp_list {
	malp_type type;
	size_t count;
	obj first;
	obj rest;
} malp_list;

obj empty_list; // todo: make const

obj cons(obj list, obj element);

obj new_list();

/*
 * returns a pointer to the newly inserted obj.
 *
 * must be hooked into the gc
 */
obj append_mutating(obj list, obj element);

void set_length_mutating(obj list, size_t count);

#define LIST_FIRST(l) ((l)->list.first)
#define LIST_SECOND(l) ((l)->list.rest->list.first)

// NUMBER

#define REAL(obj) ((obj)->real.value)
#define INT(obj) ((obj)->int.value)

typedef int64_t malp_int_t;
typedef uint64_t malp_denom_t;
typedef double malp_real_t;

typedef struct malp_real {
	malp_type type;
	malp_real_t value;
} malp_real;

typedef struct malp_int {
	malp_type type;
	malp_int_t value;
} malp_int;

typedef struct malp_ratio {
	malp_type type;
	malp_int_t numerator;
	malp_denom_t denominator;
} malp_ratio;

typedef union {
	malp_real real;
	malp_int integer;
	malp_ratio ratio;
} malp_number;

obj new_int(malp_int_t);
obj new_real(malp_real_t);
obj new_ratio(malp_int_t, malp_denom_t);
obj read_number(char *token);

#define IS_ZERO(o) ( \
	(o)->type == Int ? (o)->integer.value == 0 : \
	(o)->type == Ratio ? (o)->ratio.numerator == 0 : \
	(o)->real.value == 0)

int is_number(obj o);

// SYMBOL

typedef struct malp_symbol {
	malp_type type;
	char name[];
} malp_symbol;

obj new_symbol(char *token, size_t length);

#define SYMBOL_IS(sym, str) (0 == strcmp(sym->symbol.name, str))

// NIL

typedef int malp_nil;

obj nil_o;

int is_nil(obj);

// TRUE

typedef int malp_true;

obj true_o;

int is_true(obj);

int is_truthy(obj);

// FALSE

typedef int malp_false;

obj false_o;

int is_false(obj);

int is_falsey(obj);

// STRING

typedef struct malp_string {
	malp_type type;
	size_t length;
	char value[];
} malp_string;

obj new_string(char *token, size_t len);

obj read_string(char *token, size_t len);

// FUNCTION

typedef obj (*malp_callable)(obj args, int *err);

typedef struct malp_fn {
	malp_type type;
	obj (*fn)(obj args, int *err);
} malp_fn;

// ENV

typedef struct malp_symtab {
	obj symbol;
	obj value;
} malp_symtab_elem;

typedef struct malp_env {
	malp_type type;
	unsigned data_size;
	malp_symtab_elem *data;
	obj outer;
#if 0
	pthread_t *gc_thread;
	pthread_cont_t *wakey_wakey;
	pthread_mutex_t *lock;
#endif
} malp_env;

obj new_env(unsigned size, obj outer);

// ERROR

typedef struct malp_error {
	malp_type type;
	int error_type;
	char *message;
} malp_error;

// OBJECT

typedef struct malp_fwd {
	malp_type type;
	obj *actual;
} malp_fwd;

typedef union obj_s {
	malp_type type;
	malp_list list;
	malp_int integer;
	malp_ratio ratio;
	malp_real real;
	malp_symbol symbol;
    malp_nil nil;
    malp_true true;
    malp_false false;
	malp_string string;
	malp_fn fn;
	malp_env env;
	malp_error error;
	malp_fwd forward;
} obj_s;

#endif
