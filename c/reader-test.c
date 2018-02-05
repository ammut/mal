#include <stdio.h>
#include "reader.h"
#include "types.h"

int main(void) {
	if (42.0 == 42) {
		puts("42.0 == 42");
	}
	if (42.01 == 42) {
		puts("42.01 == 42");
	}

	MalNumber nt = try_make_number("42"),
		flt = try_make_number("42.01"),
		fail = try_make_number("wat"),
		zero = {0};
	printf("nt = {\n\tunion: \n\t\tFLOAT: %lf\n\t\tINT: %ld\n\ttype: %d\n}\n",
		   nt.value.FLOAT, nt.value.INT, nt.type);
	printf("flt = {\n\tunion: \n\t\tFLOAT: %lf\n\t\tINT: %ld\n\ttype: %d\n}\n",
		   flt.value.FLOAT, flt.value.INT, flt.type);
	printf("fail = {\n\tunion: \n\t\tFLOAT: %lf\n\t\tINT: %ld\n\ttype: %d\n}\n",
		   fail.value.FLOAT, fail.value.INT, fail.type);
	printf("zero = {\n\tunion: \n\t\tFLOAT: %lf\n\t\tINT: %ld\n\ttype: %d\n}\n",
		   zero.value.FLOAT, zero.value.INT, zero.type);
	return 0;



	char* in = "(defn fibonacci [n] (cond (zero? n) 0 (= 1 n) 1 :else (+ (fibonacci (- n 1)) (fibonacci (- n 2)))))";
	char **tokens = tokenize(in);
    Reader r = {.tokens = tokens, .position = 0};
    while(peek(&r)) {
        puts(next(&r));
    }
	read_str(in);
	return 0;
}
