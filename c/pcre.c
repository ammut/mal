#define PCRE2_CODE_UNIT_WIDTH 8

#include <stdio.h>
#include <string.h>
#include <pcre2.h>

int main(int argc, char **argv)
{
	PCRE2_SPTR pattern = "\\s*(s[^ ]*s)";
	if (argc != 2) {
		fputs("nothing to search\n", stderr);
		return 1;
	}
	PCRE2_SPTR subject = argv[1];
	int subject_len = strlen(subject);

	int errno;
	int erroffset;
	pcre2_code *re = pcre2_compile(
			pattern,
			PCRE2_ZERO_TERMINATED,
			0,
			&errno,
			(size_t*)&erroffset,
			NULL
			);

	pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(re, NULL);
	PCRE2_SIZE *out_vector = pcre2_get_ovector_pointer(match_data);

	while(1) {
		PCRE2_SIZE starting_offset = out_vector[1];
		int rc = pcre2_match(
			re,
			subject,
			subject_len,
			starting_offset,//starting_offset,
			0,
			match_data,
			NULL
			);
		if (rc == PCRE2_ERROR_NOMATCH) break;
		else if (rc < 0) return 1;
		PCRE2_UCHAR **capture_list;
		(void)pcre2_substring_list_get(match_data, &capture_list, NULL);
		for (int i = 0; i < rc; ++i) {
			printf("%2d: %.*s\n", i, (int) (out_vector[2 * i + 1] - out_vector[2 * i]), (char *) (capture_list[i]));
		}
		pcre2_substring_list_free(capture_list);
	}
}
