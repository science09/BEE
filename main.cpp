/* #include <iostream> */

#include <stdio.h>
#include "BEE.h"
#include "MEM.h"
#include <thread>
#include <mutex>
#include <string.h>

std::mutex g_mtx;

#define LEN  1
#define INTERFACE 0

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
            "addOne(5, 6.1, \"msg\");";
    g_mtx.lock();
    parser = BEE_CreateParser();
    BEE_CompileStr(parser, "0x2<<3;");
    g_mtx.unlock();
    BEE_Parse(parser);
    BEE_DestroyParser(parser);
}

extern "C" int addOne(int a)
{
    return a+1;
}


int main(int argc, char **argv)
{
    BEE_Parser     *parser;
    pthread_t  tid[LEN];
    int ret;
    char buff[1024] = {0};

#if INTERFACE
    parser = BEE_CreateParser();
    fputs("bee->", stdout);
    while (nullptr != fgets(buff, 1024, stdin)) {
        if (0 == strcmp("exit\n", buff)) {
            break;
        }
        else if (0 == strcmp("\n", buff)) {
            fputs("bee->", stdout);
            continue;
        }

        size_t len = strlen(buff);
        buff[len-1] = '\n';
        BEE_CompileStr(parser, buff);
        BEE_Parse(parser);
        fputs("bee->", stdout);
    }

    BEE_DestroyParser(parser);
#else

//    std::thread th[LEN];
//
//    for (int i = 0; i < LEN; ++i) {
//        th[i] = std::thread(test_string_routine);
//    }
//
//    for (int i = 0; i < LEN; ++i) {
//        th[i].join();
//    }
//

    FILE *fp;
    char msgBuf[1024] = {0};
    fp = fopen("test/t.bee", "r");
    if (fp == NULL) {
        fprintf(stderr, "%s not found.\n", "test/t.bee");
        exit(1);
    }
    fread(msgBuf, 1, 1024, fp);
    parser = BEE_CreateParser();
//    BEE_Compile(parser, fp);
    BEE_CompileStr(parser, msgBuf);
    BEE_Parse(parser);
    BEE_DestroyParser(parser);

#endif

    MEM_dump_blocks(stdout);

    return 0;
}