//
// Created by hadoop on 17-3-22.
//

#include "MEM.h"
#include "DBG.h"
#include "bee_def.h"


void beeFunctionDefine(char *identifier, ParameterList *parameter_list, Block *block)
{
    FunctionDefinition *f;
    BEE_Parser *parser;

    if (beeSearchFunction(identifier))
    {
        beeCompileError(FUNCTION_MULTIPLE_DEFINE_ERR,
                          STRING_MESSAGE_ARGUMENT, "name", identifier,
                          MESSAGE_ARGUMENT_END);
        return;
    }
    parser = beeGetCurrentParser();

    f = beeMalloc(sizeof(FunctionDefinition));
    f->name = identifier;
    f->type = BEE_FUNCTION_DEFINITION;
    f->u.crowbar_f.parameter = parameter_list;
    f->u.crowbar_f.block = block;
    f->next = parser->function_list;
    parser->function_list = f;
}

ParameterList * beeCreateParameter(char *identifier)
{
    ParameterList       *p;

    p = beeMalloc(sizeof(ParameterList));
    p->name = identifier;
    p->next = NULL;

    return p;
}

ParameterList * beeChainParameter(ParameterList *list, char *identifier)
{
    ParameterList *pos;

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = beeCreateParameter(identifier);

    return list;
}

ArgumentList * beeCreateArgumentList(Expression *expression)
{
    ArgumentList *al;

    al = beeMalloc(sizeof(ArgumentList));
    al->expression = expression;
    al->next = NULL;

    return al;
}

ArgumentList * beeChainArgumentList(ArgumentList *list, Expression *expr)
{
    ArgumentList *pos;

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = beeCreateArgumentList(expr);

    return list;
}

StatementList * beeCreateStatementList(Statement *statement)
{
    StatementList *sl;

    sl = beeMalloc(sizeof(StatementList));
    sl->statement = statement;
    sl->next = NULL;

    return sl;
}

StatementList * beeChainStatementList(StatementList *list, Statement *statement)
{
    StatementList *pos;

    if (list == NULL)
        return beeCreateStatementList(statement);

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = beeCreateStatementList(statement);

    return list;
}

Expression * beeAllocExpression(ExpressionType type)
{
    Expression  *exp;

    exp = beeMalloc(sizeof(Expression));
    exp->type = type;
    exp->line_number = beeGetCurrentParser()->current_line_number;

    return exp;
}

Expression * beeCreateAssignExpression(char *variable, Expression *operand)
{
    Expression *exp;

    exp = beeAllocExpression(ASSIGN_EXPRESSION);
    exp->u.assign_expression.variable = variable;
    exp->u.assign_expression.operand = operand;

    return exp;
}

static Expression convertValueToExpression(BEE_Value *v)
{
    Expression  expr;

    if (v->type == BEE_LONG_VALUE)
    {
        expr.type = INT_EXPRESSION;
        expr.u.long_value = v->u.long_value;
    }
    else if (v->type == BEE_DOUBLE_VALUE)
    {
        expr.type = DOUBLE_EXPRESSION;
        expr.u.double_value = v->u.double_value;
    }
    else
    {
        DBG_assert(v->type == BEE_BOOLEAN_VALUE,
                   ("v->type..%d\n", v->type));
        expr.type = BOOLEAN_EXPRESSION;
        expr.u.boolean_value = v->u.boolean_value;
    }
    return expr;
}

Expression * beeCreateBinaryExpression(ExpressionType operator, Expression *left, Expression *right)
{
    if ((left->type == INT_EXPRESSION
         || left->type == DOUBLE_EXPRESSION)
        && (right->type == INT_EXPRESSION
            || right->type == DOUBLE_EXPRESSION))
    {
        BEE_Value v;
        v = beeEvalBinaryExpression(beeGetCurrentParser(), NULL, operator, left, right);
        /* Overwriting left hand expression. */
        *left = convertValueToExpression(&v);

        return left;
    }
    else
    {
        Expression *exp;
        exp = beeAllocExpression(operator);
        exp->u.binary_expression.left = left;
        exp->u.binary_expression.right = right;
        return exp;
    }
}

