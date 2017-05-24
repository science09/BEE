//
// Created by hadoop on 17-3-23.
//

#include <math.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "bee_def.h"
#include "BEE_dev.h"


static StatementResult executeStatement(BEE_Parser *parser, LocalEnvironment *env, Statement *statement);

static StatementResult executeExpressionStatement(BEE_Parser *parser,
                                                  LocalEnvironment *env,
                                                  Statement *statement)
{
    StatementResult result;
    BEE_Value v;

    result.type = NORMAL_STATEMENT_RESULT;

    v = beeEvalExpression(parser, env, statement->u.expression_s);
    if (v.type == BEE_STRING_VALUE)
    {
        beeReleaseString(v.u.string_value);
    }
    result.u.return_value = v;

    return result;
}

static StatementResult executeGlobalStatement(BEE_Parser *parser, LocalEnvironment *env,
                         Statement *statement)
{
    IdentifierList *pos;
    StatementResult result;

    result.type = NORMAL_STATEMENT_RESULT;

    if (env == NULL)
    {
        beeRuntimeError(statement->line_number,
                          GLOBAL_STATEMENT_IN_TOPLEVEL_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    for (pos = statement->u.global_s.identifier_list; pos; pos = pos->next)
    {
        GlobalVariableRef *ref_pos;
        GlobalVariableRef *new_ref;
        Variable *variable;
        for (ref_pos = env->global_variable; ref_pos;
             ref_pos = ref_pos->next)
        {
            if (!strcmp(ref_pos->variable->name, pos->name))
                goto NEXT_IDENTIFIER;
        }
        variable = beeSearchGlobalVariable(parser, pos->name);
        if (variable == NULL)
        {
            beeRuntimeError(statement->line_number,
                              GLOBAL_VARIABLE_NOT_FOUND_ERR,
                              STRING_MESSAGE_ARGUMENT, "name", pos->name,
                              MESSAGE_ARGUMENT_END);
        }
        new_ref = (GlobalVariableRef *)MEM_malloc(sizeof(GlobalVariableRef));
        new_ref->variable = variable;
        new_ref->next = env->global_variable;
        env->global_variable = new_ref;
        NEXT_IDENTIFIER:
        ;
    }

    return result;
}

static StatementResult executeElseIf(BEE_Parser *parser, LocalEnvironment *env,
              Elsif *elsif_list, BEE_Boolean *executed)
{
    StatementResult result;
    BEE_Value   cond;
    Elsif *pos;

    *executed = BEE_FALSE;
    result.type = NORMAL_STATEMENT_RESULT;
    for (pos = elsif_list; pos; pos = pos->next)
    {
        cond = beeEvalExpression(parser, env, pos->condition);
        if (cond.type != BEE_BOOLEAN_VALUE)
        {
            beeRuntimeError(pos->condition->line_number, NOT_BOOLEAN_TYPE_ERR, MESSAGE_ARGUMENT_END);
        }
        if (cond.u.boolean_value)
        {
            result = beeExecuteStatementList(parser, env, pos->block->statement_list);
            *executed = BEE_TRUE;
            if (result.type != NORMAL_STATEMENT_RESULT)
                goto FUNC_END;
        }
    }

    FUNC_END:
    return result;
}

static StatementResult executeIfStatement(BEE_Parser *parser, LocalEnvironment *env,
                     Statement *statement)
{
    StatementResult result;
    BEE_Value   cond;

    result.type = NORMAL_STATEMENT_RESULT;
    cond = beeEvalExpression(parser, env, statement->u.if_s.condition);
    if (cond.type != BEE_BOOLEAN_VALUE)
    {
        beeRuntimeError(statement->u.if_s.condition->line_number,
                          NOT_BOOLEAN_TYPE_ERR, MESSAGE_ARGUMENT_END);
    }
    DBG_assert(cond.type == BEE_BOOLEAN_VALUE, ("cond.type..%d", cond.type));

    if (cond.u.boolean_value)
    {
        result = beeExecuteStatementList(parser, env,
                                            statement->u.if_s.then_block->statement_list);
    }
    else
    {
        BEE_Boolean elsif_executed;
        result = executeElseIf(parser, env, statement->u.if_s.elsif_list, &elsif_executed);
        if (result.type != NORMAL_STATEMENT_RESULT)
            goto FUNC_END;
        if (!elsif_executed && statement->u.if_s.else_block)
        {
            result = beeExecuteStatementList(parser, env,
                                               statement->u.if_s.else_block->statement_list);
        }
    }

    FUNC_END:
    return result;
}

static StatementResult executeWhileStatement(BEE_Parser *parser, LocalEnvironment *env,
                        Statement *statement)
{
    StatementResult result;
    BEE_Value   cond;

    result.type = NORMAL_STATEMENT_RESULT;
    for (;;)
    {
        cond = beeEvalExpression(parser, env, statement->u.while_s.condition);
        if (cond.type != BEE_BOOLEAN_VALUE)
        {
            beeRuntimeError(statement->u.while_s.condition->line_number,
                              NOT_BOOLEAN_TYPE_ERR, MESSAGE_ARGUMENT_END);
        }
        DBG_assert(cond.type == BEE_BOOLEAN_VALUE,
                   ("cond.type..%d", cond.type));
        if (!cond.u.boolean_value)
            break;

        result = beeExecuteStatementList(parser, env,
                                         statement->u.while_s.block->statement_list);
        if (result.type == RETURN_STATEMENT_RESULT)
        {
            break;
        }
        else if (result.type == BREAK_STATEMENT_RESULT)
        {
            result.type = NORMAL_STATEMENT_RESULT;
            break;
        }
    }

    return result;
}

static StatementResult executeForStatement(BEE_Parser *parser, LocalEnvironment *env,
                      Statement *statement)
{
    StatementResult result;
    BEE_Value   cond;

    result.type = NORMAL_STATEMENT_RESULT;

    if (statement->u.for_s.init)
    {
        beeEvalExpression(parser, env, statement->u.for_s.init);
    }
    for (;;)
    {
        if (statement->u.for_s.condition)
        {
            cond = beeEvalExpression(parser, env, statement->u.for_s.condition);
            if (cond.type != BEE_BOOLEAN_VALUE)
            {
                beeRuntimeError(statement->u.for_s.condition->line_number,
                                  NOT_BOOLEAN_TYPE_ERR, MESSAGE_ARGUMENT_END);
            }
            DBG_assert(cond.type == BEE_BOOLEAN_VALUE,
                       ("cond.type..%d", cond.type));
            if (!cond.u.boolean_value)
                break;
        }
        result = beeExecuteStatementList(parser, env,
                                         statement->u.for_s.block->statement_list);
        if (result.type == RETURN_STATEMENT_RESULT)
        {
            break;
        }
        else if (result.type == BREAK_STATEMENT_RESULT)
        {
            result.type = NORMAL_STATEMENT_RESULT;
            break;
        }

        if (statement->u.for_s.post)
        {
            beeEvalExpression(parser, env, statement->u.for_s.post);
        }
    }

    return result;
}

static StatementResult executeReturnStatement(BEE_Parser *parser, LocalEnvironment *env,
                         Statement *statement)
{
    StatementResult result;

    result.type = RETURN_STATEMENT_RESULT;
    if (statement->u.return_s.return_value)
    {
        result.u.return_value = beeEvalExpression(parser, env, statement->u.return_s.return_value);
    }
    else
    {
        result.u.return_value.type = BEE_NULL_VALUE;
    }

    return result;
}

static StatementResult executeBreakStatement(BEE_Parser *parser, LocalEnvironment *env,
                        Statement *statement)
{
    StatementResult result;

    result.type = BREAK_STATEMENT_RESULT;

    return result;
}

static StatementResult executeContinueStatement(BEE_Parser *parser, LocalEnvironment *env,
                           Statement *statement)
{
    StatementResult result;

    result.type = CONTINUE_STATEMENT_RESULT;

    return result;
}

static StatementResult executeStatement(BEE_Parser *parser, LocalEnvironment *env, Statement *statement)
{
    StatementResult result;
    BEE_Value rtnValue;

    result.type = NORMAL_STATEMENT_RESULT;

    switch (statement->type)
    {
        case EXPRESSION_STATEMENT:
            result = executeExpressionStatement(parser, env, statement);
            break;
        case GLOBAL_STATEMENT:
            result = executeGlobalStatement(parser, env, statement);
            break;
        case IF_STATEMENT:
            result = executeIfStatement(parser, env, statement);
            break;
        case WHILE_STATEMENT:
            result = executeWhileStatement(parser, env, statement);
            break;
        case FOR_STATEMENT:
            result = executeForStatement(parser, env, statement);
            break;
        case RETURN_STATEMENT:
            result = executeReturnStatement(parser, env, statement);
            break;
        case BREAK_STATEMENT:
            result = executeBreakStatement(parser, env, statement);
            break;
        case CONTINUE_STATEMENT:
            result = executeContinueStatement(parser, env, statement);
            break;
        case STATEMENT_TYPE_COUNT_PLUS_1:   /* FALLTHRU */
        default:
            DBG_panic(("bad case...%d", statement->type));
    }

    if(parser->debugMode)
    {
        if (result.type == NORMAL_STATEMENT_RESULT)
        {
            rtnValue = result.u.return_value;
            switch(rtnValue.type)
            {
                case BEE_BOOLEAN_VALUE:
                    printf("value = %s\n", (rtnValue.u.boolean_value == 1) ? "true" : "false");
                    break;
                case BEE_LONG_VALUE:
                    printf("value = %ld = 0x%lx\n", rtnValue.u.long_value, rtnValue.u.long_value);
                    break;
                case BEE_DOUBLE_VALUE:
                    printf("value = %lf\n", rtnValue.u.double_value);
                    break;
                case BEE_STRING_VALUE:
                    printf("value = %s\n", rtnValue.u.string_value->string);
                    break;
                case BEE_NULL_VALUE:
                    break;
                default:
//                printf("unimplemented type(%d)!\n", rtnValue.type);
                    break;
            }
        }
    }

    return result;
}

StatementResult beeExecuteStatementList(BEE_Parser *parser, LocalEnvironment *env, StatementList *list)
{
    StatementList *pos;
    StatementResult result;

    result.type = NORMAL_STATEMENT_RESULT;
    for (pos = list; pos; pos = pos->next)
    {
        result = executeStatement(parser, env, pos->statement);
        if (result.type != NORMAL_STATEMENT_RESULT)
            goto FUNC_END;
    }

    FUNC_END:
    return result;
}
