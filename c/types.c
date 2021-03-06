#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdarg.h>

#include "types.h"
#include "math.h"
#include "main.h"
#include "env.h"

#ifdef MPS
#define ALIGNMENT sizeof(mps_word_t)
#else
#define ALIGNMENT (sizeof(void*))
#endif

/* Align size upwards to the next multiple of the word size. */
#define ALIGN_WORD(size) \
  (((size) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))

/* Align size upwards to the next multiple of the word size, and
 * additionally ensure that it's big enough to store a forwarding
 * pointer. Evaluates its argument twice. */
#define ALIGN_OBJ(size)                                \
  (ALIGN_WORD(size) >= ALIGN_WORD(sizeof(malp_fwd))       \
   ? ALIGN_WORD(size)                                  \
   : ALIGN_WORD(sizeof(malp_fwd)))

char *type_name_string(obj object)
{
	switch (object->type) {
		case List:
			return "List";
		case Int:
		case Real:
		case Ratio:
			return "Number";
		case Symbol:
			return "Symbol";
		case Keyword:
			return "Keyword";
		case Nil:
			return "Nil";
		case True:
		case False:
			return "Bool";
		case String:
			return "String";
		case Atom:
			return "Atom";
		case BuiltinFn:
		case Fn:
			return "Function";
		case Error:
			return "Error";
		case Vector:
			return "Vector";
		case HashMap:
			return "HashMap";
		default:
			printf("missing type string: %d\n", object->type);
			exit(1);
	}
}

// LIST

const obj const empty_list = (obj)&(const malp_list){
	.type = List,
	.count = 0,
	.first = NULL,
	.rest = NULL,
};

obj cons(obj list, obj el)
{
	obj r = new_list();
	r->list.count = list->list.count + 1;
	r->list.first = el;
	r->list.rest = list;
	return r;
}

obj new_list()
{
	obj r = malloc(ALIGN_OBJ(sizeof(malp_list)));
	if (r == NULL) return r;
	r->type = List;
	r->list.first = NULL;
	r->list.rest = empty_list;
	r->list.count = 0;
	return r;
}

obj new_list_from(size_t count, ...)
{
	if (0 == count) return empty_list;

	obj head = NULL;
	obj tail = head;

	va_list elements;
	va_start(elements, count);
	size_t cnt = count;
	while (cnt-- > 0) {
		tail = append_mutating(tail, va_arg(elements, obj));
		if (NULL == head) head = tail; // set head
	}
	va_end(elements);
	set_length_mutating(head, count);
	return head;
}

obj append_mutating(obj l, obj el)
{
	obj tail = new_list();
	if (NULL == tail) return tail;
	if (NULL == l) l = tail;
	while (l->list.rest != empty_list) {
		l = l->list.rest;
	}
	l->list.rest = tail;
	tail->list.first = el;
	tail->list.rest = empty_list;
	return tail;
}

void set_length_mutating(obj l, size_t c)
{
	while (c) {
		l->list.count = c--;
		l = l->list.rest;
	}
}

obj coll_last(obj collection)
{
	if (collection->type == List) {
		if (collection == empty_list) return empty_list;
		while (collection->list.rest != empty_list)
			collection = collection->list.rest;
		return LIST_FIRST(collection);
	}
	return NULL;
}

// NUMBER

static obj new_number(size_t size)
{
	obj r = malloc(size);
	if (r == NULL) return r;
	return r;
}

obj read_number(char *token)
{
	obj r = NULL;
	int scans = 0;
	malp_int_t int_val;
	malp_real_t float_val;

	scans =
		sscanf(token, "%" PRId64, &int_val) |
		(sscanf(token, "%lf", &float_val) << 1);
	switch (scans) {
		case 0x3: {
			if (int_val == float_val) goto parse_number_int;
		}
		case 0x2: {
			r = new_real(float_val);
			break;
		}
		case 0x1:
		parse_number_int: {
			char *slash;
			if ((slash = strchr(token, '/')) == NULL) {
				r = new_int(int_val);

			// slash found -> tokenizer guarantees success
			} else {
				malp_denom_t denominator;
				scans = sscanf(++slash, "%" PRIu64, &denominator);
				if (scans) {
					r = new_ratio(int_val, denominator);
				}
			}
		}
		default: {
			break;
		}
	}
	return r;
}

obj new_int(malp_int_t value)
{
	obj r = new_number(ALIGN_OBJ(sizeof(malp_int)));
	if (r == NULL) return r;
	r->integer = (malp_int){
		.type = Int,
		.value = value,
	};
	return r;
}

