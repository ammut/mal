#include <stdio.h>
#include <string.h>

#include "types.h"
#include "printer.h"

static void pr_string(char *value, size_t len);

void print_str(MalValue *ast) {
	if (!ast) return;
	switch(ast->type) {
	case List: {
		if (ast == &empty_list) {
			fputs("()", stdout);
			break;
		}
		MalList *l = (void*)ast->value;
		putchar('(');
		do {
			print_str(l->first);
			if (l->rest) fputc(' ', stdout);
		} while (l->rest && (l = (MalList*)l->rest->value));
		putchar(')');
		break;
	}
	case Number: {
		MalNumber *n = (void*)ast->value;
		if (n->type == FLOAT) {
			printf("%lf", n->value.FLOAT);
		} else if (n->type == INT) {
			printf("%" PRId64, n->value.INT);
		}
		break;
	}
	case Symbol:
		printf("%s", (MalSymbol)ast->value);
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
		pr_string(((MalString*)ast->value)->string,
					 ((MalString*)ast->value)->length);
		break;
	case Keyword:
	case Vector:
	case Atom:
	default:
		break;	
	}
}

static void pr_string(char *value, size_t len) {
	char c;
	putchar('"');
	while (len--) {
		c = *value++;
		switch(c) {
		case '\b':
			fputs("\\b", stdout); break;
		case '\t':
			fputs("\\t", stdout); break;
		case '\n':
			fputs("\\n", stdout); break;
		case '\f':
			fputs("\\f", stdout); break;
		case '\r':
			fputs("\\r", stdout); break;
		case '"':
			fputs("\\\"", stdout); break;
		default:
			putchar(c); break;
		}
	}
	putchar('"');
}
