//
// Created by hadoop on 17-3-23.
//

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "MEM.h"
#include "DBG.h"
#include "BEE_dev.h"
#include "bee_def.h"

#define NATIVE_LIB_NAME "crowbar.lang.file"


static BEE_NativePointerInfo st_native_lib_info = {
        NATIVE_LIB_NAME
};

BEE_Value beeBuiltinPrintFunc(BEE_Parser *parser, int arg_count, BEE_Value *args)
{
    BEE_Value value;

    value.type = BEE_NULL_VALUE;

    if (arg_count < 1)
    {
        beeRuntimeError(0, ARGUMENT_TOO_FEW_ERR, MESSAGE_ARGUMENT_END);
    }
    else if (arg_count > 1)
    {
        beeRuntimeError(0, ARGUMENT_TOO_MANY_ERR, MESSAGE_ARGUMENT_END);
    }
    switch (args[0].type)
    {
        case BEE_BOOLEAN_VALUE:
            if (args[0].u.boolean_value)
            {
                printf("true");
            }
            else
            {
                printf("false");
            }
            break;
        case BEE_LONG_VALUE:
            printf("%ld", args[0].u.long_value);
            break;
        case BEE_DOUBLE_VALUE:
            printf("%f", args[0].u.double_value);
            break;
        case BEE_STRING_VALUE:
            printf("%s", args[0].u.string_value->string);
            break;
        case BEE_NATIVE_POINTER_VALUE:
            printf("(%s:%p)",
                   args[0].u.native_pointer.info->name,
                   args[0].u.native_pointer.pointer);
            break;
        case BEE_NULL_VALUE:
            printf("null");
            break;
    }

    return value;
}

BEE_Value beeBuiltinFopenFunc(BEE_Parser *parser, int arg_count, BEE_Value *args)
{
    BEE_Value value;
    FILE *fp;

    if (arg_count < 2)
    {
        beeRuntimeError(0, ARGUMENT_TOO_FEW_ERR, MESSAGE_ARGUMENT_END);
    }
    else if (arg_count > 2)
    {
        beeRuntimeError(0, ARGUMENT_TOO_MANY_ERR, MESSAGE_ARGUMENT_END);
    }
    if (args[0].type != BEE_STRING_VALUE || args[1].type != BEE_STRING_VALUE)
    {
        beeRuntimeError(0, FOPEN_ARGUMENT_TYPE_ERR, MESSAGE_ARGUMENT_END);
    }

    fp = fopen(args[0].u.string_value->string,
               args[1].u.string_value->string);
    if (fp == NULL)
    {
        value.type = BEE_NULL_VALUE;
    }
    else
    {
        value.type = BEE_NATIVE_POINTER_VALUE;
        value.u.native_pointer.info = &st_native_lib_info;
        value.u.native_pointer.pointer = fp;
    }

    return value;
}

static BEE_Boolean check_native_pointer(BEE_Value *value)
{
    return (BEE_Boolean)(value->u.native_pointer.info == &st_native_lib_info);
}

BEE_Value beeBuiltinFcloseFunc(BEE_Parser *parser, int arg_count, BEE_Value *args)
{
    BEE_Value value;
    FILE *fp;

    value.type = BEE_NULL_VALUE;
    if (arg_count < 1)
    {
        beeRuntimeError(0, ARGUMENT_TOO_FEW_ERR, MESSAGE_ARGUMENT_END);
    }
    else if (arg_count > 1)
    {
        beeRuntimeError(0, ARGUMENT_TOO_MANY_ERR, MESSAGE_ARGUMENT_END);
    }
    if (args[0].type != BEE_NATIVE_POINTER_VALUE
        || !check_native_pointer(&args[0]))
    {
        beeRuntimeError(0, FCLOSE_ARGUMENT_TYPE_ERR, MESSAGE_ARGUMENT_END);
    }
    fp = (FILE *)args[0].u.native_pointer.pointer;
    fclose(fp);

    return value;
}

BEE_Value beeBuiltinFgetsFunc(BEE_Parser *parser, int arg_count, BEE_Value *args)
{
    BEE_Value value;
    FILE *fp;
    char buf[LINE_BUF_SIZE];
    char *ret_buf = NULL;
    size_t ret_len = 0;

    if (arg_count < 1)
    {
        beeRuntimeError(0, ARGUMENT_TOO_FEW_ERR, MESSAGE_ARGUMENT_END);
    }
    else if (arg_count > 1)
    {
        beeRuntimeError(0, ARGUMENT_TOO_MANY_ERR, MESSAGE_ARGUMENT_END);
    }
    if (args[0].type != BEE_NATIVE_POINTER_VALUE
        || !check_native_pointer(&args[0]))
    {
        beeRuntimeError(0, FGETS_ARGUMENT_TYPE_ERR, MESSAGE_ARGUMENT_END);
    }
    fp = (FILE *)args[0].u.native_pointer.pointer;

    while (fgets(buf, LINE_BUF_SIZE, fp))
    {
        size_t new_len;
        new_len = ret_len + strlen(buf);
        ret_buf = (char *)MEM_realloc(ret_buf, new_len + 1);
        if (ret_len == 0)
        {
            strcpy(ret_buf, buf);
        }
        else
        {
            strcat(ret_buf, buf);
        }
        ret_len = new_len;
        if (ret_buf[ret_len-1] == '\n')
            break;
    }
    if (ret_len > 0)
    {
        value.type = BEE_STRING_VALUE;
        value.u.string_value = beeCreateBeeString(parser, ret_buf);
    }
    else
    {
        value.type = BEE_NULL_VALUE;
    }

    return value;
}

BEE_Value beeBuiltinFputsFunc(BEE_Parser *parser, int arg_count, BEE_Value *args)
{
    BEE_Value value;
    FILE *fp;

    value.type = BEE_NULL_VALUE;
    if (arg_count < 2)
    {
        beeRuntimeError(0, ARGUMENT_TOO_FEW_ERR, MESSAGE_ARGUMENT_END);
    }
    else if (arg_count > 2)
    {
        beeRuntimeError(0, ARGUMENT_TOO_MANY_ERR, MESSAGE_ARGUMENT_END);
    }
    if (args[0].type != BEE_STRING_VALUE
        || (args[1].type != BEE_NATIVE_POINTER_VALUE
            || !check_native_pointer(&args[1])))
    {
        beeRuntimeError(0, FPUTS_ARGUMENT_TYPE_ERR, MESSAGE_ARGUMENT_END);
    }
    fp = (FILE *)args[1].u.native_pointer.pointer;

    fputs(args[0].u.string_value->string, fp);

    return value;
}

BEE_Value beeBuiltinTestFunc(BEE_Parser *parser, int arg_count, BEE_Value *args)
{
    printf("arg_count: %d\n", arg_count);
    sleep(3);
    printf("args[0]: %ld\n", args[0].u.long_value);
}

void beeAddStdFp(BEE_Parser *parser)
{
    BEE_Value fp_value;

    fp_value.type = BEE_NATIVE_POINTER_VALUE;
    fp_value.u.native_pointer.info = &st_native_lib_info;

    fp_value.u.native_pointer.pointer = stdin;
    BEE_AddGlobalVariable(parser, "STDIN", &fp_value);

    fp_value.u.native_pointer.pointer = stdout;
    BEE_AddGlobalVariable(parser, "STDOUT", &fp_value);

    fp_value.u.native_pointer.pointer = stderr;
    BEE_AddGlobalVariable(parser, "STDERR", &fp_value);
}
