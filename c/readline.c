/**
 *
 *
 * mostly after https://robots.thoughtbot.com/tab-completion-in-gnu-readline
 */

#include <readline/readline.h>
#include <readline/history.h>

#include "readline.h"

static obj completion_env;

static char *next_symbol_like(const char *text, int state)
{
	static size_t i, len;
	obj symbol;

	if (!state) {
		i = 0;
		len = strlen(text);
	}

	malp_symtab_elem *symtab = completion_env->env.data;

	while ((symbol = symtab[i++].symbol) != NULL) {
		if (strncmp(symbol->symbol.name, text, len) == 0) {
			return strdup(symbol->symbol.name);
		}
	}

	return NULL;
}


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
static char **do_autocomplete(const char *text, int start, int end)
{
	rl_attempted_completion_over = 1;
	return rl_completion_matches(text, next_symbol_like);
}
#pragma GCC diagnostic pop

char *next_line()
{
	char *in = readline("user> ");
	if (in && *in) {
		add_history(in);
	}
	return in;
}

void init_readline(obj env) {
	completion_env = env;
	rl_initialize();
	rl_attempted_completion_function = do_autocomplete;
}

