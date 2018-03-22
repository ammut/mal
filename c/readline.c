/**
 *
 *
 * mostly after https://robots.thoughtbot.com/tab-completion-in-gnu-readline
 */

#include <readline/readline.h>
#include <readline/history.h>

#include "readline.h"

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

static obj completion_env;

static char *next_symbol_like(const char *text, int state)
{
	static size_t i, j, len;
	char *singleton;
	static char *singletons[] = {
		"nil",
		"false",
		"true",
		"def!",
		"fn*",
		"let*",
		"do",
		"if",
		"defmacro!",
		"macroexpand",
	};

	if (!state) {
		i = j = 0;
		len = strlen(text);
	}

	malp_symtab_elem *symtab = completion_env->env.data;

	obj symbol;
	while ((symbol = symtab[i++].symbol) != NULL) {
		if (strncmp(symbol->symbol.name, text, len) == 0) {
			return strdup(symbol->symbol.name);
		}
	}

	while (j < ARRAY_SIZE(singletons)) {
		singleton = singletons[j++];
		if (strncmp(singleton, text, len) == 0) {
			return strdup(singleton);
		}
	}

	return NULL;
}


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
static char **do_autocomplete(const char *text, int start, int end)
{
	if (rl_completion_quote_character) {
		return NULL;
	}
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
	rl_completer_word_break_characters = "(){}[]' ";
	rl_completer_quote_characters = "\"";
}

