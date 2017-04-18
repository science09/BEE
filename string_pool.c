//
// Created by hadoop on 17-3-22.
//

#include <stdio.h>
#include "MEM.h"
#include "DBG.h"
#include "bee_def.h"


static BEE_String *allocBeeString(BEE_Parser *parser, char *str, BEE_Boolean is_literal)
{
    BEE_String *ret;

    ret = (BEE_String *)MEM_malloc(sizeof(BEE_String));
    ret->ref_count = 0;
    ret->is_literal = is_literal;
    ret->string = str;

    return ret;
}

BEE_String *beeLiterToBeeString(BEE_Parser *parser, char *str)
{
    BEE_String *ret;

    ret = allocBeeString(parser, str, BEE_TRUE);
    ret->ref_count = 1;

    return ret;
}

void beeReferString(BEE_String *str)
{
    str->ref_count++;
}

void beeReleaseString(BEE_String *str)
{
    str->ref_count--;

    DBG_assert(str->ref_count >= 0, ("str->ref_count..%d\n", str->ref_count));
    if (str->ref_count == 0)
    {
        if (!str->is_literal)
        {
            MEM_free(str->string);
        }
        MEM_free(str);
    }
}

BEE_String *beeCreateBeeString(BEE_Parser *parser, char *str)
{
    BEE_String *ret = allocBeeString(parser, str, BEE_FALSE);
    ret->ref_count = 1;

    return ret;
}
