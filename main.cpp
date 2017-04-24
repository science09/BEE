/* #include <iostream> */

#include <stdio.h>
#include "BEE.h"
#include "MEM.h"
#include <thread>
#include <mutex>


std::mutex g_mtx;

#define LEN  10

void test_routine()
{

    BEE_Parser     *parser;
    FILE *fp;
    g_mtx.lock();
    parser = BEE_CreateParser();
    fp = fopen("test/t.bee", "r");
    if (fp == NULL) {
        fprintf(stderr, "%s not found.\n", "test/t.bee");
        exit(1);
    }
    BEE_Compile(parser, fp);
    g_mtx.unlock();
    BEE_Parse(parser);

    BEE_DestroyParser(parser);
}

void test_string_routine()
{
    BEE_Parser *parser;
    char * expression = "print(\"hoge\\tpiyo\\n\\\\n\");\n"
            "print(\"abc\\n\"); # comment\n"
            "test(5);";
    g_mtx.lock();
    parser = BEE_CreateParser();
    BEE_CompileStr(parser, expression);
    g_mtx.unlock();
    BEE_Parse(parser);
    BEE_DestroyParser(parser);
}


int main(int argc, char **argv)
{
    BEE_Parser     *parser;
    FILE *fp;
    pthread_t  tid[LEN];
    int ret;
/*
    if (argc != 2) {
        fprintf(stderr, "usage:%s filename", argv[0]);
        exit(1);
    }
*/



    std::thread th[LEN];

    for (int i = 0; i < LEN; ++i) {
        th[i] = std::thread(test_string_routine);
    }

    for (int i = 0; i < LEN; ++i) {
        th[i].join();
    }





//    fp = fopen("test/test.bee", "r");
//    if (fp == NULL) {
//        fprintf(stderr, "%s not found.\n", "test/test.bee");
//        exit(1);
//    }
//    interpreter = BEE_CreateParser();
//    BEE_Compile(interpreter, fp);
//    BEE_Parse(interpreter);
//    BEE_DestroyParser(interpreter);



    MEM_dump_blocks(stdout);

    return 0;
}