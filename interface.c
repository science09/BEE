//
// Created by hadoop on 17-3-22.
//

#include <stdio.h>
#include "MEM.h"
#include "DBG.h"
#include "bee_def.h"

#include "y.tab.h"
#include "lex.yy.h"

static void addBuiltinFunctions(BEE_Parser *parser)
{
    BEE_AddBuiltinFunction(parser, "print", beeBuiltinPrintFunc);
    BEE_AddBuiltinFunction(parser, "fopen", beeBuiltinFopenFunc);
    BEE_AddBuiltinFunction(parser, "fclose", beeBuiltinFcloseFunc);
    BEE_AddBuiltinFunction(parser, "fgets", beeBuiltinFgetsFunc);
    BEE_AddBuiltinFunction(parser, "fputs", beeBuiltinFputsFunc);
}

BEE_Parser * BEE_CreateParser(void)
{
    MEM_Storage  storage;
    BEE_Parser *parser;

    storage = MEM_open_storage(0);
    parser = MEM_storage_malloc(storage, sizeof(BEE_Parser));
    parser->parser_storage = storage;
    parser->execute_storage = NULL;
    parser->variable = NULL;
    parser->function_list = NULL;
    parser->statement_list = NULL;
    parser->current_line_number = 1;

    beeSetCurrentParser(parser);
    addBuiltinFunctions(parser);

    return parser;
}

void BEE_Compile(BEE_Parser *parser, FILE *fp)
{
//    extern int yyparse(void);
//    extern FILE *yyin;
    yyscan_t scanner;
    beeSetCurrentParser(parser);
//    yyin = fp;
    yylex_init(&scanner);
    yyset_in(fp, scanner);
    if (yyparse(scanner)) {
        /* BUGBUG */
        fprintf(stderr, "Error ! Error ! Error !\n");
        exit(1);
    }
    beeResetStringLiteralBuffer();
    yylex_destroy(scanner);
}

void BEE_Parse(BEE_Parser *parser)
{
    parser->execute_storage = MEM_open_storage(0);
    beeAddStdFp(parser);
    beeExecuteStatementList(parser, NULL, parser->statement_list);
}

static void releaseGlobalStrings(BEE_Parser *parser) {
    while (parser->variable)
    {
        Variable *temp = parser->variable;
        parser->variable = temp->next;
        if (temp->value.type == BEE_STRING_VALUE)
        {
            beeReleaseString(temp->value.u.string_value);
        }
    }
}

void BEE_DestroyParser(BEE_Parser *parser)
{
    releaseGlobalStrings(parser);

    if (parser->execute_storage)
    {
        MEM_dispose_storage(parser->execute_storage);
    }

    MEM_dispose_storage(parser->parser_storage);
}

void BEE_AddBuiltinFunction(BEE_Parser *parser, char *name, BEE_NativeFunctionProc *proc)
{
    FunctionDefinition *fd;

    fd = beeMalloc(sizeof(FunctionDefinition));
    fd->name = name;
    fd->type = BUILTIN_FUNCTION_DEFINITION;
    fd->u.native_f.proc = proc;
    fd->next = parser->function_list;

    parser->function_list = fd;
}
