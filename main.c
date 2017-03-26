/* #include <iostream> */
#include <stdio.h>
#include "BEE.h"
#include "MEM.h"


int main(int argc, char **argv)
{

    BEE_Parser     *interpreter;
    FILE *fp;

/*
    if (argc != 2) {
        fprintf(stderr, "usage:%s filename", argv[0]);
        exit(1);
    }
*/
    fp = fopen("test/test.bee", "r");
    if (fp == NULL) {
        fprintf(stderr, "%s not found.\n", "test/test.bee");
        exit(1);
    }
    interpreter = BEE_CreateParser();
    BEE_Compile(interpreter, fp);
    BEE_Parse(interpreter);
    BEE_DestroyParser(interpreter);

    MEM_dump_blocks(stdout);

    return 0;
}