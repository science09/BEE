//
// Created by hadoop on 17-3-21.
//

#include <stdio.h>
#include <string.h>
//#include <map>
//#include <thread>
#include "MEM.h"
#include "DBG.h"
#include "bee_def.h"

static BEE_Parser *st_current_parser;

//static std::map<std::thread::id, BEE_Parser *> st_current_parser;


BEE_Parser * beeGetCurrentParser(void)
{
//    return st_current_parser[std::this_thread::get_id()];
    return st_current_parser;
}


void beeSetCurrentParser(BEE_Parser *parser)
{
//    st_current_parser[std::this_thread::get_id()] = parser;
    st_current_parser = parser;
}


FunctionDefinition * beeSearchFunction(char *name)
{
    FunctionDefinition *pos;
    BEE_Parser *parser;

    parser = beeGetCurrentParser();
    for (pos = parser->function_list; pos; pos = pos->next)
    {
        if (!strcmp(pos->name, name))
            break;
    }
    return pos;
}


void * beeMalloc(size_t size)
{
    void *pVoid;
    BEE_Parser *parser;

    parser = beeGetCurrentParser();
    pVoid = MEM_storage_malloc(parser->parser_storage, size);

    return pVoid;
}

void * beeExecuteMalloc(BEE_Parser *parser, size_t size)
{
    void *pVoid;

    pVoid = MEM_storage_malloc(parser->parser_storage, size);

    return pVoid;
}

Variable * beeSearchLocalVariable(LocalEnvironment *env, char *identifier)
{
    Variable *pos;
    if (env == NULL)
    {
        return NULL;
    }
    for (pos = env->variable; pos; pos = pos->next)
    {
        if (!strcmp(pos->name, identifier))
            return pos;
    }

    return NULL;
}

Variable * beeSearchGlobalVariable(BEE_Parser *parser, char *identifier)
{
    Variable *pos;
    for (pos = parser->variable; pos; pos = pos->next)
    {
        if (!strcmp(pos->name, identifier))
            return pos;
    }
    return NULL;
}

void beeAddLocalVariable(LocalEnvironment *env, char *identifier, BEE_Value *value)
{
    Variable    *new_variable;

    new_variable = (Variable *)MEM_malloc(sizeof(Variable));
    new_variable->name = identifier;
    new_variable->value = *value;
    new_variable->next = env->variable;
    env->variable = new_variable;
}

void BEE_AddGlobalVariable(BEE_Parser *parser, char *identifier, BEE_Value *value)
{
    Variable    *new_variable;

    new_variable = (Variable *)beeExecuteMalloc(parser, sizeof(Variable));
    new_variable->name = (char *)beeExecuteMalloc(parser, strlen(identifier) + 1);
    strcpy(new_variable->name, identifier);
    new_variable->next = parser->variable;
    parser->variable = new_variable;
    new_variable->value = *value;
}

char * beeGetOperatorString(ExpressionType type)
{
    char        *str;

    switch (type) {
        case BOOLEAN_EXPRESSION:    /* FALLTHRU */
        case INT_EXPRESSION:        /* FALLTHRU */
        case DOUBLE_EXPRESSION:     /* FALLTHRU */
        case STRING_EXPRESSION:     /* FALLTHRU */
        case IDENTIFIER_EXPRESSION:
            DBG_panic(("bad expression type..%d\n", type));
            break;
        case ASSIGN_EXPRESSION:
            str = "=";
            break;
        case ADD_EXPRESSION:
            str = "+";
            break;
        case SUB_EXPRESSION:
            str = "-";
            break;
        case MUL_EXPRESSION:
            str = "*";
            break;
        case DIV_EXPRESSION:
            str = "/";
            break;
        case MOD_EXPRESSION:
            str = "%";
            break;
        case LOGICAL_AND_EXPRESSION:
            str = "&&";
            break;
        case LOGICAL_OR_EXPRESSION:
            str = "||";
            break;
        case EQ_EXPRESSION:
            str = "==";
            break;
        case NE_EXPRESSION:
            str = "!=";
            break;
        case GT_EXPRESSION:
            str = "<";
            break;
        case GE_EXPRESSION:
            str = "<=";
            break;
        case LT_EXPRESSION:
            str = ">";
            break;
        case LE_EXPRESSION:
            str = ">=";
            break;
        case MINUS_EXPRESSION:
            str = "-";
            break;
        case FUNCTION_CALL_EXPRESSION:  /* FALLTHRU */
        case NULL_EXPRESSION:  /* FALLTHRU */
        case EXPRESSION_TYPE_COUNT_PLUS_1:
        default:
            DBG_panic(("bad expression type..%d\n", type));
    }

    return str;
}
