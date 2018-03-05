#include <stdlib.h>

#include "types.h"
#include "math.h"
#include "env.h"
#include "core.h"

#define STRLEN_STATIC(s) (sizeof(s) - 1)

#define BUILTINS_SIZE 20

obj init_repl_env()
{
	obj env = new_env(BUILTINS_SIZE, NULL); // todo: malloc checking
	env_set(env, new_symbol("+", STRLEN_STATIC("+")), PLUS);
	env_set(env, new_symbol("-", STRLEN_STATIC("-")), MINUS);
	env_set(env, new_symbol("*", STRLEN_STATIC("*")), STAR);
	env_set(env, new_symbol("/", STRLEN_STATIC("/")), SLASH);
	env_set(env, new_symbol("number?", STRLEN_STATIC("number?")), numberQUESTION);
	return env;
}

DEF_BUILTIN_FN(numberQUESTION)(obj args, int *err)
{
	if (args->list.rest == empty_list) {
		*err = ArityError; // todo
		return NULL;
	}
	args = args->list.first;
	return is_number(args) ?
		true_o :
		false_o;
}

DEF_BUILTIN_FN(PLUS)(obj args, int *err)
{
	malp_number tmp = {
		.integer = {
			.type = Int,
			.value = 0
		}
	};
	obj res = (obj)&tmp, arg;
	while (args != empty_list) {
		arg = args->list.first;
		if (!is_number(arg)) {
			*err = InvalidArgumentError; // todo
			return NULL;
		}
		add_mutating(res, arg);

		args = args->list.rest;
	}
	switch (res->type) {
		case Int:
			return new_int(res->integer.value);
		case Ratio:
			return new_ratio(res->ratio.numerator, res->ratio.denominator);
		case Real:
			return new_real(res->real.value);
		default:
			return NULL;
	}
}

DEF_BUILTIN_FN(MINUS)(obj args, int *err)
{
	if (args == empty_list) {
		*err = ArityError; // todo
		return NULL;
	}

	malp_number tmp = {
		.integer = {
			.type = Int,
			.value = 0
		}
	};
	obj res = (obj)&tmp, arg;

	if (args->list.count > 1) {
		if (!is_number(args->list.first)) {
			*err = InvalidArgumentError; // todo
			return NULL;
		}
		tmp = *(malp_number*)args->list.first;
		args = args->list.rest;
	}

	while (args != empty_list) {
		arg = args->list.first;
		if (!is_number(arg)) {
			*err = InvalidArgumentError; // todo
			return NULL;
		}
		subtract_mutating(res, arg);

		args = args->list.rest;
	}
	switch (res->type) {
		case Int:
			return new_int(res->integer.value);
		case Ratio:
			return new_ratio(res->ratio.numerator, res->ratio.denominator);
		case Real:
			return new_real(res->real.value);
		default:
			return NULL;
	}
}

DEF_BUILTIN_FN(STAR)(obj args, int *err)
{
	malp_number tmp = {
		.integer = {
			.type = Int,
			.value = 1
		}
	};
	obj res = (obj)&tmp, arg;
	while (args != empty_list) {
		arg = args->list.first;
		if (!is_number(arg)) {
			*err = InvalidArgumentError; // todo
			return NULL;
		}
		multiply_mutating(res, arg);

		args = args->list.rest;
	}
	switch (res->type) {
		case Int:
			return new_int(res->integer.value);
		case Ratio:
			return new_ratio(res->ratio.numerator, res->ratio.denominator);
		case Real:
			return new_real(res->real.value);
		default:
			return NULL;
	}
}

DEF_BUILTIN_FN(SLASH)(obj args, int *err)
{
	if (args == empty_list) {
		*err = ArityError; // todo
		return NULL;
	}

	malp_number tmp = {
		.integer = {
			.type = Int,
			.value = 1
		}
	};
	obj res = (obj)&tmp, arg;

	if (args->list.count > 1) {
		if (!is_number(args->list.first)) {
			*err = InvalidArgumentError; // todo
			return NULL;
		}
		tmp = *(malp_number*)args->list.first;
		args = args->list.rest;
	}

	while (args != empty_list) {
		arg = args->list.first;
		if (!is_number(arg)) {
			*err = InvalidArgumentError; // todo
			return NULL;
		}
		divide_mutating(res, arg);

		args = args->list.rest;
	}
	switch (res->type) {
		case Int:
			return new_int(res->integer.value);
		case Ratio:
			return new_ratio(res->ratio.numerator, res->ratio.denominator);
		case Real:
			return new_real(res->real.value);
		default:
			return NULL;
	}
}
