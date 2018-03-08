#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "math.h"
#include "env.h"
#include "core.h"
#include "printer.h"

#define RET_ARITY_ERROR_IF(cond, err) \
	if (cond) { \
		(err) = ArityError; \
		return NULL; \
	}

void core_load_vars(obj env)
{
	env_set(env, new_symbol("+", STRLEN_STATIC("+")), malp_core_PLUS);
	env_set(env, new_symbol("-", STRLEN_STATIC("-")), malp_core_MINUS);
	env_set(env, new_symbol("*", STRLEN_STATIC("*")), malp_core_STAR);
	env_set(env, new_symbol("/", STRLEN_STATIC("/")), malp_core_SLASH);
	env_set(env, new_symbol("number?", STRLEN_STATIC("number?")), malp_core_numberQUESTION);
	env_set(env, new_symbol("pr-str", STRLEN_STATIC("pr-str")), malp_core_prMINUSstr);
	env_set(env, new_symbol("str", STRLEN_STATIC("str")), malp_core_str);
	env_set(env, new_symbol("prn", STRLEN_STATIC("prn")), malp_core_prn);
	env_set(env, new_symbol("print", STRLEN_STATIC("print")), malp_core_print);
	env_set(env, new_symbol("println", STRLEN_STATIC("println")), malp_core_println);
	env_set(env, new_symbol("list", STRLEN_STATIC("list")), malp_core_list);
	env_set(env, new_symbol("list?", STRLEN_STATIC("list?")), malp_core_listQUESTION);
	env_set(env, new_symbol("empty?", STRLEN_STATIC("empty?")), malp_core_emptyQUESTION);
	env_set(env, new_symbol("count", STRLEN_STATIC("count")), malp_core_count);
	env_set(env, new_symbol("=", STRLEN_STATIC("=")), malp_core_EQUAL);
	env_set(env, new_symbol("<", STRLEN_STATIC("<")), malp_core_LT);
	env_set(env, new_symbol("<=", STRLEN_STATIC("<=")), malp_core_LE);
	env_set(env, new_symbol(">", STRLEN_STATIC(">")), malp_core_GT);
	env_set(env, new_symbol(">=", STRLEN_STATIC(">=")), malp_core_GE);
}

DEF_BUILTIN_FN(malp_core_numberQUESTION)(obj args, int *err)
{
	if (args == empty_list) {
		*err = ArityError; // todo
		return NULL;
	}
	args = LIST_FIRST(args);
	return is_number(args) ?
		true_o :
		false_o;
}

DEF_BUILTIN_FN(malp_core_PLUS)(obj args, int *err)
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

DEF_BUILTIN_FN(malp_core_MINUS)(obj args, int *err)
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

DEF_BUILTIN_FN(malp_core_STAR)(obj args, int *err)
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

DEF_BUILTIN_FN(malp_core_SLASH)(obj args, int *err)
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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
DEF_BUILTIN_FN(malp_core_prMINUSstr)(obj args, int *err)
{
	return pr_str_all(args, PRSTRALL_READABLY | PRSTRALL_SPACING);
}

DEF_BUILTIN_FN(malp_core_str)(obj args, int *err)
{
	return pr_str_all(args, 0);
}

DEF_BUILTIN_FN(malp_core_prn)(obj args, int *err)
{
	print_string(pr_str_all(args, PRSTRALL_READABLY | PRSTRALL_SPACING), 1);
	return nil_o;
}

DEF_BUILTIN_FN(malp_core_print)(obj args, int *err)
{
	print_string(pr_str_all(args, PRSTRALL_SPACING), 0);
	return nil_o;
}

DEF_BUILTIN_FN(malp_core_println)(obj args, int *err)
{
	print_string(pr_str_all(args, PRSTRALL_SPACING), 1);
	return nil_o;
}

DEF_BUILTIN_FN(malp_core_list)(obj args, int *err)
{
	return args;
}
#pragma GCC diagnostic pop

DEF_BUILTIN_FN(malp_core_listQUESTION)(obj args, int *err)
{
	RET_ARITY_ERROR_IF(args->list.count != 1, *err);
	if (List == LIST_FIRST(args)->type) {
		return true_o;
	}
	return false_o;
}

DEF_BUILTIN_FN(malp_core_emptyQUESTION)(obj args, int *err)
{
	RET_ARITY_ERROR_IF(args->list.count != 1, *err);
	if (List != LIST_FIRST(args)->type) {
		*err = InvalidArgumentError;
		return NULL;
	}
	if (empty_list == LIST_FIRST(args)) {
		return true_o;
	}
	return false_o;
}

