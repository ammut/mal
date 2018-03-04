#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "types.h"
#include "reader.h"
#include "printer.h"

obj READ(char *in) {
	obj r = read_str(in);
	return r;
}

obj EVAL(obj ast) {
	return ast;
}

void PRINT(obj ast) {
	print_str(ast);
	putchar('\n');
}

void rep(char *in) {
	PRINT(EVAL(READ(in)));
}

void init_readline();
int main() {
	char *line;
	init_readline();
	while(1) {
		line = readline("user> ");
		if (!line) {
			puts("");
			return 0;
		}
		if (*line) {
			add_history(line);
			rep(line);
		} else {
			free(line);
		}
	}
}

void init_readline() {
	rl_initialize();
}
