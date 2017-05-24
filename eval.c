//
// Created by hadoop on 17-3-22.
//

#include <math.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "bee_def.h"
#include "BEE_dev.h"


static BEE_Value evalBooleanExpression(BEE_Boolean boolean_value)
{
    BEE_Value   v;

    v.type = BEE_BOOLEAN_VALUE;
    v.u.boolean_value = boolean_value;

    return v;
}

static BEE_Value evalLongExpression(int long_value)
{
    BEE_Value   v;

    v.type = BEE_LONG_VALUE;
    v.u.long_value = long_value;

    return v;
}

static BEE_Value evalDoubleExpression(double double_value)
{
    BEE_Value   v;

    v.type = BEE_DOUBLE_VALUE;
    v.u.double_value = double_value;

    return v;
}

static BEE_Value evalStringExpression(BEE_Parser *parser, char *string_value)
{
    BEE_Value   v;

    v.type = BEE_STRING_VALUE;
    v.u.string_value = beeLiterToBeeString(parser, string_value);

    return v;
}

static BEE_Value evalNullExpression(void)
{
    BEE_Value   v;

    v.type = BEE_NULL_VALUE;

    return v;
}

static void referIfString(BEE_Value *v)
{
    if (v->type == BEE_STRING_VALUE)
    {
        beeReferString(v->u.string_value);
    }
}

static void releaseIfString(BEE_Value *v)
{
    if (v->type == BEE_STRING_VALUE)
    {
        beeReleaseString(v->u.string_value);
    }
}

static Variable * searchGlobalVariableFromEnv(BEE_Parser *parser, LocalEnvironment *env, char *name)
{
    GlobalVariableRef *pos;

    if (env == NULL) {
        return beeSearchGlobalVariable(parser, name);
    }

    for (pos = env->global_variable; pos; pos = pos->next) {
        if (!strcmp(pos->variable->name, name)) {
            return pos->variable;
        }
    }

    return NULL;
}

static BEE_Value evalIdentifierExpression(BEE_Parser *parser, LocalEnvironment *env, Expression *expr)
{
    BEE_Value   v;
    Variable    *vp;

    vp = beeSearchLocalVariable(env, expr->u.identifier);
    if (vp != NULL)
    {
        v = vp->value;
    }
    else
    {
        vp = searchGlobalVariableFromEnv(parser, env, expr->u.identifier);
        if (vp != NULL)
        {
            v = vp->value;
        }
        else 
        {
            beeRuntimeError(expr->line_number, VARIABLE_NOT_FOUND_ERR,
                              STRING_MESSAGE_ARGUMENT,
                              "name", expr->u.identifier,
                              MESSAGE_ARGUMENT_END);
        }
    }
    referIfString(&v);

    return v;
}

static BEE_Value evalExpression(BEE_Parser *parser, LocalEnvironment *env, Expression *expr);

static BEE_Value evalAssignExpression(BEE_Parser *parser, LocalEnvironment *env,
                       char *identifier, Expression *expression)
{
    BEE_Value   v;
    Variable    *left;

    v = evalExpression(parser, env, expression);

    left = beeSearchLocalVariable(env, identifier);
    if (left == NULL)
    {
        left = searchGlobalVariableFromEnv(parser, env, identifier);
    }
    if (left != NULL)
    {
        releaseIfString(&left->value);
        left->value = v;
        referIfString(&v);
    }
    else
    {
        if (env != NULL)
        {
            beeAddLocalVariable(env, identifier, &v);
        }
        else
        {
            BEE_AddGlobalVariable(parser, identifier, &v);
        }
        referIfString(&v);
    }

    return v;
}

