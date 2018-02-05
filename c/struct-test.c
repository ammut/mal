#include <stdio.h>

typedef struct A {
	struct A *next;
} A;

A nil = { .next = &nil };



int main() {
	int (*destroy)(const char*) = NULL;
	(destroy != NULL ? destroy : puts)("wat");

	printf("&nil: %p\nnil.next: %p\n", &nil, nil.next);

	char *fat_cow = "fat cow!";
	long long i = 0;
	char *c;

	long long *ptr = &i;
	*(char**)(ptr + 1) = fat_cow;

	printf("ptr: %p\n"
		   "ptr + 1: %p\n"
		   "&c: %p\n"
		   "*(char**)(ptr + 1): %s\n",
		   ptr,
		   ptr + 1,
		   &c,
		   *(char**)(ptr + 1));
}
