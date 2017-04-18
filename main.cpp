/* #include <iostream> */

#include <stdio.h>
#include "BEE.h"
#include "MEM.h"
#include <pthread.h>
#include <c++/5/thread>



#define LEN  2

void test_routine(BEE_Parser *parser)
{
    BEE_Parser     *interpreter;
    FILE *fp;
    fp = fopen("test/t.bee", "r");
    if (fp == NULL) {
        fprintf(stderr, "%s not found.\n", "test/t.bee");
        exit(1);
    }

    BEE_Compile(parser, fp);
    BEE_Parse(parser);

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
    parser = BEE_CreateParser();
    std::thread th[LEN];

//    for (int i = 0; i < LEN; ++i) {
//        ret = pthread_create(&tid[i], NULL, (void *)test_routine, (void*)parser);
//        if (ret != 0)
//        {
//            perror("create thread error!");
//            return -1;
//        }
//    }
//
//    for (int i = 0; i < LEN; ++i) {
//        pthread_join(tid[i], NULL);
//    }

    BEE_DestroyParser(parser);

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