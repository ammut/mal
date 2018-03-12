cmake_minimum_required(VERSION 3.9)
project(mal C)

set(CMAKE_C_STANDARD 11)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wextra -Werror -pedantic -O2")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wextra -Werror -pedantic")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g -lreadline -lpthread")

set(SOURCES
        reader.h reader.c
        readline.h readline.c
        printer.h printer.c
        main.h
        env.h env.c
        core.h core.c
        math.h math.c
        types.h types.c
        )

add_executable(step1_eval step1_read_print.c ${SOURCES} main.h)
add_executable(step2_eval step2_eval.c ${SOURCES})
add_executable(step3_env step3_env.c ${SOURCES})
add_executable(step4_if_fn_do step4_if_fn_do.c ${SOURCES})
add_executable(step5_tco step5_tco.c ${SOURCES})
add_executable(step6_file step6_file.c ${SOURCES})

TARGET_LINK_LIBRARIES(step1_eval readline)
TARGET_LINK_LIBRARIES(step2_eval readline)
TARGET_LINK_LIBRARIES(step3_env readline)
TARGET_LINK_LIBRARIES(step4_if_fn_do readline)
TARGET_LINK_LIBRARIES(step5_tco readline)
TARGET_LINK_LIBRARIES(step6_file readline)
