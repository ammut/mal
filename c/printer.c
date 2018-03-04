#include <stdio.h>
#include <string.h>

#include "types.h"
#include "printer.h"

static void pr_string(char *value, size_t len);

void print_str(obj ast)
{
	if (!ast) { return; }
	switch (ast->type) {
		case List:
			putchar('(');
			while (ast != empty_list) {
				print_str(ast->list.first);
				ast = ast->list.rest;
				if (ast != empty_list) fputc(' ', stdout);
			}
			putchar(')');
			break;
		case Int:
			printf("%" PRId64, ast->integer.value);
			break;
		case Real:
			printf("%lf", ast->real.value);
			break;
		case Ratio:
			printf("%" PRId64 "/%" PRIu64, ast->ratio.numerator,
				   ast->ratio.denominator);
			break;
		case Symbol:
			printf("%s", ast->symbol.name);
			break;
		case Nil:
			printf("nil");
			break;
		case True:
			printf("true");
			break;
		case False:
			printf("false");
			break;
		case String:
			pr_string(ast->string.value,
					  ast->string.length);
			break;
		case Fn:
			printf("#(...)");
		case Keyword:
		case Vector:
		case Atom:
		case HashMap:
		default:
			break;
	}
}

static void pr_string(char *value, size_t len)
{
	char c;
	putchar('"');
	while (len--) {
		c = *value++;
		switch (c) {
			case '\b':
				fputs("\\b", stdout);
				break;
			case '\t':
				fputs("\\t", stdout);
				break;
			case '\n':
				fputs("\\n", stdout);
				break;
			case '\f':
				fputs("\\f", stdout);
				break;
			case '\r':
				fputs("\\r", stdout);
				break;
			case '"':
				fputs("\\\"", stdout);
				break;
			default:
				putchar(c);
				break;
		}
	}
	putchar('"');
}