obj new_real(malp_real_t value)
{
	obj r = new_number(ALIGN_OBJ(sizeof(malp_real)));
	if (r == NULL) return r;
	r->real = (malp_real){
		.type = Real,
		.value = value,
	};
	return r;
}

obj new_ratio(malp_int_t numerator, malp_denom_t denominator)
{
	obj r = new_number(ALIGN_OBJ(sizeof(malp_ratio)));
	if (r == NULL) return r;
	r->ratio = (malp_ratio){
		.type = Ratio,
		.numerator = numerator,
		.denominator = denominator,
	};
	NORMALIZE_RATIO(r->ratio);
	return r;
}

int is_number(obj o)
{
	return o->type == Real || o->type == Int || o->type == Ratio;
}

// SYMBOL

obj new_symbol(char *token)
{
	size_t length = strlen(token);
	obj r = malloc(ALIGN_OBJ(sizeof(malp_symbol) + length + 1));
	if (r == NULL) return r;
	r->type = Symbol;
	strcpy(r->symbol.name, token);
	return r;
}

// KEYWORD

obj new_keyword(char *token, size_t length)
{
	obj r = malloc(ALIGN_OBJ(sizeof(malp_symbol) + length + 1));
	if (r == NULL) return r;
	r->type = Keyword;
	strcpy(r->symbol.name, token);
	return r;
}

// NIL

obj nil_o = &(obj_s){
	.type = Nil
};

int is_nil(obj obj)
{
	return obj == nil_o;
}

// TRUE

obj true_o = &(obj_s){
	.type = True
};

int is_true(obj obj)
{
	return obj == true_o;
}

int is_truthy(obj obj)
{
	return !is_falsey(obj);
}

// FALSE

obj false_o = &(obj_s){
	.type = False
};

int is_false(obj obj)
{
	return obj == false_o;
}

int is_falsey(obj obj)
{
	return is_false(obj) || is_nil(obj);
}

// STRING

obj empty_string = (obj)&(malp_string){
	.type = String,
	.length = 0,
};

obj new_empty_string(size_t len)
{
	obj r  = malloc(ALIGN_OBJ(sizeof(malp_string) + len + 1));
	if (r == NULL) return r;
	r->type = String;
	r->string.length = len;
	r->string.value[len] = 0;
	return r;
}

obj new_string(char *value)
{
	size_t len = strlen(value);
	obj r  = malloc(ALIGN_OBJ(sizeof(malp_string) + len + 1));
	if (r == NULL) return r;
	r->type = String;
	r->string.length = len;
	strcpy(r->string.value, value);
	r->string.value[len] = 0;
	return r;
}

static size_t decode_malp_string(char *buffer, char *token, size_t len, int *err);

obj read_string(char *token, size_t len)
{
	char buf[ALIGN_WORD(len + 1)];
	int err = 0;
	len = decode_malp_string(buf, token + 1, len - 2, &err);
	if (err) return NULL;
	buf[len] = 0;
	return new_string(buf);
}

static int decode_hex(char **buffer, char **token, int *err);
static int decode_octal(char **buffer, char **token, int *err);
static int decode_unicode(char **buffer, char **token, int *err);

/*
 * we escape: \", \\, \b, \f, \n, \r, \t, \ooo, \xXX, \uXXXX, \UXXXXXXXX
 */
static size_t decode_malp_string(char *buffer, char *token, size_t len,
								 int *err)
{
	char *end = token + len;
	char *start = buffer;

	while(token < end) {
		if (*token != '\\') {
			*buffer++ = *token++;

		} else {
			switch(*(++token)) {
				case '"':
				case '\\': *buffer++ = *token; ++token; break;
				case 'b': *buffer++ = 0x08; ++token; break;
				case 'f': *buffer++ = 0x0c; ++token; break;
				case 'n': *buffer++ = 0x0a; ++token; break;
				case 'r': *buffer++ = 0x0d; ++token; break;
				case 't': *buffer++ = 0x09; ++token; break;
				case 'v': *buffer++ = 0x0b; ++token; break;
				case 'x': decode_hex(&buffer, &token, err); break;
				case 'u':
				case 'U': decode_unicode(&buffer, &token, err); break;
				default:
					if (*token >= '0' && *token < '8') {
						decode_octal(&buffer, &token, err);
					} else {
						*err = 1;
						return buffer - start;
					}
			}
		}
	}

	return buffer - start;
}

static int hex2i(char c)
{
	int b = (uint8_t)c;
	int maskLetter = (('9' - b) >> 31);
	int maskSmall = (('Z' - b) >> 31);
	int offset = '0' + (maskLetter & ((int)('A' - '0' - 10))) + (maskSmall & ((int)('a' - 'A')));
	return b - offset;
}