DEF_BUILTIN_FN(malp_core_count)(obj args, int *err)
{
	RET_ARITY_ERROR_IF(args->list.count != 1, *err);
	if (LIST_FIRST(args) == nil_o) return new_int(0);
	if (List != LIST_FIRST(args)->type) {
		*err = InvalidArgumentError;
		return NULL;
	}
	return new_int((malp_int_t)LIST_FIRST(args)->list.count);
}

DEF_BUILTIN_FN(malp_core_EQUAL)(obj args, int *err)
{
	RET_ARITY_ERROR_IF(args == empty_list, *err); // todo
	if (args->list.count == 1) return true_o;

	obj first = LIST_FIRST(args);
	obj next;
	while ((args = args->list.rest) != empty_list) {
		next = LIST_FIRST(args);

		if (first == next) continue;

		if (IS_COLLECTION(first) && IS_COLLECTION(next)) {
			if (COUNT(first) != COUNT(next)) return false_o;

			while (first != empty_list) {
				obj left = LIST_FIRST(first);
				obj right = LIST_FIRST(next);

				if (false_o == malp_core_EQUAL_(cons(cons(empty_list, left), right) , err))
					return false_o;

				first = first->list.rest;
				next = next->list.rest;
			}

			continue;
		}

		if (first->type != next->type) return false_o;

		switch (first->type) {
			case Int:
				if (first->integer.value != next->integer.value)
					return false_o;
				break;
			case Ratio:
				if (first->ratio.numerator != next->ratio.numerator ||
					first->ratio.denominator != next->ratio.denominator)
					return false_o;
				break;
			case Real:
				if (first->real.value != next->real.value)
					return false_o;
				break;
			case Keyword:
			case Symbol:
				if (strcmp(first->symbol.name, next->symbol.name) != 0)
					return false_o;
				break;
			case String:
				if (first->string.length != next->string.length)
					return false_o;
				if (0 != strncmp(first->string.value,
								 next->string.value,
								 first->string.length))
					return false_o;
				break;
			case BuiltinFn:
			case Fn:
			case Error:
			case Atom:
			default:
				return false_o;
		}
	}

	return true_o;
}

DEF_BUILTIN_FN(malp_core_LT)(obj args, int *err)
{
	RET_ARITY_ERROR_IF(empty_list == args, *err);

	obj first = LIST_FIRST(args);
	if (!is_number(first)) {
		*err = InvalidArgumentError;
		return NULL;
	}

	while (empty_list != (args = args->list.rest)) {
		obj next = LIST_FIRST(args);
		if (!is_number(next)) {
			*err = InvalidArgumentError;
			return NULL;
		}
		if (NUMBER_VALUE(first) >= NUMBER_VALUE(next)) {
			return false_o;
		}
		first = next;
	}
	return true_o;
}

DEF_BUILTIN_FN(malp_core_LE)(obj args, int *err)
{
	RET_ARITY_ERROR_IF(empty_list == args, *err);

	obj first = LIST_FIRST(args);
	if (!is_number(first)) {
		*err = InvalidArgumentError;
		return NULL;
	}

	while (empty_list != (args = args->list.rest)) {
		obj next = LIST_FIRST(args);
		if (!is_number(next)) {
			*err = InvalidArgumentError;
			return NULL;
		}
		if (NUMBER_VALUE(first) > NUMBER_VALUE(next)) {
			return false_o;
		}
		first = next;
	}
	return true_o;
}

DEF_BUILTIN_FN(malp_core_GT)(obj args, int *err)
{
	RET_ARITY_ERROR_IF(empty_list == args, *err);

	obj first = LIST_FIRST(args);
	if (!is_number(first)) {
		*err = InvalidArgumentError;
		return NULL;
	}

	while (empty_list != (args = args->list.rest)) {
		obj next = LIST_FIRST(args);
		if (!is_number(next)) {
			*err = InvalidArgumentError;
			return NULL;
		}
		if (NUMBER_VALUE(first) <= NUMBER_VALUE(next)) {
			return false_o;
		}
		first = next;
	}
	return true_o;
}

DEF_BUILTIN_FN(malp_core_GE)(obj args, int *err)
{
	RET_ARITY_ERROR_IF(empty_list == args, *err);

	obj first = LIST_FIRST(args);
	if (!is_number(first)) {
		*err = InvalidArgumentError;
		return NULL;
	}

	while (empty_list != (args = args->list.rest)) {
		obj next = LIST_FIRST(args);
		if (!is_number(next)) {
			*err = InvalidArgumentError;
			return NULL;
		}
		if (NUMBER_VALUE(first) < NUMBER_VALUE(next)) {
			return false_o;
		}
		first = next;
	}
	return true_o;
}

