#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "types.h"
#include "reader.h"
#include "printer.h"
#include "env.h"

obj READ(char *in) {
	obj r = read_str(in);
	return r;
}

obj EVAL(obj ast, obj env, int *err);

obj eval_ast(obj ast, obj env, int *err)
{
	switch (ast->type) {
		case Symbol: {
			obj value = env_get(env, ast->symbol.name);
			if (NULL == value) *err = 1; // todo
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
	if (ast == empty_list) {
		return ast;
	} else if (ast->type == List) {
		ast = eval_ast(ast, env, err);
		if (*err) return NULL;
		obj first = ast->list.first;
		obj rest = ast->list.rest;
		if (first->type != Fn) {
			*err = 1; // todo
			return NULL;
		}
		return first->fn.fn(rest, err);
	} else {
		return eval_ast(ast, env, err);
	}
}

void PRINT(obj ast) {
	print_str(ast);
	putchar('\n');
}

void rep(char *in, obj env, int *err) {
	PRINT(EVAL(READ(in), env, err));
}

void init_readline();
int main() {
	char *line;
	init_readline();
	init_builtins();

	obj env = (obj)&(malp_env){
		.type = Env,
		.data = builtins,
	};
	int err;

	while(1) {
		line = readline("user> ");
		if (!line) {
			puts("");
			return 0;
		}
		if (*line) {
			add_history(line);
			rep(line, env, &err);
		} else {
			free(line);
		}

		if (err) {
			fputs("something bad happened!\n", stderr);
			err = 0;
		}
	}
}

void init_readline() {
	rl_initialize();
}
