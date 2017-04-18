cmake_minimum_required(VERSION 3.6)
project(Bee)

#make VERBOSE=1

set(CMAKE_CXX_STANDARD 11)

#SET(PARSER_DIR ${CMAKE_SOURCE_DIR}/driver/mysql  ${CMAKE_CURRENT_BINARY_DIR})
MESSAGE(STATUS "DIR====" ${CMAKE_SOURCE_DIR})
message(status "DIR***" ${CMAKE_SOURCE_DIR})


add_custom_command(
        COMMAND bison --yacc -dv ${CMAKE_SOURCE_DIR}/bee.y
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/y.tab.c ${CMAKE_CURRENT_BINARY_DIR}/y.tab.h
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_custom_command(
        COMMAND flex ${CMAKE_SOURCE_DIR}/bee.l
#        TARGET GenServerSource
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

#include_directories(memory debug)

set(SOURCE_FILES
        main.cpp
        MEM.h
        DBG.h
        BEE.h
        BEE_dev.h
        bee_def.h
        util.c
        string.c
        string_pool.c
        error_message.c
        interface.c
        create.c
        eval.c
        builtin.c
        execute.c
        lex.yy.c
        y.tab.c
        memory/memory.h memory/memory.c memory/storage.c debug/debug.h test.cpp)
add_executable(Bee ${SOURCE_FILES})

target_link_libraries(Bee m)