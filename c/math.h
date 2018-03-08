#include "types.h"

// https://stackoverflow.com/a/32008147/4427997
#define ABS(x) _Generic((x), \
    signed char: abs(x), short: abs(x), int: abs(x), long: labs(x), long long: llabs(x))

#define NORMALIZE_RATIO(r) \
do { \
    malp_denom_t gcd = ugcd(ABS((r).numerator), (r).denominator); \
    (r).numerator /= (malp_int_t)gcd; \
    (r).denominator /= gcd; \
} while(0);

malp_denom_t ugcd(malp_denom_t u, malp_denom_t v);

malp_denom_t ulcm(malp_denom_t M, malp_denom_t n);

void add_mutating(obj l, obj r);

void subtract_mutating(obj l, obj r);

void multiply_mutating(obj l, obj r);

void divide_mutating(obj l, obj r);
