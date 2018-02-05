#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "types.h"

void destroy_mal_value(MalValue *v) {
	if (!v) return;
	switch(v->type) {
		case List:
		case Number:
		case Symbol:
		case String:
			free(v);
		case Nil:
		case True:
		case False:
			break; // do nothing
		case Keyword:
		case Vector:
		case HashMap:
		case Atom:
		default:
			break;
	}
}

// LIST

MalValue *mal_list() {
	MalValue *r;
	size_t r_size = (sizeof *r + sizeof(MalList) + 3) / 4 * 4;
	r = malloc(r_size);
	if (r == NULL) return r;
	r->type = List;
	r->gc_mark = 0;
	((MalList*)r->value)->first = NULL;
	((MalList*)r->value)->count = 0;
	return r;
}

MalValue *mal_list_from(MalValue *el) {
	MalValue *r = mal_list();
	MalList *list = (MalList*)r->value;
	list->first = el;
	list->count = 1;
	list->rest = NULL;
	return r;
}

MalValue *add_last_mutating(MalValue *h, MalValue *el) {
	MalValue *new = mal_list();
	if (!new) return NULL;
	MalList *l = (MalList*)h->value;
	while (l->rest) {
		++l->count;
		l = (MalList*)l->rest->value;
	}
	l->rest = new;
	l = (MalList*)new->value;
	l->first = el;
	l->count = 1;
	l->rest = NULL;
	return new;
}

MalValue empty_list = {
	.type = List,
};

// NUMBER

MalNumber parse_number(char *token) {
	MalIntValue intval;
	MalFloatValue floatval;
	int success;
	MalNumber r = { .value.INT = 0, .type = NONE };
	
	success = sscanf(token, "%" PRId64, &intval) +
		(sscanf(token, "%lf", &floatval) << 1);
	switch (success) {
		case 3:
			if (intval == floatval) goto parse_number_int;
		case 2:
			r.type = FLOAT;
			r.value.FLOAT = floatval;
			break;
		case 1:
		parse_number_int:
			r.type = INT;
			r.value.INT = intval;
		default:
			break;
	}
	return r;
}

static MalValue *mal_number() {
	MalValue *r;
	size_t r_size = sizeof *r + sizeof(MalNumber);
	r_size = (r_size + 3) / 4 * 4;
	r = malloc(r_size);
	if (r == NULL) return r;
	r->type = Number;
	r->gc_mark = 0;
	return r;
}

MalValue *mal_number_int(MalIntValue value) {
	MalValue *r = mal_number();
	if (r == NULL) return r;
	*(MalNumber*)r->value = (MalNumber){ .value.INT = value, .type = INT };
	return r;
}

MalValue *mal_number_float(MalFloatValue value) {
	MalValue *r = mal_number();
	if (r == NULL) return r;
	*(MalNumber*)r->value = (MalNumber){ .value.FLOAT = value, .type = FLOAT };
	return r;
}

MalValue *mal_number_from(char *token) {
	MalNumber n = parse_number(token);
	if (n.type == NONE) return NULL;
	MalValue *r = mal_number();
	if (r == NULL) return r;
	*(MalNumber*)r->value = n;
	return r;
}

// SYMBOL

MalValue *mal_symbol(char *token, size_t length) {
	MalValue *r;
	size_t r_size = sizeof *r + length + 1;
	r_size = (r_size + 3) / 4 * 4;
	r = malloc(r_size);
	if (r == NULL) return r;
	r->type = Symbol;
	r->gc_mark = 0;
	strcpy((MalSymbol)r->value, token);
	return r;
}

// NIL

MalValue mal_nil = { .type = Nil };

int is_nil(MalValue *other) {
	return other == &mal_nil;
}

// TRUE

MalValue mal_true = { .type = True };

int is_true(MalValue *other) {
	return other == &mal_true;
}

int truthy(MalValue *other) {
	return !is_false(other) && !is_nil(other);
}

// FALSE

MalValue mal_false = { .type = False };

int is_false(MalValue *other) {
	return other == &mal_false;
}

int falsey(MalValue *other) {
	return is_false(other) || is_nil(other);
}

// STRING

MalValue *mal_string(char *value, size_t len) {
	MalValue *r;
	size_t r_size = sizeof *r + sizeof(MalString) + len;
	r_size = (r_size + 3) / 4 * 4;
	r = malloc(r_size);
	if (r == NULL) return r;
	r->type = String;
	r->gc_mark = 0;
	memcpy(((MalString*)r->value)->string, value, len);
	((MalString*)r->value)->length = len;
	return r;
}

static size_t unescape_mal_string(char *buf, char *value, size_t len, int *err);

MalValue *mal_string_from(char *token, size_t len) {
	char buf[(len - 2) * 4]; // -2 quotes, x4 unicode
	int err;
	len = unescape_mal_string(buf, token + 1, len - 2, &err);
	if (err) return NULL;
	return mal_string(buf, len);
}

