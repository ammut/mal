#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
enum wat {one, two, three, four, five, six, seven, eight, nine, ten, eleven};

struct value {
	enum wat type;
	int32_t value[];
};
struct bar {
	enum wat val_type;
	enum wat bar_type;
	union repr {
		double d1;
		double d2;
	} bar_value;
};
struct foo {
	size_t len;
	char string[];
};
int main() {
	char *str = malloc(4);
	strcpy(str, "1234");
	str[5] = 'x';
	struct bar *bar;
	struct value *value;
	printf("%lu\n", sizeof(struct value));
	printf("%lu\n", sizeof(struct bar));
	printf("%lu\n", sizeof(struct foo));
	printf("%p, %p, %p\n", value, &value->type, &value->value);
	printf("%p, %p, %p\n", bar, &bar->val_type, &bar->bar_type);


	return 0;
	char *skull = "\\u2620";
	char *uskull = "☠";
	char buf[10];
	char buf2[10];
	sprintf(buf, "%s", skull);
	sprintf(buf2, "%s", buf);
	printf("%s\n%s\n%s\n", skull, uskull, buf2);

	printf("%lu\n%lu\n%lu\n", strlen(skull), strlen(uskull), strlen(buf2));
}