static BEE_Boolean evalBinaryBoolean(BEE_Parser *parser, ExpressionType expressType,
                    BEE_Boolean left, BEE_Boolean right, int line_number)
{
    BEE_Boolean result;

    if (expressType == EQ_EXPRESSION) 
    {
        result = (BEE_Boolean)(left == right);
    }
    else if (expressType == NE_EXPRESSION) 
    {
        result = (BEE_Boolean)(left != right);
    } 
    else
    {
        char *op_str = beeGetOperatorString(expressType);
        beeRuntimeError(line_number, NOT_BOOLEAN_OPERATOR_ERR,
                          STRING_MESSAGE_ARGUMENT, "expressType", op_str,
                          MESSAGE_ARGUMENT_END);
    }

    return result;
}

static void evalBinaryInt(BEE_Parser *parser, ExpressionType expressType,
                int left, int right,
                BEE_Value *result, int line_number)
{
    if (dkc_is_math_operator(expressType))
    {
        result->type = BEE_LONG_VALUE;
    }
    else if (dkc_is_compare_operator(expressType))
    {
        result->type = BEE_BOOLEAN_VALUE;
    }
    else if (dkc_is_logical_operator(expressType))
    {
        result->type = BEE_BOOLEAN_VALUE;
    }
    else
    {
        DBG_panic(("expressType..%d\n", expressType));
    }

    switch (expressType)
    {
        case BOOLEAN_EXPRESSION:    /* FALLTHRU */
        case INT_EXPRESSION:        /* FALLTHRU */
        case DOUBLE_EXPRESSION:     /* FALLTHRU */
        case STRING_EXPRESSION:     /* FALLTHRU */
        case IDENTIFIER_EXPRESSION: /* FALLTHRU */
        case ASSIGN_EXPRESSION:
            DBG_panic(("bad case...%d", expressType));
            break;
        case ADD_EXPRESSION:
            result->u.long_value = left + right;
            break;
        case SUB_EXPRESSION:
            result->u.long_value = left - right;
            break;
        case MUL_EXPRESSION:
            result->u.long_value = left * right;
            break;
        case DIV_EXPRESSION:
            result->u.long_value = left / right;
            break;
        case MOD_EXPRESSION:
            result->u.long_value = left % right;
            break;
        case LSHIFT_EXPRESSION:
            result->u.long_value = left << right;
            break;
        case RSHIFT_EXPRESSION:
            result->u.long_value = left >> right;
            break;
        case BIT_AND_EXPRESSION:
            result->u.long_value = left & right;
            break;
        case BIT_XOR_EXPRESSION:
            result->u.long_value = left ^ right;
            break;
        case BIT_OR_EXPRESSION:
            result->u.long_value = left | right;
            break;
        case LOGICAL_AND_EXPRESSION:
            result->u.boolean_value = (BEE_Boolean)(left && right);
            break;
        case LOGICAL_OR_EXPRESSION:
            result->u.boolean_value = (BEE_Boolean)(left || right);
            break;
        case EQ_EXPRESSION:
            result->u.boolean_value = (BEE_Boolean)(left == right);
            break;
        case NE_EXPRESSION:
            result->u.boolean_value = (BEE_Boolean)(left != right);
            break;
        case GT_EXPRESSION:
            result->u.boolean_value = (BEE_Boolean)(left > right);
            break;
        case GE_EXPRESSION:
            result->u.boolean_value = (BEE_Boolean)(left >= right);
            break;
        case LT_EXPRESSION:
            result->u.boolean_value = (BEE_Boolean)(left < right);
            break;
        case LE_EXPRESSION:
            result->u.boolean_value = (BEE_Boolean)(left <= right);
            break;
        case MINUS_EXPRESSION:              /* FALLTHRU */
        case FUNCTION_CALL_EXPRESSION:      /* FALLTHRU */
        case NULL_EXPRESSION:               /* FALLTHRU */
        case EXPRESSION_TYPE_COUNT_PLUS_1:  /* FALLTHRU */
        default:
            DBG_panic(("bad case...%d", expressType));
    }
}

