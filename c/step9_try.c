#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <err.h>


#include "types.h"
#include "reader.h"
#include "printer.h"
#include "env.h"
#include "readline.h"
#include "core.h"

void print_malp_error(int error_type)
{
	if (error_type) {
		char buf[32];
		char *msg;
		switch (error_type) {
			case ArityError:
				msg = "Wrong number of args passed";
				break;
			case InvalidArgumentError:
				msg = "Invalid argument";
				break;
			case ReaderError:
				msg = "Could not parse";
				break;
			case SymbolNotFoundError:
				msg = "Unable to resolve symbol";
				break;
			case NotCallableError:
				msg = "Object is not callable";
				break;
			case IOError:
				msg = "I/O Error";
				break;
			case IndexOutOfBoundsError:
				msg = "Index out of bounds";
				break;
			default:
				sprintf(buf, "Unknown error type: %d", error_type);
				msg = buf;
				break;
		}
		error(0, 0, "Uncaught error: %s", msg);
	}
}

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
				el->list.rest = empty_list;
			}

			return res;
		}
		default:
			return ast;
	}
}

#define IS_PAIR(ast) ((ast)->type == List && (ast) != empty_list)

obj quasiquote(obj ast, int*err)
{
	if (!IS_PAIR(ast)) {
		return cons(cons(empty_list, ast), quote_sym);
	}
	// ast is a non-empty list
	obj first = LIST_FIRST(ast);
	if (OBJ_IS_SYMBOL(first, "unquote")) {
		if (first->list.count < 2) {
			*err = InvalidArgumentError;
			return NULL;
		}
		return LIST_SECOND(ast);
	}
	if (IS_PAIR(first)
		&& OBJ_IS_SYMBOL(LIST_FIRST(first), "splice-unquote")) {
		if (first->list.count < 2) {
			*err = InvalidArgumentError;
			return NULL;
		}
		return cons(cons(cons(empty_list, quasiquote(ast->list.rest, err)), LIST_SECOND(first)), concat_sym);
	}
	return cons(cons(cons(empty_list, quasiquote(ast->list.rest, err)), quasiquote(first, err)), cons_sym);
}

int is_macro_call(obj ast, obj env)
{
	return ast->type == List
		   && ast->list.count > 0
		   && (ast = env_get(env, LIST_FIRST(ast))) != NULL
		   && ast->type == Fn
		   && ast->fn.is_macro;
}

obj macroexpand(obj ast, obj env, int *err)
{
	while (is_macro_call(ast, env)) {
		obj macro = env_get(env, LIST_FIRST(ast));
		obj env = new_env((unsigned) macro->fn.binds->list.count, macro->fn.env);
		env_bind_args(env, macro->fn.binds, ast->list.rest, err);
		if (*err) return NULL;
		ast = EVAL(macro->fn.ast, env, err);
	}
	return ast;
}

obj EVAL(obj ast, obj env, int *err) {
eval_start:
	if (ast == NULL) return ast; // todo remove once reader is complete
	if (ast == empty_list) {
		return ast;
	} else if (ast->type == List) {
		ast = macroexpand(ast, env, err);
		if (*err) return ast;
		obj first = ast->list.first;

		if (ast->type != List) return eval_ast(ast, env, err);
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
			ast = result;
			env = let_env;
			goto eval_start;

		} else if (OBJ_IS_SYMBOL(first, "fn*")) {
			return new_fn(
				LIST_SECOND(ast->list.rest),
				env,
				LIST_FIRST(ast->list.rest)
			); // todo: (every? symbol? binds)

		} else if (OBJ_IS_SYMBOL(first, "if")) {
			if (ast->list.count != 4 && ast->list.count != 3) {
				*err = ArityError;
				return NULL;
			}
			obj results = ast->list.rest->list.rest;
			if (is_truthy(EVAL(LIST_SECOND(ast), env, err))) {
				ast = LIST_FIRST(results);
			} else {
				ast = LIST_SECOND(results);
				if (NULL == ast) return nil_o;
			}
			goto eval_start;

		} else if (OBJ_IS_SYMBOL(first, "do")) {
			ast = ast->list.rest;
			if (empty_list == ast) return nil_o;
			while (empty_list != ast->list.rest && !*err) {
				EVAL(LIST_FIRST(ast), env, err);

				ast = ast->list.rest;
			}
			if (*err) return NULL;
			ast = LIST_FIRST(ast);
			goto eval_start;

		} else if (OBJ_IS_SYMBOL(first, "quote")) {
			if (ast->list.count != 2) {
				*err = ArityError;
				return NULL;
			}
			return LIST_SECOND(ast);

		} else if (OBJ_IS_SYMBOL(first, "quasiquote")) {
			if (ast->list.count != 2) {
				*err = ArityError;
				return NULL;
			}
			ast = quasiquote(LIST_SECOND(ast), err);
			if (*err) return ast;
			goto eval_start;

		} else if (OBJ_IS_SYMBOL(first, "defmacro!")) {
			ast = ast->list.rest;
			if (ast->list.count != 2) {
				*err = ArityError; // todo
				return NULL;
			}
			obj symbol = LIST_FIRST(ast);
			if (symbol->type != Symbol) {
				*err = InvalidArgumentError; // todo
				return NULL;
			}
			obj value = EVAL(LIST_SECOND(ast), env, err);
			if (*err) return NULL;
			if (value->type != Fn) {
				*err = InvalidArgumentError;
				return NULL;
			}
			value->fn.is_macro = 1;
			env_set(env, symbol, value);
			return value;

		} else if (OBJ_IS_SYMBOL(first, "macroexpand")) {
			return macroexpand(LIST_SECOND(ast), env, err);

		} else {
			ast = eval_ast(ast, env, err);
			if (*err) return NULL;
			first = ast->list.first;
			obj rest = ast->list.rest;

			if (first->type == BuiltinFn) {
				return first->builtin_fn.fn(rest, err);
			} else if (first->type == Fn) {
				env = new_env((unsigned) first->fn.binds->list.count, first->fn.env);
				env_bind_args(env, first->fn.binds, rest, err);
				if (*err) return NULL;
				ast = first->fn.ast;
				goto eval_start;
			} else if (first->type == Keyword) {
				return NULL; // todo
			} else if (first->type == HashMap) {
				return NULL; // todo
			} else {
				*err = NotCallableError;
				return NULL;
			}
		}

	} else {
		return eval_ast(ast, env, err);
	}
}