static int decode_hex(char **buffer, char **token, int *err)
{
	char *_token = *token, *_buffer = *buffer;
	*_buffer = (char)((hex2i(*_token) << 4u) | hex2i(*(_token + 1)));
	*buffer = _buffer;
	*token = _token;
	*err = 0; // -Werror=unused-parameter
	return 1;
}

static int decode_octal(char **buffer, char **token, int *err)
{
	char *_buffer = *buffer, *_token = *token;
	*_buffer = (char)(*_token++ - '0');
	if (*_token >= '0' && *_token < '8') {
		*_buffer += *_token++ - '0';
		if (*_token >= '0' && *_token < '8') {
			*_buffer += *_token++ - '0';
		}
	}
	*buffer = _buffer + 1;
	*token = _token;
	*err = 0; // -Werror=unused-parameter
	return 1;
}

static int decode_unicode(char **buffer, char **token, int *err)
{
	int16_t a, b, c, d;
	int32_t code;
	char *_buffer = *buffer, *_token = *token;
	if (-1 != (a = hex2i(_token[1])) &&
		-1 != (b = hex2i(_token[2])) &&
		-1 != (c = hex2i(_token[3]))	&&
		-1 != (d = hex2i(_token[4]))) {

		code = d + (c << 4u) + (b << 8u) + (a << 12u);

		if ('U' == *_token) {
			if (-1 != (a = hex2i(_token[5])) &&
				-1 != (b = hex2i(_token[6])) &&
				-1 != (c = hex2i(_token[7])) &&
				-1 != (d = hex2i(_token[8]))) {
				code = d + (c << 4u) + (b << 8u) + (a << 12u) + (code << 16u);
				_token += 4;
			} else {
				goto decode_unicode_err;
			}
		}

		if ((code >= 0x800 && code - 0xd800u < 0x800) ||
			(code >= 0x110000)) goto decode_unicode_err;

		_token += 5;
		*token = _token;

		if (code < 0x80) {
			*_buffer++ = code;
		} else if (code < 0x800) {
			*_buffer++ = 192 + code / 64;
			*_buffer++ = 128 + code % 64;
		} else if (code < 0x10000) {
			*_buffer++ = 224 + code / 4096;
			*_buffer++ = 128 + code / 64 % 64;
			*_buffer++ = 128 + code % 64;
		} else if (code < 0x110000) {
			*_buffer++ = 240 + code / 262144;
			*_buffer++ = 128 + code / 4096 % 64;
			*_buffer++ = 128 + code / 64 % 64;
			*_buffer++ = 128 + code % 64;
		}
		*buffer = _buffer;
		return 1;
	} else {
		decode_unicode_err:
		*err = 1;
		return 0;
	}

}

// ATOM

obj new_atom(obj value)
{
	obj r = malloc(ALIGN_OBJ(sizeof(malp_atom)));
	if (r == NULL) return r;
	r->type = Atom;
	r->atom.value = value;
	return r;
}

// FUNCTION

obj new_fn(obj ast, obj env, obj binds)
{
	obj r = malloc(ALIGN_OBJ(sizeof(malp_fn)));
	if (r == NULL) return r;
	r->type = Fn;
	r->fn.is_macro = 0;
	r->fn.ast = ast;
	r->fn.env = env;
	r->fn.binds = binds;
	return r;
}

obj invoke_form(obj form, obj env, obj *err)
{
	obj first = LIST_FIRST(form);
	obj args = form->list.rest;
	switch (first->type) {
		case BuiltinFn:
			return first->builtin_fn.fn(args, err);
		case Fn:
			env = new_env((unsigned) first->fn.binds->list.count,
							  first->fn.env);
			env_bind_args(env, first->fn.binds, args, err);
			if (*err) { return NULL; }
			return EVAL(first->fn.ast, env, err);
		case Keyword:
			return NULL; // todo
		case HashMap:
			return NULL; // todo
		default: {
			char fmt[] = "%s is not callable";
			obj msg = new_empty_string(sizeof(fmt) + 10);
			sprintf(msg->string.value, fmt, type_name_string(first));
			*err = new_error(msg);
			return NULL;
		}
	}
}

// ENV

obj new_env(unsigned size, obj outer)
{
	obj r = malloc(ALIGN_OBJ(sizeof(malp_env)));
	if (r == NULL) return r;
	r->type = Env;
	r->env.data_size = size;
	r->env.data = calloc(size, sizeof(malp_symtab_elem));
	r->env.outer = outer;
	return r;
}

