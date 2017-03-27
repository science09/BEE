/* #include <iostream> */
#include <stdio.h>
#include "BEE.h"
#include "MEM.h"
#include <pthread.h>


#define LEN  2

void test_routine()
{
    BEE_Parser     *interpreter;
    FILE *fp;
    fp = fopen("test/test.bee", "r");
    if (fp == NULL) {
        fprintf(stderr, "%s not found.\n", "test/t.bee");
        exit(1);
    }
    interpreter = BEE_CreateParser();
    BEE_Compile(interpreter, fp);
    BEE_Parse(interpreter);
    BEE_DestroyParser(interpreter);
}


int main(int argc, char **argv)
{

    BEE_Parser     *interpreter;
    FILE *fp;
    pthread_t  tid[LEN];
    int ret;
/*
    if (argc != 2) {
        fprintf(stderr, "usage:%s filename", argv[0]);
        exit(1);
    }
*/
    for (int i = 0; i < LEN; ++i) {
        ret = pthread_create(&tid[i], NULL, (void *)test_routine, NULL);
        if (ret != 0)
        {
            perror("create thread error!");
            return -1;
        }
    }

    for (int i = 0; i < LEN; ++i) {
        pthread_join(tid[i], NULL);
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