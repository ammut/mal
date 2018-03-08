#ifndef _TYPES_H
#define _TYPES_H

#include <inttypes.h>
#include <stdlib.h>

#define STRLEN_STATIC(s) (sizeof(s) - 1)

typedef union obj_s *obj;

// TYPE

enum malp_type {
	List,
	Int,
	Ratio,
	Real,
	Symbol,
	Keyword,
	Nil,
	True,
	False,
	String,
	BuiltinFn,
	Fn,
	Env,
	Error,
	Vector,
	HashMap,
	Atom,
	Forward,
};

typedef int malp_type;

#define IS_COLLECTION(l) ((l)->type == List || (l)->type == Vector)
#define COUNT(l) ((l)->list.count)

// LIST

typedef struct malp_list {
	malp_type type;
	size_t count;
	obj first;
	obj rest;
} malp_list;

const obj const empty_list;

obj empty_list_str;

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

#define MAP_FROM_TO(f, t) \
do { \
	(t) = new_list(); \
	(t)->list.count = (f)->list.count; \
	(t)->list.first = EVAL(ast->list.first, env, err); \
	obj el = res; \
	while ((ast = ast->list.rest) != empty_list) { \
	el->list.rest = new_list(); \
	el = el->list.rest; \
	el->list.count = ast->list.count; \
	el->list.first = EVAL(ast->list.first, env, err); \
	} \
	el->list.rest = empty_list; \
}

// NUMBER

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

int is_number(obj o);

#define NUMBER_VALUE(n) (\
	(n)->type == Int ? (n)->integer.value : \
	(n)->type == Ratio ? (n)->ratio.numerator / (malp_real_t) (n)->ratio.denominator : \
	(n)->real.value)

#define IS_ZERO(o) (NUMBER_VALUE(o) == 0)

// SYMBOL

typedef struct malp_symbol {
	malp_type type;
	char name[];
} malp_symbol;

obj new_symbol(char *token, size_t length);

#define SYMBOL_IS(sym, str) (0 == strcmp(sym->symbol.name, str))

#define OBJ_IS_SYMBOL(o, str) ((o)->type == Symbol && SYMBOL_IS(o, str))

// KEYWORD

typedef struct malp_symbol malp_keyword;

obj new_keyword(char *token, size_t length);

// NIL

typedef int malp_nil;

obj nil_o;

obj nil_str;

int is_nil(obj);

// TRUE

typedef int malp_true;

obj true_o;

obj true_str;

int is_true(obj);

int is_truthy(obj);

// FALSE

typedef int malp_false;

obj false_o;

obj false_str;

int is_false(obj);

int is_falsey(obj);

// STRING

typedef struct malp_string {
	malp_type type;
	size_t length;
	char value[];
} malp_string;

obj new_empty_string(size_t len);

obj new_string(char *token, size_t len);

obj read_string(char *token, size_t len);

obj empty_string;

// VECTOR

typedef struct malp_vector {
	malp_type type;
	size_t count;
	uint32_t bitmask;
	obj data[32];
} malp_vector;

// FUNCTION

typedef obj (*malp_callable)(obj args, int *err);

typedef struct malp_builtin_fn {
	malp_type type;
	obj (*fn)(obj args, int *err);
} malp_builtin_fn;

typedef struct malp_fn {
	malp_type type;
	obj ast;
	obj env;
	obj binds;
} malp_fn;

obj new_fn(obj ast, obj env, obj binds);

obj fn_str;

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
	malp_keyword keyword;
    malp_nil nil;
    malp_true true;
    malp_false false;
	malp_string string;
	malp_builtin_fn builtin_fn;
	malp_fn fn;
	malp_env env;
	malp_error error;
	malp_fwd forward;
} obj_s;

#endif
