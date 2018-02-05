#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "types.h"
#include "gc.h"
#include "reader.h"
#include "printer.h"

MalValue *READ(char *in, GcRoot *env) {
	MalValue *r = read_str(in, env);
	free(in);
	return r;
}

MalValue *EVAL(MalValue *ast, GcRoot *env) {
	env->repl_point = ast;
	return ast;
}

void PRINT(MalValue *ast) {
	print_str(ast);
	puts("");
}

void rep(char *in, GcRoot *env) {
	PRINT(EVAL(READ(in, env), env));
}

void init_readline();
int main() {
	char *line;
	init_readline();
	GcRoot gc = new_gc;
	while(1) {
		line = readline("user> ");
		if (!line) {
			puts("");
			return 0;
		}
		if (*line) {
			add_history(line);
			rep(line, &gc);
		} else {
			free(line);
		}
//		gc_run(&gc);
	}
}

void init_readline() {
	rl_initialize();
}
