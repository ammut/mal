#include "types.h"
#include "math.h"

/**
 * https://rosettacode.org/wiki/Greatest_common_divisor#C
 */
malp_denom_t ugcd(malp_denom_t u, malp_denom_t v)
{
	if (v) { while ((u %= v) && (v %= u)) {} }
	return (u + v);
}

/**
 * https://rosettacode.org/wiki/Least_common_multiple#C
 */
malp_denom_t ulcm(malp_denom_t m, malp_denom_t n)
{
	return m / ugcd(m, n) * n;
}

#if 0
static void upcast_mutating(obj n, enum malp_type type)
{
	switch (type) {
		case Ratio:
			if (n->type == Int) {
				n->ratio.numerator = n->integer.value;
				n->ratio.denominator = 1;
				n->type = Ratio;
			}
			break;
		case Real:
			if (n->type == Int) {
				n->real.value = n->integer.value;
				n->type = Real;
			} else if (n->type == Ratio) {
				n->real.value =
					n->ratio.numerator / (malp_real_t) n->ratio.denominator;
				n->type = Real;
			}
			break;
	}
}
#endif

void add_mutating(obj l, obj r)
{
	switch (r->type) {
		case Int:
			switch (l->type) {
				case Int:
					l->integer.value += r->integer.value;
					break;
				case Ratio:
					l->ratio.numerator +=
						r->integer.value * l->ratio.denominator;
					NORMALIZE_RATIO(l->ratio);
					break;
				case Real:
					l->real.value += r->integer.value;
					break;
				default:
					return;
			}
			break;
		case Ratio:
			switch (l->type) {
				case Int:
					l->ratio.numerator =
						l->integer.value * r->ratio.denominator +
						r->ratio.numerator;
					l->ratio.denominator = r->ratio.denominator;
					l->type = Ratio;
					break;
				case Ratio: {
					malp_denom_t gcd = ugcd(l->ratio.denominator,
											r->ratio.denominator);
					l->ratio.numerator =
						l->ratio.numerator * r->ratio.denominator / gcd +
						r->ratio.numerator * l->ratio.denominator / gcd;
					l->ratio.denominator =
						l->ratio.denominator * r->ratio.denominator / gcd;
				}
					NORMALIZE_RATIO(l->ratio);
					break;
				case Real:
					l->real.value +=
						r->ratio.numerator / (malp_real_t) r->ratio.denominator;
					l->type = Real;
					break;
				default:
					return;
			}
			break;

		case Real:
			switch (l->type) {
				case Int:
					l->real.value = l->integer.value + r->real.value;
					l->type = Real;
					break;
				case Ratio:
					l->real.value =
						r->real.value + l->ratio.numerator /
										(malp_real_t) l->ratio.denominator;
					l->type = Real;
					break;
				case Real:
					l->real.value += r->real.value;
					break;
				default:
					return;
			}
			break;
		default:
			return;
	}
}

void subtract_mutating(obj l, obj r)
{
	switch (r->type) {
		case Int:
			switch (l->type) {
				case Int:
					l->integer.value -= r->integer.value;
					break;
				case Ratio:
					l->ratio.numerator -=
						r->integer.value * l->ratio.denominator;
					NORMALIZE_RATIO(l->ratio);
					break;
				case Real:
					l->real.value -= r->integer.value;
					break;
				default:
					return;
			}
			break;
		case Ratio:
			switch (l->type) {
				case Int:
					l->ratio.numerator =
						l->integer.value * r->ratio.denominator -
						r->ratio.numerator;
					l->ratio.denominator = r->ratio.denominator;
					l->type = Ratio;
					break;
				case Ratio: {
					malp_denom_t gcd = ugcd(l->ratio.denominator,
											r->ratio.denominator);
					l->ratio.numerator =
						l->ratio.numerator * r->ratio.denominator / gcd -
						r->ratio.numerator * l->ratio.denominator / gcd;
					l->ratio.denominator =
						l->ratio.denominator * r->ratio.denominator / gcd;
				}
					NORMALIZE_RATIO(l->ratio);
					break;
				case Real:
					l->real.value -=
						r->ratio.numerator / (malp_real_t) r->ratio.denominator;
					l->type = Real;
					break;
				default:
					return;
			}
			break;

		case Real:
			switch (l->type) {
				case Int:
					l->real.value = l->integer.value - r->real.value;
					l->type = Real;
					break;
				case Ratio:
					l->real.value = l->ratio.numerator /
									(malp_real_t) l->ratio.denominator -
									r->real.value;
					l->type = Real;
					break;
				case Real:
					l->real.value -= r->real.value;
					l->type = Real;
					break;
				default:
					return;
			}
			break;
		default:
			return;
	}
}