Expression * beeCreateMinusExpression(Expression *operand)
{
    if (operand->type == INT_EXPRESSION
        || operand->type == DOUBLE_EXPRESSION)
    {
        BEE_Value       v;
        v = beeEvalMinusExpression(beeGetCurrentParser(), NULL, operand);
        /* Notice! Overwriting operand expression. */
        *operand = convertValueToExpression(&v);
        return operand;
    }
    else
    {
        Expression      *exp;
        exp = beeAllocExpression(MINUS_EXPRESSION);
        exp->u.minus_expression = operand;
        return exp;
    }
}

Expression * beeCreateIdentifierExpression(char *identifier)
{
    Expression  *exp;

    exp = beeAllocExpression(IDENTIFIER_EXPRESSION);
    exp->u.identifier = identifier;

    return exp;
}

Expression * beeCreateFunctionCallExpression(char *func_name, ArgumentList *argument)
{
    Expression  *exp;

    exp = beeAllocExpression(FUNCTION_CALL_EXPRESSION);
    exp->u.function_call_expression.identifier = func_name;
    exp->u.function_call_expression.argument = argument;

    return exp;
}

Expression * beeCreateBooleanExpression(BEE_Boolean value)
{
    Expression *exp;

    exp = beeAllocExpression(BOOLEAN_EXPRESSION);
    exp->u.boolean_value = value;

    return exp;
}

Expression * beeCreateNullExpression(void)
{
    Expression  *exp;

    exp = beeAllocExpression(NULL_EXPRESSION);

    return exp;
}

static Statement * allocStatement(StatementType type)
{
    Statement *st;

    st = beeMalloc(sizeof(Statement));
    st->type = type;
    st->line_number = beeGetCurrentParser()->current_line_number;

    return st;
}

Statement * beeCreateGlobalStatement(IdentifierList *identifier_list)
{
    Statement *st;

    st = allocStatement(GLOBAL_STATEMENT);
    st->u.global_s.identifier_list = identifier_list;

    return st;
}

IdentifierList * beeCreateGlobalIdentifier(char *identifier)
{
    IdentifierList      *i_list;

    i_list = beeMalloc(sizeof(IdentifierList));
    i_list->name = identifier;
    i_list->next = NULL;

    return i_list;
}

IdentifierList * beeChainIdentifier(IdentifierList *list, char *identifier)
{
    IdentifierList *pos;

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = beeCreateGlobalIdentifier(identifier);

    return list;
}

Statement * beeCreateIfStatement(Expression *condition,
                        Block *then_block, Elsif *elsif_list,
                        Block *else_block)
{
    Statement *st;

    st = allocStatement(IF_STATEMENT);
    st->u.if_s.condition = condition;
    st->u.if_s.then_block = then_block;
    st->u.if_s.elsif_list = elsif_list;
    st->u.if_s.else_block = else_block;

    return st;
}

Elsif * beeChainElseIfList(Elsif *list, Elsif *add)
{
    Elsif *pos;

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = add;

    return list;
}

Elsif * beeCreateElseIf(Expression *expr, Block *block)
{
    Elsif *ei;

    ei = beeMalloc(sizeof(Elsif));
    ei->condition = expr;
    ei->block = block;
    ei->next = NULL;

    return ei;
}

Statement * beeCreateWhileStatement(Expression *condition, Block *block)
{
    Statement *st;

    st = allocStatement(WHILE_STATEMENT);
    st->u.while_s.condition = condition;
    st->u.while_s.block = block;

    return st;
}

Statement * beeCreateForStatement(Expression *init, Expression *cond, Expression *post, Block *block)
{
    Statement *st;

    st = allocStatement(FOR_STATEMENT);
    st->u.for_s.init = init;
    st->u.for_s.condition = cond;
    st->u.for_s.post = post;
    st->u.for_s.block = block;

    return st;
}

Block * beeCreateBlock(StatementList *statement_list)
{
    Block *block;

    block = beeMalloc(sizeof(Block));
    block->statement_list = statement_list;

    return block;
}

Statement * beeCreateExpressionStatement(Expression *expression)
{
    Statement *st;

    st = allocStatement(EXPRESSION_STATEMENT);
    st->u.expression_s = expression;

    return st;
}

Statement * beeCreateReturnStatement(Expression *expression)
{
    Statement *st;

    st = allocStatement(RETURN_STATEMENT);
    st->u.return_s.return_value = expression;

    return st;
}

Statement * beeCreateBreakStatement(void)
{
    return allocStatement(BREAK_STATEMENT);
}

Statement * beeCreateContinueStatement(void)
{
    return allocStatement(CONTINUE_STATEMENT);
}

