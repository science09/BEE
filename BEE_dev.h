//
// Created by hadoop on 17-3-21.
//

#ifndef BEE_BEE_DEV_H
#define BEE_BEE_DEV_H

#include "BEE.h"


typedef enum {
    BEE_FALSE = 0,
    BEE_TRUE = 1
} BEE_Boolean;

typedef struct BEE_String_tag  BEE_String;

typedef struct {
    char        *name;
} BEE_NativePointerInfo;

typedef enum {
    BEE_BOOLEAN_VALUE = 1,
    BEE_LONG_VALUE,
    BEE_DOUBLE_VALUE,
    BEE_STRING_VALUE,
    BEE_NATIVE_POINTER_VALUE,
    BEE_NULL_VALUE
} BEE_ValueType;

typedef struct {
    BEE_NativePointerInfo       *info;
    void                        *pointer;
} BEE_NativePointer;


typedef struct {
    BEE_ValueType       type;
    union {
        BEE_Boolean         boolean_value;
        long                long_value;
        double              double_value;
        BEE_String          *string_value;
        BEE_NativePointer   native_pointer;
    } u;
} BEE_Value;


typedef BEE_Value BEE_NativeFunctionProc(BEE_Parser *parser, int arg_count, BEE_Value *args);

void BEE_AddBuiltinFunction(BEE_Parser *parser, char *name, BEE_NativeFunctionProc *proc);
void BEE_AddGlobalVariable(BEE_Parser *inter, char *identifier, BEE_Value *value);

#endif //BEE_BEE_DEV_H
