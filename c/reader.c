#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "reader.h"
#include "types.h"
#include "gc.h"

static size_t whitespace(char*);
static size_t rose(char*);
static size_t special(char*);
static size_t string(char*);
static size_t comment(char*);
static size_t other(char*);

static char **tokenize(char*);

static MalValue *read_form(Reader*, GcRoot*);
static MalValue *read_list(Reader*, GcRoot*);
static MalValue *read_atom(Reader*);

static char* peek(Reader *reader) {
	return reader->tokens[reader->position];
}

static char* next(Reader *reader) {
	return reader->tokens[reader->position++];
}

MalValue *read_str(char *in, GcRoot *env) {
	char **tokens = tokenize(in);
	if (tokens == NULL) return (void *) tokens; // todo
	Reader r = { .tokens = tokens, .position = 0 };
	gc_pause(env);
	MalValue *v = read_form(&r, env);
	free (*tokens); free(tokens);
	gc_resume(env);
	return v;
}

static MalValue *read_form(Reader *r, GcRoot *gc) {
	MalValue *v;
	if ('(' == peek(r)[0]) v = read_list(r, gc);
	else v = read_atom(r);
	if (NULL == v) return v;
	gc_set_root(v, gc);
	return v;
}

static MalValue *read_list(Reader *r, GcRoot *gc) {
	(void)next(r);
	if (peek(r)[0] == ')') return &empty_list;
	MalValue *initial = read_form(r, gc);
	MalValue *head = mal_list_from(initial);
	if (NULL == head || NULL == initial) goto read_list_err;
	while (peek(r) && peek(r)[0] != ')') { // read_form$read_atom will call next(r)
		MalValue *e_l = read_form(r, gc);
		if (!e_l) goto read_list_err;
		e_l = add_last_mutating(head, e_l);
		if (!e_l) goto read_list_err;
		gc_set_root(e_l, gc);
	}
	if (NULL == next(r)) goto read_list_err; 
	return head;

 read_list_err:
	if (head != NULL) destroy_mal_value(head);
	return NULL;
}

static MalValue *read_atom(Reader *r) {
	char *token = next(r);
	size_t len = strlen(token);
	MalNumber num;

	// todo: rebuild the lexer

	num = parse_number(token);
	if (num.type != NONE) return mal_number_from(token);
	if (rose(token)) return NULL;
	if (special(token)) return NULL;
	if (string(token)) {
		return mal_string_from(token, len);
	}
	if (comment(token)) return NULL;
	if (other(token)) {
		if (strcmp(token, "true") == 0) return &mal_true;
		if (strcmp(token, "false") == 0) return &mal_false;
		if (strcmp(token, "nil") == 0) return &mal_nil;
		return mal_symbol(token, len);
	}
	return NULL;
}

/**
 * returns a list of strings, which must be freed when done with it by first
 * freeing the first element of the list and then the list itself, i.e.:
 *
 * ```c
 * char **tokens = tokenize(in);
 * ...
 * free(*tokens); free(tokens);
 */
static char **tokenize(char* in) {
	size_t in_len = strlen(in);

	size_t buff_pos = 0;
	// 2x len for 0-terminated strings
	char *token_buff = malloc(in_len * 2 * sizeof *token_buff);
	if (NULL == token_buff) goto tokenize_err;

	size_t tokens_pos = 0;
	// len + 1 for 0-terminator
	char **tokens = calloc(in_len + 1, sizeof *tokens);
	if (NULL == tokens) goto tokenize_err;

	size_t in_pos = 0;
	size_t found = 0;

	while(1) {
		in_pos += whitespace(in + in_pos);
		if (in_pos == in_len) break; 
		found = rose(in + in_pos);
		if (!found) found = special(in + in_pos);
		if (!found) found = string(in + in_pos);
		if (!found) found = comment(in + in_pos);
		if (!found) found = other(in + in_pos); // this *should* match the rest
		assert(found);
		
		tokens[tokens_pos++] = token_buff + buff_pos;
		while(found--) token_buff[buff_pos++] = in[in_pos++];
		token_buff[buff_pos++] = '\0';
	}		

	return tokens;

	tokenize_err:
	if (token_buff) free(token_buff);
	if (tokens) free(tokens);
	return NULL;
}


static int is_ws(char c) {
	return 9 == c || 10 == c || 11 == c || 12 == c||
		  13 == c || ' ' == c || ',' == c;
}

static size_t whitespace(char *in) {
	size_t r = 0;
	char c = in[r];
	while(is_ws(c)) {
		c = in[++r];
	}
	return r;
}

static size_t rose(char *in) {
	if (in[0] == '~' && in[1] == '@') {
		return 2;
	}
	return 0;
}

static size_t special(char *in) {
	char c = in[0];
	if ('[' == c || ']' == c || '{' == c || '}' == c || '(' == c || ')' == c ||
		'\'' == c || '`' == c || '~' == c || '@' == c || '^' == c) {
		return 1;
	}
	return 0;
}

static size_t string(char *in) {
	size_t pos = 0;
	switch(in[pos]) {
		case '"':
			++pos; goto string_string;
		case '\0':
		default:
			goto string_err;
	}
string_string:
	switch(in[pos]) {
		case '\0':
			goto string_err;
		case '\\':
			++pos; goto string_esc;
		case '"':
			++pos; goto string_end;
		default:
			++pos; goto string_string;
	}
string_esc:
	switch(in[pos]) {
		case '\0':
			goto string_err;
		default:
			++pos; goto string_string;
	}
string_end:
	return pos;
string_err:
	return 0;
}

static size_t comment(char *in) {
	size_t pos = 0;
	if (in[pos] == ';') while (in[++pos] != '\n' && in[pos] != '\0');
	return pos;
}

static size_t other(char *in) {
	size_t pos = 0;
	char c = in[pos];
	while (!is_ws(c) && '[' != c && ']' != c && '{' != c && '}' != c &&
		   '(' != c && ')' != c && '\'' != c && '"' != c && '`' != c &&
		   ',' != c && ';' != c && '\0' != c) {
		c = in[++pos];
	}
	return pos;
}

#if 0
static size_t lex(char *in) {
	return 0;
}
#endif