static void evalBinaryDouble(BEE_Parser *parser, ExpressionType expressType,
                   double left, double right,
                   BEE_Value *result, int line_number)
{
    if (dkc_is_math_operator(expressType))
    {
        result->type = BEE_DOUBLE_VALUE;
    }
    else if (dkc_is_compare_operator(expressType))
    {
        result->type = BEE_BOOLEAN_VALUE;
    }
    else
    {
        DBG_panic(("expressType..%d\n", expressType));
    }

    switch (expressType)
    {
        case BOOLEAN_EXPRESSION:    /* FALLTHRU */
        case INT_EXPRESSION:        /* FALLTHRU */
        case DOUBLE_EXPRESSION:     /* FALLTHRU */
        case STRING_EXPRESSION:     /* FALLTHRU */
        case IDENTIFIER_EXPRESSION: /* FALLTHRU */
        case ASSIGN_EXPRESSION:
            DBG_panic(("bad case...%d", expressType));
            break;
        case ADD_EXPRESSION:
            result->u.double_value = left + right;
            break;
        case SUB_EXPRESSION:
            result->u.double_value = left - right;
            break;
        case MUL_EXPRESSION:
            result->u.double_value = left * right;
            break;
        case DIV_EXPRESSION:
            result->u.double_value = left / right;
            break;
        case MOD_EXPRESSION:
            result->u.double_value = fmod(left, right);
            break;
        case LOGICAL_AND_EXPRESSION:        /* FALLTHRU */
        case LOGICAL_OR_EXPRESSION:
            DBG_panic(("bad case...%d", expressType));
            break;
        case EQ_EXPRESSION:
            result->u.long_value = left == right;
            break;
        case NE_EXPRESSION:
            result->u.long_value = left != right;
            break;
        case GT_EXPRESSION:
            result->u.long_value = left > right;
            break;
        case GE_EXPRESSION:
            result->u.long_value = left >= right;
            break;
        case LT_EXPRESSION:
            result->u.long_value = left < right;
            break;
        case LE_EXPRESSION:
            result->u.long_value = left <= right;
            break;
        case MINUS_EXPRESSION:              /* FALLTHRU */
        case FUNCTION_CALL_EXPRESSION:      /* FALLTHRU */
        case NULL_EXPRESSION:               /* FALLTHRU */
        case EXPRESSION_TYPE_COUNT_PLUS_1:  /* FALLTHRU */
        default:
            DBG_panic(("bad default...%d", expressType));
    }
}

static BEE_Boolean evalCompareString(ExpressionType expressType,
                    BEE_Value *left, BEE_Value *right, int line_number)
{
    BEE_Boolean result;
    int cmp;

    cmp = strcmp(left->u.string_value->string, right->u.string_value->string);

    if (expressType == EQ_EXPRESSION) {
        result = (BEE_Boolean)(cmp == 0);
    } else if (expressType == NE_EXPRESSION) {
        result = (BEE_Boolean)(cmp != 0);
    } else if (expressType == GT_EXPRESSION) {
        result = (BEE_Boolean)(cmp > 0);
    } else if (expressType == GE_EXPRESSION) {
        result = (BEE_Boolean)(cmp >= 0);
    } else if (expressType == LT_EXPRESSION) {
        result = (BEE_Boolean)(cmp < 0);
    } else if (expressType == LE_EXPRESSION) {
        result = (BEE_Boolean)(cmp <= 0);
    } else {
        char *op_str = beeGetOperatorString(expressType);
        beeRuntimeError(line_number, BAD_OPERATOR_FOR_STRING_ERR,
                          STRING_MESSAGE_ARGUMENT, "expressType", op_str,
                          MESSAGE_ARGUMENT_END);
    }
    beeReleaseString(left->u.string_value);
    beeReleaseString(right->u.string_value);

    return result;
}

