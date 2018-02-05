#ifndef _TYPES_H
#define _TYPES_H

#include <inttypes.h>

typedef enum {
	List, Number, Symbol, Nil, True, False, String, Keyword, Vector, HashMap, Atom
} MalType;

typedef struct sMalValue {
	uint8_t gc_mark;
	MalType type;
	struct sMalValue *gc_next;
	char value[];
} MalValue;

void destroy_mal_value(MalValue*);

// LIST

typedef struct {
	MalValue *first;
	MalValue *rest;
	size_t count;
} MalList;

MalValue *mal_list();

/*
 * returns a pointer to the newly inserted MalValue-list element.
 *
 * must be hooked into the gc
 */
MalValue *add_last_mutating(MalValue *head, MalValue *element);

MalValue empty_list;

MalValue *mal_list_from(MalValue *element);

// NUMBER

typedef enum {
	INT, FLOAT, NONE
} MalNumberType;

typedef int64_t MalIntValue;
typedef double MalFloatValue;

typedef struct {
	MalNumberType type;
	union {
		MalFloatValue FLOAT;
		MalIntValue INT;
	} value;
} MalNumber;

MalNumber parse_number(char *token);

MalValue *mal_number_float(MalFloatValue);
MalValue *mal_number_int(MalIntValue);
MalValue *mal_number_from(char *token);

void set_number_float(double value);
void set_number_int(int64_t value);

// SYMBOL

typedef char *MalSymbol;

MalValue *mal_symbol(char *token, size_t length);

// NIL

MalValue mal_nil;

int is_nil(MalValue*);

// TRUE

MalValue mal_true;

int is_true(MalValue*);

int truthy(MalValue*);

// FALSE

MalValue mal_false;

int is_false(MalValue*);

int falsey(MalValue*);

// STRING

typedef struct {
	size_t length;
	char string[];
} MalString;

MalValue *mal_string(char *token, size_t len);
MalValue *mal_string_from(char *token, size_t len);

#endif
