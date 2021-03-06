#ifndef _ENV_H
#define _ENV_H

#include <stdlib.h>

#include "types.h"

void env_set(obj env, obj symbol, obj value);

obj env_find(obj env, obj symbol);

obj env_get(obj env, obj symbol);

void env_bind_args(obj env, obj binds, obj exprs, obj *err);

#endif
