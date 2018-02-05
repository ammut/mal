#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

char *READ(char *in)
{
	return in;
}

char *EVAL(char *ast)
{
	return ast;
}

char *PRINT(char *ast)
{
	return ast;
}

char *rep(char *in)
{
	return PRINT(EVAL(READ(in)));
}

int main()
{
	char *line;
	while(1) {
		line = readline("user> ");
		if (!line) {
			return 0;
		}
		add_history(line);
		puts(rep(line));
		free(line);
	}
}
