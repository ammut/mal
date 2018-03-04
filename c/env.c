#include <string.h>

#include "types.h"
#include "core.h"

obj get_symbol(obj env, char *name)
{
	malp_symtab *symtab = env->env.symtab;
	for (unsigned i = 0; symtab[i].name; ++i) {
		if (strcmp(symtab[i].name, name) == 0) {
			return symtab[i].value;
		}
	}
	return NULL;
}