void PRINT(obj ast) {
	print_string(pr_str(ast, 1), 1);
}

void rep(char *in, obj env) {
	obj ast = READ(in);
	free(in);
	if (!ast) return;
	int err = 0;
	ast = EVAL(ast, env, &err);
	print_malp_error(err);
	if (!ast) return;

	obj tmp = env_get(env, STAR2_sym);
	if (NULL != tmp) env_set(env, STAR3_sym, tmp);
	tmp = env_get(env, STAR1_sym);
	if (NULL != tmp) env_set(env, STAR2_sym, tmp);
	env_set(env, STAR1_sym, ast);

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

DEF_BUILTIN_FN(malp_core_eval)(obj ast, int *err)
{
	static obj env;
	if (NULL == env) {
		env = ast;
		return NULL;
	}
	return EVAL(LIST_FIRST(ast), env, err);
}

int main(int argc, char **argv) {
	obj ARGV = empty_list;

	char *run_file_name = NULL;
	char *run_cmd = NULL;
	int arg_start = argc;

	if (argc > 1) {
		if (!strcmp(argv[1], "-c")) {
			if (argc < 2) {
				error(2, 0, "Argument expected for the -c option");
			}
			run_cmd = argv[2];
			arg_start = 3;
		} else if (!strcmp(argv[1], "-")) {
			arg_start = 2;
		} else {
			run_file_name = argv[1];
			arg_start = 2;
		}
	}

	for (int i = argc - 1; i >= arg_start; --i) {
		ARGV = cons(ARGV, new_string(
			argv[i],
			strlen(argv[i])
		));
	}

	cons_sym = new_symbol("cons", 4);
	quote_sym = new_symbol("quote", 5);
	concat_sym = new_symbol("concat", 6);
	STAR1_sym = new_symbol("*1", 2);
	STAR2_sym = new_symbol("*2", 2);
	STAR3_sym = new_symbol("*3", 2);

	obj repl_env = new_env(64, NULL);
	init_readline(repl_env);

	core_load_vars(repl_env);

	env_set(repl_env, new_symbol("*ARGV*", 6), ARGV);
	env_set(repl_env, new_symbol("eval", 4), malp_core_eval);
	malp_core_eval_(repl_env, NULL);
	empty_list_str = new_string("()", 2);
	empty_string = new_string("", 0);
	fn_str = new_string("#(...)", 6);
	nil_str = new_string("nil", 3);
	true_str = new_string("true", 4);
	false_str = new_string("false", 5);

	int error_flag = 0;
	EVAL(READ("(def! not (fn* (a) (if a false true)))"), repl_env, &error_flag);
	EVAL(READ("(def! load-file (fn* (f) (eval (read-string (str \"(do \" (slurp f) \")\")))))"), repl_env, &error_flag);
	EVAL(READ("(def! inc (fn* (n) (+ n 1)))"), repl_env, &error_flag);
	EVAL(READ("(def! dec (fn* (n) (- n 1)))"), repl_env, &error_flag);
	EVAL(READ("(defmacro! cond (fn* (& xs) (if (> (count xs) 0) (list 'if (first xs) (if (> (count xs) 1) (nth xs 1) (throw \"odd number of forms to cond\")) (cons 'cond (rest (rest xs)))))))"), repl_env, &error_flag);
	EVAL(READ("(defmacro! or (fn* (& xs) (if (empty? xs) nil (if (= 1 (count xs)) (first xs) `(let* (or_FIXME ~(first xs)) (if or_FIXME or_FIXME (or ~@(rest xs))))))))"), repl_env, &error_flag);

	err_marker = &error_flag;

	if (NULL != run_file_name) {
		ARGV = cons(ARGV, new_string(
			run_file_name,
			strlen(run_file_name)
		));
		env_set(repl_env, new_symbol("*ARGV*", 6), ARGV);
		EVAL(READ("(load-file (first *ARGV*))"), repl_env, &error_flag);
		print_malp_error(error_flag);

	} else if (NULL != run_cmd) {
		EVAL(READ(run_cmd), repl_env, &error_flag);
		print_malp_error(error_flag);

	} else while(1) {
		char *line = next_line();
		if (!line) {
			fputc('\n', stdout);
			return 0;
		}
		if (*line) {
			rep(line, repl_env);
		} else {
			free(line);
		}
	}
}
