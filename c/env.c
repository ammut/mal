#include <string.h>

#include "types.h"
#include "core.h"



void env_set(obj env, obj symbol, obj value)
{
	malp_symtab_elem *symtab = env->env.data;
	unsigned i = 0;
	while (i < env->env.data_size
		   && NULL != symtab[i].symbol
		   && !SYMBOL_IS(symtab[i].symbol, symbol->symbol.name))
		++i;
	if (i == env->env.data_size) {
		env->env.data = calloc(env->env.data_size * 2, sizeof(malp_symtab_elem));
		if (NULL == env->env.data) {
			env->env.data = symtab;
			return; // todo: crash horribly
		}
		env->env.data_size *= 2;
		for (unsigned j = 0; j < i; ++j) {
			env_set(env, symtab[j].symbol, symtab[j].value);
		}
		symtab = env->env.data;
	}
	symtab[i].symbol = symbol;
	symtab[i].value = value;
}

obj env_find(obj env, obj symbol)
{
	while (NULL != env) {
		malp_symtab_elem *symtab = env->env.data;
		for (unsigned i = 0; i < env->env.data_size && NULL != symtab[i].symbol; ++i) {
			if (0 == strcmp(symtab[i].symbol->symbol.name, symbol->symbol.name)) {
				return env;
			}
		}

		env = env->env.outer;
	}
	return env;
}

obj env_get(obj env, obj symbol)
{
	env = env_find(env, symbol);
	if (NULL == env) return env;
	malp_symtab_elem *symtab = env->env.data;
	for (unsigned i = 0; i < env->env.data_size; ++i) {
		if (0 == strcmp(symtab[i].symbol->symbol.name, symbol->symbol.name)) {
			return symtab[i].value;
		}
	}
	return NULL;
}

void env_bind_args(obj env, obj binds, obj exprs, obj *err)
{
	size_t passed = exprs->list.count;
	while (empty_list != binds) {
		if (OBJ_IS_SYMBOL(LIST_FIRST(binds), "&")) {
			if (LIST_SECOND(binds)) {
				env_set(
					env,
					LIST_SECOND(binds),
//					exprs == empty_list ? nil_o : exprs // should be nil, empty_list for mal compliance
					exprs
				);
			}
			return;
		}
		if (empty_list == exprs) {
			*err = new_arity_error(passed, "");
			return;
		}
		env_set(env, LIST_FIRST(binds), LIST_FIRST(exprs));

		binds = binds->list.rest;
		exprs = exprs->list.rest;
	}
}
