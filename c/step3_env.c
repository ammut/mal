#include <stdio.h>
#include <stdlib.h>

#include "readline.h"

#include "types.h"
#include "reader.h"
#include "printer.h"
#include "env.h"
#include "core.h"


obj READ(char *in) {
	obj r = read_str(in);
	return r;
}

obj EVAL(obj ast, obj env, int *err);

obj eval_ast(obj ast, obj env, int *err)
{
	switch (ast->type) {
		case Symbol: {
			obj value = env_get(env, ast);
			if (NULL == value) *err = SymbolNotFoundError; // todo
			return value;
		}
		case List: {
			obj res = new_list();
			res->list.count = ast->list.count;
			res->list.first = EVAL(ast->list.first, env, err);

			obj el = res;
			while ((ast = ast->list.rest) != empty_list) {
				el->list.rest = new_list();
				el = el->list.rest;
				el->list.count = ast->list.count;
				el->list.first = EVAL(ast->list.first, env, err);
			}
			el->list.rest = empty_list;

			return res;
		}
		default:
			return ast;
	}
}

obj EVAL(obj ast, obj env, int *err) {
	if (ast == NULL) return ast; // todo remove once reader is complete
	if (ast == empty_list) {
		return ast;
	} else if (ast->type == List) {
		obj first = ast->list.first;

		if (first->type == Symbol && SYMBOL_IS(first, "def!")) {
			if (ast->list.count != 3) {
				*err = ArityError; // todo
				return NULL;
			}
			obj symbol = LIST_FIRST(ast->list.rest);
			if (symbol->type != Symbol) {
				*err = InvalidArgumentError; // todo
				return NULL;
			}
			obj value = EVAL(LIST_SECOND(ast->list.rest), env, err);
			if (*err) return NULL;
			env_set(env, symbol, value);
			return value;

		} else if (first->type == Symbol && SYMBOL_IS(first, "let*")) {
			if (ast->list.count != 3) {
				*err = ArityError; // todo
				return NULL;
			}
			obj bindings = LIST_FIRST(ast->list.rest);
			obj result = LIST_SECOND(ast->list.rest);
			if (List != bindings->type) {
				*err = InvalidArgumentError; // todo
				return NULL;
			}
			if (bindings->list.count % 2 != 0) {
				*err = ArityError; // todo
				return NULL;
			}
			obj let_env = new_env((unsigned)bindings->list.count / 2, env);
			while (empty_list != bindings) {
				if (LIST_FIRST(bindings)->type != Symbol) {
					*err = InvalidArgumentError; // todo
					return NULL;
				}
				env_set(let_env, LIST_FIRST(bindings), EVAL(LIST_SECOND(bindings), let_env, err));
				if (*err) return NULL;

				bindings = bindings->list.rest->list.rest;
			}
			return EVAL(result, let_env, err);


		} else {
			ast = eval_ast(ast, env, err);
			if (*err) return NULL;
			first = ast->list.first;
			obj rest = ast->list.rest;
			if (first->type != BuiltinFn) {
				*err = NotCallableError; // todo
				return NULL;
			}
			return first->fn.fn(rest, err);
		}

	} else {
		return eval_ast(ast, env, err);
	}
}

void PRINT(obj ast) {
	print_str(ast);
}

void rep(char *in, obj env, int *err) {
	PRINT(EVAL(READ(in), env, err));
}

int main() {
	char *line;

	obj repl_env = init_repl_env();
	int err = 0;

	init_readline(repl_env);

	while(1) {
		line = next_line();
		if (!line) {
			fputc('\n', stdout);
			return 0;
		}
		if (*line) {
			rep(line, repl_env, &err);
		} else {
			free(line);
		}

		if (err) {
			char *msg;
			switch (err) {
				case ArityError:
					msg = "Wrong number of args passed\n";
					break;
				case InvalidArgumentError:
					msg = "Invalid argument\n";
					break;
				case ReaderError:
					msg = "Could not parse\n";
					break;
				case SymbolNotFoundError:
					msg = "Unable to resolve symbol\n";
					break;
				case NotCallableError:
					msg = "Object is not callable\n";
					break;
				default:
					msg = "something bad happened!\n";
					break;
			}
			fputs("Error: ", stderr);
			fputs(msg, stderr);
			err = 0;
		}
	}
}