static int unescape_hex(char **buffer, char **token, int *err);
static int unescape_unicode(char **buffer, char **token, int *err);
static int unescape_octal(char **buffer, char **token, int *err);
static int16_t hex2i(char);
/*
 * we escape: \", \\, \b, \f, \n, \r, \t, \ooo, \xXX, \uXXXX, \UXXXXXXXX
 */
static size_t unescape_mal_string(char *buffer, char *token, size_t len, int *err) {
	char *end = token + len;
	char *start = buffer;

	while(token < end) {
		if (*token == '\\') {
			switch(*(++token)) {
			case '"':
			case '\\':
				*buffer++ = *token; ++token; break;
			case 'b':
				*buffer++ = 0x08; ++token; break;
			case 'f':
				*buffer++ = 0x0c; ++token; break;
			case 'n':
				*buffer++ = 0x0a; ++token; break;
			case 'r':
				*buffer++ = 0x0d; ++token; break;
			case 't':
				*buffer++ = 0x09; ++token; break;
			case 'v':
				*buffer++ = 0x0b; ++token; break;
			case 'x':
				unescape_hex(&buffer, &token, err); break;
			case 'u':
			case 'U':
				unescape_unicode(&buffer, &token, err); break;
			default:
				if (*token >= '0' && *token < '8') {
					unescape_octal(&buffer, &token, err);
				} else {
					*err = 1;
					return buffer - start;
				}
			}

		} else {
			*buffer++ = *token++;
		}
	}

	return buffer - start;
}


static int unescape_hex(char **_buffer, char **_token, int *err) {
	char a, b;
	char *buffer = *_buffer, *token = *_token;
	if (-1 == (a = hex2i(*token++))) {
		*err = 1;
		return 0;
	}

	if (-1 != (b = hex2i(*token++))) { // x++ > *x
		a = (a << 4u) + b;
	}
	*buffer++ = a;

	while (-1 != b && -1 != (a = hex2i(*token++))) {
		if (-1 != (b = hex2i(*token++))) {
			a = (a << 4u) + b;
		}
		*buffer++ = a;
	}
	*_buffer = buffer;
	*_token = token;
	return 1;
}

static int unescape_unicode(char **_buffer, char **_token, int *err) {
	int16_t a, b, c, d;
	int32_t code;
	char *buffer = *_buffer, *token = *_token;
	if (-1 != (a = hex2i(*(token + 1))) &&
		-1 != (b = hex2i(*(token + 2))) &&
		-1 != (c = hex2i(*(token + 3)))	&&
		-1 != (d = hex2i(*(token + 4)))) {
		
		code = d + (c << 4u) + (b << 8u) + (a << 12u);
		
		if ('U' == *token) {
			if (-1 != (a = hex2i(*(token + 5))) &&
				-1 != (b = hex2i(*(token + 6))) &&
				-1 != (c = hex2i(*(token + 7))) &&
				-1 != (d = hex2i(*(token + 8)))) {
				code = d + (c << 4u) + (b << 8u) + (a << 12u) + (code << 16u);
				token += 4;
			} else {
				goto unescape_unicode_err;
			}
		}

		token += 5;

		if (code < 0x80) {
			*buffer++ = code;
		} else if (code < 0x800) {
			*buffer++ = 192 + code / 64;
			*buffer++ = 128 + code % 64;
		} else if (code - 0xd800u < 0x800) {
			goto unescape_unicode_err;
		} else if (code < 0x10000) {
			*buffer++ = 224 + code / 4096;
			*buffer++ = 128 + code / 64 % 64;
			*buffer++ = 128 + code % 64;
		} else if (code < 0x110000) {
			*buffer++ = 240 + code / 262144;
			*buffer++ = 128 + code / 4096 % 64;
			*buffer++ = 128 + code / 64 % 64;
			*buffer++ = 128 + code % 64;
		} else {
			goto unescape_unicode_err;
		}
		*_buffer = buffer;
		*_token = token;
		return 1;
	} else {
		unescape_unicode_err:
		*err = 1;
		return 0;
	}

}

static int unescape_octal(char **_buffer, char **_token, int *err) {
	char *buffer = *_buffer, *token = *_token;
	if (*token >= '0' && *token < '8') {
		*buffer = *token++ - '0';
		if (*token >= '0' && *token < '8') {
			*buffer += *token++ - '0';
			if (*token >= '0' && *token < '8') {
				*buffer += *token++ - '0';
			}
		}
		*_buffer = buffer;
		*_token = token;
		return 1;
	} else {
	    *err = 1;
		return 0;
	}
}

static int16_t hex2i(char c) {
	return c >= '0' && c <= '9' ?
		c - '0' :
		(c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f') ?
		(c & ~0x20) - 'A' + 10 :
		-1;
}


