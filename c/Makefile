CFLAGS += -g -Wall -Wextra -Werror -pedantic 
LDFLAGS += -g

#####################


SOURCES_BASE = reader.c printer.c types.c gc.c
SOURCES_LISP = #env.c core.h core.c stepA_mal.c
SOURCES = $(SOURCES_BASE) $(SOURCES_LISP)
HEADERS = $(SOURCES:%.c=%.h);

OBJECTS = $(SOURCES:%.c=%.o)

#####################

STEPS = step0_repl.c step1_read_print.c step2_eval.c step3_env.c \
       step4_if_fn_do.c step5_tco.c step6_file.c step7_quote.c \
       step8_macros.c step9_try.c
STEPO = $(STEPS:%.c=%.o)
STEPB = $(STEPO:%.o=%)

LIBRARIES = readline pthread

LDFLAGS += $(LIBRARIES:%=-l%)

#####################



$(OBJECTS) $(STEPO): %.o: %.c $(HEADERS)
	gcc $(CFLAGS) -c $(@:%.o=%.c) -o $@

$(STEPB): %: %.o $(OBJECTS)
	gcc $+ $(LDFLAGS) -o $@

clean:
	rm -f $(OBJECTS) $(STEPB) $(STEPO)

.PHONY: stats stats-lisp tests $(TESTS)

####################

stats: $(SOURCES)
	@wc $^
	@printf "%5s %5s %5s %s\n" `grep -E "^[[:space:]]*//|^[[:space:]]*$$" $^ | wc` "[comments/blanks]"
stats-lisp: $(SOURCES_LISP)
	@wc $^
	@printf "%5s %5s %5s %s\n" `grep -E "^[[:space:]]*//|^[[:space:]]*$$" $^ | wc` "[comments/blanks]"

tests: $(TESTS)

$(TESTS):
	@echo "Running $@"; \
	./$@ || exit 1; \
