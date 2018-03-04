#include "types.h"

malp_int_t ugcd(malp_denom_t u, malp_denom_t v);

malp_int_t ulcm(malp_denom_t M, malp_denom_t n);

void add_mutating(obj l, obj r);

void subtract_mutating(obj l, obj r);

void multiply_mutating(obj l, obj r);

void divide_mutating(obj l, obj r);
