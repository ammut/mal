#include <stdio.h>
#include <stdlib.h>


#include "types.h"
#include "reader.h"
#include "printer.h"
#include "env.h"
#include "readline.h"
#include "core.h"

obj READ(char *in) {
	obj r = read_str(in);
	return r;
}

obj EVAL(obj ast, obj env, int *err);

obj invoke_fn(obj fn, obj args, int *err)
{
	if (fn->type == BuiltinFn) {
		return fn->builtin_fn.fn(args, err);
	} else if (fn->type == Fn) {
		obj fn_env = new_env((unsigned) fn->fn.binds->list.count, fn->fn.env);
		env_bind_args(fn_env, fn->fn.binds, args, err);
		if (*err) return NULL;
		return EVAL(fn->fn.ast, fn_env, err);
	} else if (fn->type == Keyword) {
		return NULL; // todo
	} else if (fn->type == HashMap) {
		return NULL; // todo
	} else {
		*err = NotCallableError;
		return NULL;
	}
}

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
				el->list.rest = empty_list;
			}

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

		if (OBJ_IS_SYMBOL(first, "def!")) {
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

		} else if (OBJ_IS_SYMBOL(first, "let*")) {
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

		} else if (OBJ_IS_SYMBOL(first, "fn*")){
			return new_fn(
				LIST_SECOND(ast->list.rest),
				env,
				LIST_FIRST(ast->list.rest)
			); // todo: (every? symbol? binds)

		} else if (OBJ_IS_SYMBOL(first, "if")){
			if (ast->list.count != 4 && ast->list.count != 3) {
				*err = ArityError;
				return NULL;
			}
			obj results = ast->list.rest->list.rest;
			if (is_truthy(EVAL(LIST_SECOND(ast), env, err))) {
				return EVAL(LIST_FIRST(results), env, err);
			} else {
				results = LIST_SECOND(results);
				if (NULL == results) return nil_o;
				return EVAL(results, env, err);
			}

		} else if (OBJ_IS_SYMBOL(first, "do")){
			ast = ast->list.rest;
			obj result = NULL;
			while (empty_list != ast && !*err) {
				result = EVAL(LIST_FIRST(ast), env, err);

				ast = ast->list.rest;
			}
			return result;

		} else {
			ast = eval_ast(ast, env, err);
			if (*err) return NULL;
			first = ast->list.first;
			obj rest = ast->list.rest;
			return invoke_fn(first, rest, err);
		}

	} else {
		return eval_ast(ast, env, err);
	}
}

void PRINT(obj ast) {
	print_string(pr_str(ast, 1), 1);
}

void rep(char *in, obj env, int *err) {
	obj ast = READ(in);
	if (!ast) return;
	ast = EVAL(ast, env, err);
	if (!ast) return;
	PRINT(ast);
}

static int *err_marker;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void keyboard_interrupt(int _)
{
	*err_marker = KeyboardInterrupt;
}
#pragma GCC diagnostic pop

int main() {
	char *line;

	obj repl_env = new_env(1, NULL);
	core_load_vars(repl_env);
	empty_list_str = new_string("()", 2);
	fn_str = new_string("#(...)", 6);
	nil_str = new_string("nil", 3);
	true_str = new_string("true", 4);
	false_str = new_string("false", 5);
	int err = 0;

	init_readline(repl_env);

	EVAL(READ("(def! not (fn* (a) (if a false true)))"), repl_env, &err);

	err_marker = &err;

	while(1) {
		line = next_line();
		if (!line) {
			fputc('\n', stdout);
			return 0;
		}
		if (*line) {
			rep(line, repl_env, &err);
		}
		free(line);

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