static BEE_Boolean evalBinaryNull(BEE_Parser *parser, ExpressionType expressType,
                 BEE_Value *left, BEE_Value *right, int line_number)
{
    BEE_Boolean result;

    if (expressType == EQ_EXPRESSION)
    {
        result = (BEE_Boolean)(left->type == BEE_NULL_VALUE && right->type == BEE_NULL_VALUE);
    }
    else if (expressType == NE_EXPRESSION)
    {
        result =  (BEE_Boolean) !(left->type == BEE_NULL_VALUE
                    && right->type == BEE_NULL_VALUE);
    }
    else
    {
        char *op_str = beeGetOperatorString(expressType);
        beeRuntimeError(line_number, NOT_NULL_OPERATOR_ERR,
                          STRING_MESSAGE_ARGUMENT, "expressType", op_str,
                          MESSAGE_ARGUMENT_END);
    }
    releaseIfString(left);
    releaseIfString(right);

    return result;
}

BEE_String * chainString(BEE_Parser *parser, BEE_String *left, BEE_String *right)
{
    int len;
    char *str;
    BEE_String *ret;

    len = strlen(left->string) + strlen(right->string);
    str = (char *)MEM_malloc(len + 1);
    strcpy(str, left->string);
    strcat(str, right->string);
    ret = beeCreateBeeString(parser, str);
    beeReleaseString(left);
    beeReleaseString(right);

    return ret;
}

BEE_Value beeEvalBinaryExpression(BEE_Parser *parser, LocalEnvironment *env,
                           ExpressionType expressionType,
                           Expression *left, Expression *right)
{
    BEE_Value   left_val;
    BEE_Value   right_val;
    BEE_Value   result;

    left_val = evalExpression(parser, env, left);
    right_val = evalExpression(parser, env, right);

    if (left_val.type == BEE_LONG_VALUE && right_val.type == BEE_LONG_VALUE)
    {
        evalBinaryInt(parser, expressionType, left_val.u.long_value, right_val.u.long_value,
                        &result, left->line_number);
    }
    else if (left_val.type == BEE_DOUBLE_VALUE && right_val.type == BEE_DOUBLE_VALUE)
    {
        evalBinaryDouble(parser, expressionType, left_val.u.double_value, right_val.u.double_value,
                           &result, left->line_number);
    }
    else if (left_val.type == BEE_LONG_VALUE && right_val.type == BEE_DOUBLE_VALUE)
    {
        left_val.u.double_value = left_val.u.long_value;
        evalBinaryDouble(parser, expressionType, left_val.u.double_value, right_val.u.double_value,
                           &result, left->line_number);
    }
    else if (left_val.type == BEE_DOUBLE_VALUE && right_val.type == BEE_LONG_VALUE)
    {
        right_val.u.double_value = right_val.u.long_value;
        evalBinaryDouble(parser, expressionType, left_val.u.double_value, right_val.u.double_value,
                           &result, left->line_number);
    }
    else if (left_val.type == BEE_BOOLEAN_VALUE && right_val.type == BEE_BOOLEAN_VALUE)
    {
        result.type = BEE_BOOLEAN_VALUE;
        result.u.boolean_value = evalBinaryBoolean(parser, expressionType,
                                      left_val.u.boolean_value,
                                      right_val.u.boolean_value,
                                      left->line_number);
    }
    else if (left_val.type == BEE_STRING_VALUE && expressionType == ADD_EXPRESSION)
    {
        char    buf[LINE_BUF_SIZE];
        BEE_String *right_str;

        if (right_val.type == BEE_LONG_VALUE)
        {
            sprintf(buf, "%d", right_val.u.long_value);
            right_str = beeCreateBeeString(parser, MEM_strdup(buf));
        }
        else if (right_val.type == BEE_DOUBLE_VALUE)
        {
            sprintf(buf, "%f", right_val.u.double_value);
            right_str = beeCreateBeeString(parser, MEM_strdup(buf));
        }
        else if (right_val.type == BEE_BOOLEAN_VALUE)
        {
            if (right_val.u.boolean_value)
            {
                right_str = beeCreateBeeString(parser,
                                                      MEM_strdup("true"));
            }
            else
            {
                right_str = beeCreateBeeString(parser,
                                                      MEM_strdup("false"));
            }
        }
        else if (right_val.type == BEE_STRING_VALUE)
        {
            right_str = right_val.u.string_value;
        }
        else if (right_val.type == BEE_NATIVE_POINTER_VALUE)
        {
            sprintf(buf, "(%s:%p)",
                    right_val.u.native_pointer.info->name,
                    right_val.u.native_pointer.pointer);
            right_str = beeCreateBeeString(parser, MEM_strdup(buf));
        }
        else if (right_val.type == BEE_NULL_VALUE)
        {
            right_str = beeCreateBeeString(parser, MEM_strdup("null"));
        }
        result.type = BEE_STRING_VALUE;
        result.u.string_value = chainString(parser, left_val.u.string_value, right_str);
    }
    else if (left_val.type == BEE_STRING_VALUE && right_val.type == BEE_STRING_VALUE)
    {
        result.type = BEE_BOOLEAN_VALUE;
        result.u.boolean_value = evalCompareString(expressionType, &left_val, &right_val, left->line_number);
    }
    else if (left_val.type == BEE_NULL_VALUE || right_val.type == BEE_NULL_VALUE)
    {
        result.type = BEE_BOOLEAN_VALUE;
        result.u.boolean_value = evalBinaryNull(parser, expressionType, &left_val, &right_val, left->line_number);
    }
    else
    {
        char *op_str = beeGetOperatorString(expressionType);
        beeRuntimeError(left->line_number, BAD_OPERAND_TYPE_ERR,
                          STRING_MESSAGE_ARGUMENT, "expressType", op_str,
                          MESSAGE_ARGUMENT_END);
    }

    return result;
}

