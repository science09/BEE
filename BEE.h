//
// Created by hadoop on 17-3-21.
//

#ifndef BEE_BEE_H
#define BEE_BEE_H

#include <stdio.h>

typedef struct BEE_Parser_tag BEE_Parser;

BEE_Parser *BEE_CreateParser(void);
void BEE_Compile(BEE_Parser *parser, FILE *fp);
void BEE_CompileStr(BEE_Parser *parser, char *expression);
void BEE_Parse(BEE_Parser *parser);
void BEE_DestroyParser(BEE_Parser *parser);

#endif //BEE_BEE_H
