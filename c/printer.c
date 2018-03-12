#include <stdio.h>
#include <string.h>

#include "types.h"
#include "printer.h"

void print_string(obj string, int newline)
{
	fwrite(
		string->string.value,
		sizeof(string->string.value[0]),
		string->string.length,
		stdout
	);
	if (newline) fputc('\n', stdout);
}

#if 0
static size_t string_lengths(obj l)
{
	size_t total = 2 + l->list.count - 1; // brackets + spaces
	while (l != empty_list) {
		total += LIST_FIRST(l)->string.length;

		l = l->list.rest;
	}
	return total;
}
#endif

static obj pr_str_readably(obj str)
{
	char buf[str->string.length * 2 + 2]; // *2 for \x -> \\x, +2 for ""
	char *const value = str->string.value;

	size_t buf_pos = 0;
	size_t val_pos = 0;
	const size_t len = str->string.length;

	buf[buf_pos++] = '"';

	while (val_pos < len) {
		char c = value[val_pos++];
		switch (c) {
			case '\b':
				buf[buf_pos++] = '\\';
				buf[buf_pos++] = 'b';
				break;
			case '\t':
				buf[buf_pos++] = '\\';
				buf[buf_pos++] = 't';
				break;
			case '\n':
				buf[buf_pos++] = '\\';
				buf[buf_pos++] = 'n';
				break;
			case '\f':
				buf[buf_pos++] = '\\';
				buf[buf_pos++] = 'f';
				break;
			case '\r':
				buf[buf_pos++] = '\\';
				buf[buf_pos++] = 'r';
				break;
			case '\\':
				buf[buf_pos++] = '\\';
				buf[buf_pos++] = '\\';
				break;
			case '"':
				buf[buf_pos++] = '\\';
				buf[buf_pos++] = '"';
				break;
			default:
				buf[buf_pos++] = c;
				break;
		}
	}
	buf[buf_pos++] = '"';
	return new_string(buf, buf_pos);
}

obj pr_str_all(obj ast, int flags)
{
	unsigned with_spacing = flags & PRSTRALL_SPACING ? 1 : 0;
	unsigned with_brackets = flags & PRSTRALL_BRACKETS ? 1 : 0;
	int print_readably = flags & PRSTRALL_READABLY;

	if (ast == empty_list && with_brackets) {
		return new_empty_string(2);
	} else if (ast == empty_list) {
		return empty_string;
	}

	size_t total_size = (ast->list.count - 1) * with_spacing + with_brackets * 2;

	obj res = new_list();
	res->list.count = ast->list.count;
	LIST_FIRST(res) = pr_str(LIST_FIRST(ast), print_readably);
	total_size += LIST_FIRST(res)->string.length;

	obj el = res;
	while ((ast = ast->list.rest) != empty_list) {
		el->list.rest = new_list();
		el = el->list.rest;
		el->list.count = ast->list.count;
		LIST_FIRST(el) = pr_str(LIST_FIRST(ast), print_readably);
		el->list.rest = empty_list;
		total_size += LIST_FIRST(el)->string.length;
	}

	el = new_empty_string(total_size);
	char *buf = el->string.value;
	size_t pos = with_brackets;
	while (res != empty_list) {
		memcpy(buf + pos,
			   LIST_FIRST(res)->string.value,
			   LIST_FIRST(res)->string.length);
		pos += LIST_FIRST(res)->string.length;

		if (pos < total_size && with_spacing) {
			buf[pos++] = ' ';
		}

		res = res->list.rest;
	}

	return el;
}

obj pr_str(obj ast, int print_readably)
{
	switch (ast->type) {
		case List: {
			int flags = PRSTRALL_BRACKETS |
				PRSTRALL_SPACING |
				(print_readably ? PRSTRALL_READABLY : 0);
			ast = pr_str_all(ast, flags);
			ast->string.value[0] = '(';
			ast->string.value[ast->string.length - 1] = ')';

			return ast;
		}
		case Int: {
			char buf[21] = { 0 }; // longest int value
			sprintf(buf, "%" PRId64, ast->integer.value);
			return new_string(buf, strlen(buf));
		}
		case Real: {
			char buf[16] = { 0 }; // - . e EXP(- <4>) \0
			sprintf(buf, "%lg", ast->real.value);
			return new_string(buf, strlen(buf));
		}
		case Ratio: {
			char buf[41] = {0};
			sprintf(buf, "%" PRId64 "/%" PRIu64, ast->ratio.numerator,
				   ast->ratio.denominator);
			return new_string(buf, strlen(buf));
		}
		case Symbol:
		case Keyword:
			return new_string(ast->symbol.name, strlen(ast->symbol.name));
		case Nil:
			if (print_readably) return nil_str;
			return empty_string;
		case True:
			return true_str;
		case False:
			return false_str;
		case String:
			if (print_readably) return pr_str_readably(ast);
			return ast;
		case BuiltinFn:
		case Fn:
			return fn_str;
		case Atom: {
			obj val = ast->atom.value;
			val = pr_str(val, print_readably);
			ast = new_empty_string(7 + val->string.length);
			sprintf(ast->string.value, "(atom %s)", val->string.value);
			return ast;
		}
		case Vector:
		case HashMap:
		default:
			return new_string("<impl missing>", 14);
	}
}