void multiply_mutating(obj l, obj r)
{
	switch (r->type) {
		case Int:
			switch (l->type) {
				case Int:
					l->integer.value *= r->integer.value;
					break;
				case Ratio:
					l->ratio.numerator *= r->integer.value;
					NORMALIZE_RATIO(l->ratio);
					break;
				case Real:
					l->real.value *= r->integer.value;
					break;
				default:
					return;
			}
			break;
		case Ratio:
			switch (l->type) {
				case Int:
					l->ratio.numerator =
						l->integer.value * r->ratio.numerator;
					l->ratio.denominator = r->ratio.denominator;
					l->type = Ratio;
					break;
				case Ratio:
					l->ratio.numerator *= r->ratio.numerator;
					l->ratio.denominator *= r->ratio.denominator;
					NORMALIZE_RATIO(l->ratio);
					break;
				case Real:
					l->real.value *=
						r->ratio.numerator / (malp_real_t) r->ratio.denominator;
					l->type = Real;
					break;
				default:
					return;
			}
			break;

		case Real:
			switch (l->type) {
				case Int:
					l->real.value = l->integer.value * r->real.value;
					l->type = Real;
					break;
				case Ratio:
					l->real.value =
						r->real.value * l->ratio.numerator /
						(malp_real_t) l->ratio.denominator;
					l->type = Real;
					break;
				case Real:
					l->real.value *= r->real.value;
					break;
				default:
					return;
			}
			break;
		default:
			return;
	}
}

void divide_mutating(obj l, obj r)
{
	switch (r->type) {
		case Int:
			switch (l->type) {
				case Int:
					if (0 == l->integer.value % r->integer.value) {
						l->integer.value /= r->integer.value;
						break;
					}
					l->ratio.numerator =
						r->integer.value < 0 ? -l->integer.value
											 : l->integer.value;
					l->ratio.denominator = ABS(r->integer.value);
					NORMALIZE_RATIO(l->ratio);
					l->type = Ratio;
					break;
				case Ratio:
					l->ratio.denominator *= ABS(r->integer.value);
					NORMALIZE_RATIO(l->ratio);
					break;
				case Real:
					l->real.value /= r->integer.value;
					break;
				default:
					return;
			}
			break;
		case Ratio:
			switch (l->type) {
				case Int:
					l->ratio.numerator =
						l->integer.value * r->ratio.denominator;
					l->ratio.denominator = ABS(r->ratio.numerator);
					if (r->ratio.numerator < 0) {
						l->ratio.numerator = -l->ratio.numerator;
					}
					NORMALIZE_RATIO(l->ratio);
					l->type = Ratio;
					break;
				case Ratio:
					l->ratio.numerator *= r->ratio.numerator < 0 ?
										  -r->ratio.denominator :
										  r->ratio.denominator;
					l->ratio.denominator *= ABS(r->ratio.numerator);
					NORMALIZE_RATIO(l->ratio);
					break;
				case Real:
					l->real.value /=
						r->ratio.numerator / (malp_real_t) r->ratio.denominator;
					l->type = Real;
					break;
				default:
					return;
			}
			break;

		case Real:
			switch (l->type) {
				case Int:
					l->real.value = l->integer.value / r->real.value;
					l->type = Real;
					break;
				case Ratio:
					l->real.value =
						r->real.value / l->ratio.numerator /
						(malp_real_t) l->ratio.denominator;
					l->type = Real;
					break;
				case Real:
					l->real.value /= r->real.value;
					break;
				default:
					return;
			}
			break;
		default:
			return;
	}
}
