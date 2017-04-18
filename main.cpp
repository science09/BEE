/* #include <iostream> */

#include <stdio.h>
#include "BEE.h"
#include "MEM.h"
#include <thread>



#define LEN  2

void test_routine()
{

    BEE_Parser     *parser;
    FILE *fp;
    parser = BEE_CreateParser();
    fp = fopen("test/t.bee", "r");
    if (fp == NULL) {
        fprintf(stderr, "%s not found.\n", "test/t.bee");
        exit(1);
    }

    BEE_Compile(parser, fp);
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
        th[i] = std::thread(test_routine);
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