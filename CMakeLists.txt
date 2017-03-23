cmake_minimum_required(VERSION 3.6)
project(Bee)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp MEM.h DBG.h BEE.h BEE_dev.h bee_def.h util.c string.c string_pool.c error_message.c interface.c create.c eval.c builtin.c execute.c)
add_executable(Bee ${SOURCE_FILES})