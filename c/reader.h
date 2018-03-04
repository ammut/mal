#ifndef _READER_H
#define _READER_H

#include <string.h>

#include "types.h"

typedef struct reader {
	char **tokens;
	unsigned position;
} reader_s;

obj read_str(char* in);

#endif
