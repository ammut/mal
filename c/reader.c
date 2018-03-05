#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "reader.h"
#include "types.h"
#include "core.h"

static obj read_form(reader_s*);
static obj read_list(reader_s*);
static obj read_atom(reader_s*);

static char **tokenize(char*);

static size_t get_token_type(char *in);
static size_t skip_whitespace(char *in);
static size_t tokenize_next(char* in, int *err);

enum token_type {
	rose_token, special_token, string_token,
	comment_token, number_token, other_token
};

static char* peek(reader_s *reader)
{
	return reader->tokens[reader->position];
}

static char* next(reader_s *reader)
{
	return reader->tokens[reader->position++];
}

obj read_str(char *in)
{
	char **tokens = tokenize(in);
	if (tokens == NULL) return (void *) tokens; // todo: crash horribly
	reader_s r = { .tokens = tokens, .position = 0 };
	obj v = read_form(&r);
	free (*tokens); free(tokens);
	return v;
}

static obj read_form(reader_s *r)
{
	obj v;
	if ('(' == peek(r)[0]) v = read_list(r);
	else v = read_atom(r);
	if (NULL == v) return v;
	return v;
}

static obj read_list(reader_s *r)
{
	(void)next(r);
	if (peek(r)[0] == ')') return empty_list;

	obj head = NULL;
	obj tail = head;
	unsigned count = 0;

	while (peek(r) && peek(r)[0] != ')') { // read_form$read_atom will call next(r)
		obj el = read_form(r);
		if (!el) goto read_list_err;
		tail = append_mutating(tail, el);
		++count;
		if (!tail) goto read_list_err;
		if (!head) head = tail;
	}
	if (NULL == next(r)) goto read_list_err;
	set_length_mutating(head, count);
	return head;

 read_list_err:
	return NULL;
}

static obj read_atom(reader_s *r)
{
	char *token = next(r);
	size_t len = strlen(token);

	// todo: rebuild the lexer

	switch (get_token_type(token)) {
		case rose_token:
			return NULL;
		case special_token:
			return NULL;
		case string_token:
			return read_string(token, len);
		case comment_token:
			return NULL;
		case number_token:
			return read_number(token);
		case other_token:
			if (strcmp(token, "true") == 0) return true_o;
			if (strcmp(token, "false") == 0) return false_o;
			if (strcmp(token, "nil") == 0) return nil_o;
			return new_symbol(token, len);
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
static char **tokenize(char* in)
{
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
	int err;

	while(1) {
		in_pos += skip_whitespace(in + in_pos);
		if (in_pos == in_len) break; // EOI
		found = tokenize_next(in + in_pos, &err);
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


static int is_ws(char c)
{
	return 9 == c || 10 == c || 11 == c || 12 == c||
		  13 == c || ' ' == c || ',' == c;
}

static int is_other(char c)
{
	return !is_ws(c) && '[' != c && ']' != c && '{' != c && '}' != c &&
		   '(' != c && ')' != c && '\'' != c && '"' != c && '`' != c &&
		   ',' != c && ';' != c && '\0' != c;
}

static size_t skip_whitespace(char *in)
{
	size_t r = 0;
	char c = in[r];
	while(is_ws(c)) {
		c = in[++r];
	}
	return r;
}

#if 0

static size_t tokenize_rose(char *in) {
	if (in[0] == '~' && in[1] == '@') {
		return 2;
	}
	return 0;
}

static size_t tokenize_special(char *in) {
	char c = in[0];
	if ('[' == c || ']' == c || '{' == c || '}' == c || '(' == c || ')' == c ||
		'\'' == c || '`' == c || '~' == c || '@' == c || '^' == c) {
		return 1;
	}
	return 0;
}

static size_t tokenize_string(char *in) {
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

static size_t tokenize_comment(char *in) {
	size_t pos = 0;
	if (in[pos] == ';') while (in[++pos] != '\n' && in[pos] != '\0');
	return pos;
}

static size_t tokenize_other(char *in) {
	size_t pos = 0;
	char c = in[pos];
	while (!is_ws(c) && '[' != c && ']' != c && '{' != c && '}' != c &&
		   '(' != c && ')' != c && '\'' != c && '"' != c && '`' != c &&
		   ',' != c && ';' != c && '\0' != c) {
		c = in[++pos];
	}
	return pos;
}

#endif

static size_t tokenize_next(char* in, int *err)
{
	size_t pos = 0;
	switch (in[0]) {
		case '~':
			if (in[1] == '@') {
				pos = 2;
				goto tokenize_next_exit;
			} else if (in[1] == '\0') {
				pos = 1;
				goto tokenize_next_exit;
			}
		case '[':
		case ']':
		case '{':
		case '}':
		case '(':
		case ')':
		case '\'':
		case '`':
		case '^':
		case '@':
			pos = 1;
			goto tokenize_next_exit;

		case '"':
			pos = 1;
			goto tokenize_next_string;

		case ';':
			pos = 1;
			goto tokenize_next_comment;

		default:
			goto tokenize_next_other;
	}
tokenize_next_exit:
	return pos;

tokenize_next_string:
	switch(in[pos]) {
		case '\0':
			*err = ReaderError; // todo: actual error codes
			goto tokenize_next_exit;
		case '"':
			++pos;
			goto tokenize_next_exit;
		case '\\':
			++pos;
			goto tokenize_next_string_esc;
		default:
			++pos;
			goto tokenize_next_string;
	}
tokenize_next_string_esc:
	switch(in[pos]) {
		case '\0':
			*err = ReaderError; // todo: actual error codes
			goto tokenize_next_exit;
		default:
			++pos;
			goto tokenize_next_string;
	}

tokenize_next_comment:
	while (in[pos] != '\n' && in[pos] != '\0') ++pos;
	goto tokenize_next_exit;

tokenize_next_other:
	{
		int num_len;
		malp_int_t i;
		malp_real_t r;
		if (sscanf(in, "%" PRId64 "%n", &i, &num_len)) {
			if (!is_other(in[num_len])) {
				pos = (size_t)num_len;
				goto tokenize_next_exit;
			}
			int denom_len;
			if (in[num_len] == '/' &&
				sscanf(in , "%" PRId64 "%n", &i, &denom_len) &&
				!is_other(in[num_len + 1 + denom_len])) {

				if (i == 0) {
					*err = ReaderError; // todo: actual error code
				}
				pos = (size_t)num_len + 1 + denom_len;
				goto tokenize_next_exit;
			}
		}
		if (sscanf(in, "%lf%n", &r, &num_len)) {

			pos = (size_t)num_len;
			if (is_other(in[num_len])) {
				*err = ReaderError; // todo: actual error codes
			}
			goto tokenize_next_exit;
		}
	}
	char c = in[pos];
	while (is_other(c)) {
		c = in[++pos];
	}
	goto tokenize_next_exit;
}

static size_t get_token_type(char* in)
{
	switch (in[0]) {
		case '~':
			if (in[1] == '@') {
				return rose_token;
			}
		case '[':
		case ']':
		case '{':
		case '}':
		case '(':
		case ')':
		case '\'':
		case '`':
		case '^':
		case '@':
			return special_token;

		case '"':
			return string_token;

		case ';':
			return comment_token;

		default:
			break;
	}
	if ((in[0] == '-' && in[1] != 0) || (in[0] >= '0' && in[0] <= '9'))
		return number_token;
	return other_token;
}