static BEE_Value eval_logical_and_or_expression(BEE_Parser *parser,
                               LocalEnvironment *env,
                               ExpressionType expressType,
                               Expression *left, Expression *right)
{
    BEE_Value   left_val;
    BEE_Value   right_val;
    BEE_Value   result;

    result.type = BEE_BOOLEAN_VALUE;
    left_val = evalExpression(parser, env, left);

    if (left_val.type != BEE_BOOLEAN_VALUE)
    {
        beeRuntimeError(left->line_number, NOT_BOOLEAN_TYPE_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    if (expressType == LOGICAL_AND_EXPRESSION)
    {
        if (!left_val.u.boolean_value)
        {
            result.u.boolean_value = BEE_FALSE;
            return result;
        }
    }
    else if (expressType == LOGICAL_OR_EXPRESSION)
    {
        if (left_val.u.boolean_value)
        {
            result.u.boolean_value = BEE_TRUE;
            return result;
        }
    }
    else
    {
        DBG_panic(("bad expressType..%d\n", expressType));
    }

    right_val = evalExpression(parser, env, right);
    if (right_val.type != BEE_BOOLEAN_VALUE)
    {
        beeRuntimeError(right->line_number, NOT_BOOLEAN_TYPE_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    result.u.boolean_value = right_val.u.boolean_value;

    return result;
}


BEE_Value beeEvalMinusExpression(BEE_Parser *parser, LocalEnvironment *env, Expression *operand)
{
    BEE_Value   operand_val;
    BEE_Value   result;

    operand_val = evalExpression(parser, env, operand);
    if (operand_val.type == BEE_LONG_VALUE)
    {
        result.type = BEE_LONG_VALUE;
        result.u.long_value = -operand_val.u.long_value;
    }
    else if (operand_val.type == BEE_DOUBLE_VALUE)
    {
        result.type = BEE_DOUBLE_VALUE;
        result.u.double_value = -operand_val.u.double_value;
    }
    else 
    {
        beeRuntimeError(operand->line_number, MINUS_OPERAND_TYPE_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    return result;
}

static LocalEnvironment * allocLocalEnvironment()
{
    LocalEnvironment *ret;

    ret = (LocalEnvironment *)MEM_malloc(sizeof(LocalEnvironment));
    ret->variable = NULL;
    ret->global_variable = NULL;

    return ret;
}

static void disposeLocalEnvironment(BEE_Parser *parser, LocalEnvironment *env)
{
    while (env->variable)
    {
        Variable        *temp;
        temp = env->variable;
        if (env->variable->value.type == BEE_STRING_VALUE)
        {
            beeReleaseString(env->variable->value.u.string_value);
        }
        env->variable = temp->next;
        MEM_free(temp);
    }
    while (env->global_variable)
    {
        GlobalVariableRef *ref;
        ref = env->global_variable;
        env->global_variable = ref->next;
        MEM_free(ref);
    }

    MEM_free(env);
}

static BEE_Value callBuiltinFunction(BEE_Parser *parser, LocalEnvironment *env,
                     Expression *expr, BEE_NativeFunctionProc *proc)
{
    BEE_Value   value;
    int         arg_count;
    ArgumentList        *arg_p;
    BEE_Value   *args;
    int         i;

    for (arg_count = 0, arg_p = expr->u.function_call_expression.argument;
         arg_p; arg_p = arg_p->next) {
        arg_count++;
    }

    args = (BEE_Value *)MEM_malloc(sizeof(BEE_Value) * arg_count);

    for (arg_p = expr->u.function_call_expression.argument, i = 0;
         arg_p; arg_p = arg_p->next, i++)
    {
        args[i] = evalExpression(parser, env, arg_p->expression);
    }
    value = proc(parser, arg_count, args);
    for (i = 0; i < arg_count; i++)
    {
        releaseIfString(&args[i]);
    }
    MEM_free(args);

    return value;
}

static BEE_Value callBeeFunction(BEE_Parser *parser, LocalEnvironment *env, Expression *expr, FunctionDefinition *func)
{
    BEE_Value   value;
    StatementResult     result;
    ArgumentList        *arg_p;
    ParameterList       *param_p;
    LocalEnvironment    *local_env;

    local_env = allocLocalEnvironment();

    for (arg_p = expr->u.function_call_expression.argument,
                 param_p = func->u.crowbar_f.parameter;
         arg_p;
         arg_p = arg_p->next, param_p = param_p->next)
    {
        BEE_Value arg_val;

        if (param_p == NULL)
        {
            beeRuntimeError(expr->line_number, ARGUMENT_TOO_MANY_ERR, MESSAGE_ARGUMENT_END);
        }
        arg_val = evalExpression(parser, env, arg_p->expression);
        beeAddLocalVariable(local_env, param_p->name, &arg_val);
    }
    if (param_p)
    {
        beeRuntimeError(expr->line_number, ARGUMENT_TOO_FEW_ERR, MESSAGE_ARGUMENT_END);
    }
    result = beeExecuteStatementList(parser, local_env, func->u.crowbar_f.block->statement_list);
    if (result.type == RETURN_STATEMENT_RESULT)
    {
        value = result.u.return_value;
    }
    else
    {
        value.type = BEE_NULL_VALUE;
    }
    disposeLocalEnvironment(parser, local_env);

    return value;
}

static BEE_Value evalFunctionCallExpression(BEE_Parser *parser, LocalEnvironment *env, Expression *expr)
{
    BEE_Value           value;
    FunctionDefinition  *func;

    char *identifier = expr->u.function_call_expression.identifier;

    func = beeSearchFunction(identifier);
    if (func == NULL)
    {
        printf("function %s not found!\n", identifier);
        //如果能找到符号表，则可以执行
        func = (FunctionDefinition *)beeMalloc(sizeof(FunctionDefinition));
        func->name = identifier;

        ArgumentList * argList = expr->u.function_call_expression.argument;
        while (argList != NULL)
        {
            switch(argList->expression->type)
            {
                case INT_EXPRESSION:
                    printf("value: %ld\n", argList->expression->u.long_value);
                    break;
                case BOOLEAN_EXPRESSION:
                    printf("bool value: \n");
                    break;
                case DOUBLE_EXPRESSION:
                    printf("value: %f\n", argList->expression->u.double_value);
                    break;
                case STRING_EXPRESSION:
                    printf("value: %s\n", argList->expression->u.string_value);
                    break;
                default:
                    printf("un implemetion call \n");
            }

            argList = argList->next;
        }

        func->type = BEE_FUNCTION_DEFINITION;
//        func->u.crowbar_f.parameter ;
//        func->u.crowbar_f.block = block;
        func->next = parser->function_list;
        parser->function_list = func;
        value = callBeeFunction(parser, env, expr, func);
        printf("function called!\n");

//        beeRuntimeError(expr->line_number, FUNCTION_NOT_FOUND_ERR,
//                          STRING_MESSAGE_ARGUMENT, "name", identifier,
//                          MESSAGE_ARGUMENT_END);
    }
    else
    {
        switch (func->type)
        {
            case BEE_FUNCTION_DEFINITION:
                value = callBeeFunction(parser, env, expr, func);
                break;
            case BUILTIN_FUNCTION_DEFINITION:
                value = callBuiltinFunction(parser, env, expr, func->u.native_f.proc);
                break;
            default:
                DBG_panic(("bad case..%d\n", func->type));
        }
    }

    return value;
}

static BEE_Value evalExpression(BEE_Parser *parser, LocalEnvironment *env, Expression *expr)
{
    BEE_Value   v;
    switch (expr->type)
    {
        case BOOLEAN_EXPRESSION:
            v = evalBooleanExpression(expr->u.boolean_value);
            break;
        case INT_EXPRESSION:
            v = evalLongExpression(expr->u.long_value);
            break;
        case DOUBLE_EXPRESSION:
            v = evalDoubleExpression(expr->u.double_value);
            break;
        case STRING_EXPRESSION:
            v = evalStringExpression(parser, expr->u.string_value);
            break;
        case IDENTIFIER_EXPRESSION:
            v = evalIdentifierExpression(parser, env, expr);
            break;
        case ASSIGN_EXPRESSION:
            v = evalAssignExpression(parser, env,
                                       expr->u.assign_expression.variable,
                                       expr->u.assign_expression.operand);
            break;
        case ADD_EXPRESSION:        /* FALLTHRU */
        case SUB_EXPRESSION:        /* FALLTHRU */
        case MUL_EXPRESSION:        /* FALLTHRU */
        case DIV_EXPRESSION:        /* FALLTHRU */
        case MOD_EXPRESSION:        /* FALLTHRU */
        case EQ_EXPRESSION: /* FALLTHRU */
        case NE_EXPRESSION: /* FALLTHRU */
        case GT_EXPRESSION: /* FALLTHRU */
        case GE_EXPRESSION: /* FALLTHRU */
        case LT_EXPRESSION: /* FALLTHRU */
        case LE_EXPRESSION:
            v = beeEvalBinaryExpression(parser, env,
                                           expr->type,
                                           expr->u.binary_expression.left,
                                           expr->u.binary_expression.right);
            break;
        case LOGICAL_AND_EXPRESSION:/* FALLTHRU */
        case LOGICAL_OR_EXPRESSION:
            v = eval_logical_and_or_expression(parser, env, expr->type,
                                               expr->u.binary_expression.left,
                                               expr->u.binary_expression.right);
            break;
        case MINUS_EXPRESSION:
            v = beeEvalMinusExpression(parser, env, expr->u.minus_expression);
            break;
        case FUNCTION_CALL_EXPRESSION:
            v = evalFunctionCallExpression(parser, env, expr);
            break;
        case NULL_EXPRESSION:
            v = evalNullExpression();
            break;
        case EXPRESSION_TYPE_COUNT_PLUS_1:  /* FALLTHRU */
        default:
            DBG_panic(("bad case. type..%d\n", expr->type));
    }
    return v;
}

BEE_Value beeEvalExpression(BEE_Parser *parser, LocalEnvironment *env, Expression *expr)
{
    return evalExpression(parser, env, expr);
}
