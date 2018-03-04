cmake_minimum_required(VERSION 3.9)
project(mal)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wextra -Werror -pedantic")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g -lreadline -lpthread")

set(SOURCES
        reader.h reader.c
        printer.h printer.c
        env.h env.c
        core.h core.c
        math.h math.c
        types.h types.c
        )

add_executable(step1_eval step1_read_print.c ${SOURCES})
add_executable(step2_eval step2_eval.c ${SOURCES})
add_executable(step3_env step3_env.c ${SOURCES})

TARGET_LINK_LIBRARIES(step1_eval readline)
TARGET_LINK_LIBRARIES(step2_eval readline)
TARGET_LINK_LIBRARIES(step3_env readline)
