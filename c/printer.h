#ifndef _PRINTER_H
#define _PRINTER_H

#include "types.h"

#define PRSTRALL_READABLY 0x1
#define PRSTRALL_SPACING 0x2
#define PRSTRALL_BRACKETS 0x4

void print_string(obj string, int newline);

obj pr_str(obj ast, int print_readably);

/**
 * Use PRSTRALL_READABLY, PRSTRALL_SPACING, PRSTRALL_BRACKETS
 * to configure. brackets have to be set at call site.
 *
 * @param _buf
 * @param ast
 * @param flags
 */
obj pr_str_all(obj ast, int flags);

#endif
