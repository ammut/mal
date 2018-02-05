#ifndef _READER_H
#define _READER_H

#include <string.h>

#include "types.h"
#include "gc.h"

typedef struct {
	char **tokens;
	unsigned position;
} Reader;

// char **tokenize(char* in);
// char *peek(Reader *);
// char *next(Reader *);

MalValue *read_str(char* in, GcRoot *env);


#endif
