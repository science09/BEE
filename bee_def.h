//
// Created by hadoop on 17-3-21.
//

#ifndef BEE_BEE_DEF_H
#define BEE_BEE_DEF_H

#include <stdio.h>
#include "MEM.h"
#include "BEE.h"
#include "BEE_dev.h"

#define LARGE(a, b) ((a) > (b) ? (a) : (b))

#define MESSAGE_ARGUMENT_MAX    (256)
#define LINE_BUF_SIZE           (1024)

typedef enum {
    PARSE_ERR = 1,
    CHARACTER_INVALID_ERR,
    FUNCTION_MULTIPLE_DEFINE_ERR,
    COMPILE_ERROR_COUNT_PLUS_1
} CompileError;

typedef enum {
    VARIABLE_NOT_FOUND_ERR = 1,
    FUNCTION_NOT_FOUND_ERR,
    ARGUMENT_TOO_MANY_ERR,
    ARGUMENT_TOO_FEW_ERR,
    NOT_BOOLEAN_TYPE_ERR,
    MINUS_OPERAND_TYPE_ERR,
    BAD_OPERAND_TYPE_ERR,
    NOT_BOOLEAN_OPERATOR_ERR,
    FOPEN_ARGUMENT_TYPE_ERR,
    FCLOSE_ARGUMENT_TYPE_ERR,
    FGETS_ARGUMENT_TYPE_ERR,
    FPUTS_ARGUMENT_TYPE_ERR,
    NOT_NULL_OPERATOR_ERR,
    DIVISION_BY_ZERO_ERR,
    GLOBAL_VARIABLE_NOT_FOUND_ERR,
    GLOBAL_STATEMENT_IN_TOPLEVEL_ERR,
    BAD_OPERATOR_FOR_STRING_ERR,
    RUNTIME_ERROR_COUNT_PLUS_1
} RuntimeError;

typedef enum {
    INT_MESSAGE_ARGUMENT = 1,
    DOUBLE_MESSAGE_ARGUMENT,
    STRING_MESSAGE_ARGUMENT,
    CHARACTER_MESSAGE_ARGUMENT,
    POINTER_MESSAGE_ARGUMENT,
    MESSAGE_ARGUMENT_END
} MessageArgumentType;

typedef struct {
    char *format;
} MessageFormat;

typedef struct Expression_tag Expression;

typedef enum {
    BOOLEAN_EXPRESSION = 1,
    INT_EXPRESSION,
    DOUBLE_EXPRESSION,
    STRING_EXPRESSION,
    IDENTIFIER_EXPRESSION,
    ASSIGN_EXPRESSION,
    ADD_EXPRESSION,
    SUB_EXPRESSION,
    MUL_EXPRESSION,
    DIV_EXPRESSION,
    MOD_EXPRESSION,
    EQ_EXPRESSION,
    NE_EXPRESSION,
    GT_EXPRESSION,
    GE_EXPRESSION,
    LT_EXPRESSION,
    LE_EXPRESSION,
    LOGICAL_AND_EXPRESSION,
    LOGICAL_OR_EXPRESSION,
    MINUS_EXPRESSION,
    FUNCTION_CALL_EXPRESSION,
    NULL_EXPRESSION,
    EXPRESSION_TYPE_COUNT_PLUS_1
} ExpressionType;

#define dkc_is_math_operator(operator) \
  ((operator) == ADD_EXPRESSION || (operator) == SUB_EXPRESSION\
   || (operator) == MUL_EXPRESSION || (operator) == DIV_EXPRESSION\
   || (operator) == MOD_EXPRESSION)

#define dkc_is_compare_operator(operator) \
  ((operator) == EQ_EXPRESSION || (operator) == NE_EXPRESSION\
   || (operator) == GT_EXPRESSION || (operator) == GE_EXPRESSION\
   || (operator) == LT_EXPRESSION || (operator) == LE_EXPRESSION)

#define dkc_is_logical_operator(operator) \
  ((operator) == LOGICAL_AND_EXPRESSION || (operator) == LOGICAL_OR_EXPRESSION)

typedef struct ArgumentList_tag {
    Expression *expression;
    struct ArgumentList_tag *next;
} ArgumentList;

typedef struct {
    char        *variable;
    Expression  *operand;
} AssignExpression;

typedef struct {
    Expression  *left;
    Expression  *right;
} BinaryExpression;

typedef struct {
    char                *identifier;
    ArgumentList        *argument;
} FunctionCallExpression;

typedef struct Expression_tag {
    ExpressionType type;
    int line_number;
    union {
        BEE_Boolean             boolean_value;
        long                    long_value;
        double                  double_value;
        char                    *string_value;
        char                    *identifier;
        AssignExpression        assign_expression;
        BinaryExpression        binary_expression;
        Expression              *minus_expression;
        FunctionCallExpression  function_call_expression;
    } u;
} Expression;

typedef struct Statement_tag Statement;

typedef struct StatementList_tag {
    Statement   *statement;
    struct StatementList_tag    *next;
} StatementList;

typedef struct {
    StatementList       *statement_list;
} Block;

typedef struct IdentifierList_tag {
    char        *name;
    struct IdentifierList_tag   *next;
} IdentifierList;

typedef struct {
    IdentifierList      *identifier_list;
} GlobalStatement;

typedef struct Elsif_tag {
    Expression  *condition;
    Block       *block;
    struct Elsif_tag    *next;
} Elsif;

typedef struct {
    Expression  *condition;
    Block       *then_block;
    Elsif       *elsif_list;
    Block       *else_block;
} IfStatement;

typedef struct {
    Expression  *condition;
    Block       *block;
} WhileStatement;

typedef struct {
    Expression  *init;
    Expression  *condition;
    Expression  *post;
    Block       *block;
} ForStatement;

typedef struct {
    Expression *return_value;
} ReturnStatement;

typedef enum {
    EXPRESSION_STATEMENT = 1,
    GLOBAL_STATEMENT,
    IF_STATEMENT,
    WHILE_STATEMENT,
    FOR_STATEMENT,
    RETURN_STATEMENT,
    BREAK_STATEMENT,
    CONTINUE_STATEMENT,
    STATEMENT_TYPE_COUNT_PLUS_1
} StatementType;

struct Statement_tag {
    StatementType       type;
    int                 line_number;
    union {
        Expression      *expression_s;
        GlobalStatement global_s;
        IfStatement     if_s;
        WhileStatement  while_s;
        ForStatement    for_s;
        ReturnStatement return_s;
    } u;
};

typedef struct ParameterList_tag {
    char        *name;
    struct ParameterList_tag *next;
} ParameterList;

typedef enum {
    BEE_FUNCTION_DEFINITION = 1,
    BUILTIN_FUNCTION_DEFINITION
} FunctionDefinitionType;

typedef struct FunctionDefinition_tag {
    char                *name;
    FunctionDefinitionType      type;
    union {
        struct {
            ParameterList       *parameter;
            Block               *block;
        } crowbar_f;
        struct {
            BEE_NativeFunctionProc      *proc;
        } native_f;
    } u;
    struct FunctionDefinition_tag       *next;
} FunctionDefinition;

typedef struct Variable_tag {
    char        *name;
    BEE_Value   value;
    struct Variable_tag *next;
} Variable;

typedef enum {
    NORMAL_STATEMENT_RESULT = 1,
    RETURN_STATEMENT_RESULT,
    BREAK_STATEMENT_RESULT,
    CONTINUE_STATEMENT_RESULT,
    STATEMENT_RESULT_TYPE_COUNT_PLUS_1
} StatementResultType;

typedef struct {
    StatementResultType type;
    union {
        BEE_Value       return_value;
    } u;
} StatementResult;

typedef struct GlobalVariableRef_tag {
    Variable    *variable;
    struct GlobalVariableRef_tag *next;
} GlobalVariableRef;

typedef struct {
    Variable            *variable;
    GlobalVariableRef   *global_variable;
} LocalEnvironment;

struct BEE_String_tag {
    int         ref_count;
    char        *string;
    BEE_Boolean is_literal;
};

typedef struct {
    BEE_String  *strings;
} StringPool;

struct BEE_Parser_tag {
    MEM_Storage         parser_storage;
    MEM_Storage         execute_storage;
    Variable            *variable;
    FunctionDefinition  *function_list;
    StatementList       *statement_list;
    int                 current_line_number;
};


/* create.c */
void beeFunctionDefine(char *identifier, ParameterList *parameter_list, Block *block);
ParameterList *beeCreateParameter(char *identifier);
ParameterList *beeChainParameter(ParameterList *list, char *identifier);
ArgumentList *beeCreateArgumentList(Expression *expression);
ArgumentList *beeChainArgumentList(ArgumentList *list, Expression *expr);
StatementList *beeCreateStatementList(Statement *statement);
StatementList *beeChainStatementList(StatementList *list, Statement *statement);
Expression *beeAllocExpression(ExpressionType type);
Expression *beeCreateAssignExpression(char *variable, Expression *operand);
Expression *beeCreateBinaryExpression(ExpressionType operator, Expression *left, Expression *right);
Expression *beeCreateMinusExpression(Expression *operand);
Expression *beeCreateIdentifierExpression(char *identifier);
Expression *beeCreateFunctionCallExpression(char *func_name, ArgumentList *argument);
Expression *beeCreateBooleanExpression(BEE_Boolean value);
Expression *beeCreateNullExpression(void);
Statement *beeCreateGlobalStatement(IdentifierList *identifier_list);
IdentifierList *beeCreateGlobalIdentifier(char *identifier);
IdentifierList *beeChainIdentifier(IdentifierList *list, char *identifier);
Statement *beeCreateIfStatement(Expression *condition,
                                   Block *then_block, Elsif *elsif_list,
                                   Block *else_block);
Elsif     *beeChainElseIfList(Elsif *list, Elsif *add);
Elsif     *beeCreateElseIf(Expression *expr, Block *block);
Statement *beeCreateWhileStatement(Expression *condition, Block *block);
Statement *beeCreateForStatement(Expression *init, Expression *cond, Expression *post, Block *block);
Block     *beeCreateBlock(StatementList *statement_list);
Statement *beeCreateExpressionStatement(Expression *expression);
Statement *beeCreateReturnStatement(Expression *expression);
Statement *beeCreateBreakStatement(void);
Statement *beeCreateContinueStatement(void);

/* string.c */
char *beeCreateIdentifier(char *str);
void beeOpenStringLiteral(void);
void beeAddStringLiteral(int letter);
void beeResetStringLiteralBuffer(void);
char *beeCloseStringLiteral(void);

/* execute.c */
StatementResult beeExecuteStatementList(BEE_Parser *inter, LocalEnvironment *env, StatementList *list);

/* eval.c */
BEE_Value beeEvalBinaryExpression(BEE_Parser *inter,
                                     LocalEnvironment *env,
                                     ExpressionType operator,
                                     Expression *left, Expression *right);
BEE_Value beeEvalMinusExpression(BEE_Parser *inter, LocalEnvironment *env, Expression *operand);
BEE_Value beeEvalExpression(BEE_Parser *inter, LocalEnvironment *env, Expression *expr);

/* string_pool.c */
BEE_String *beeLiterToBeeString(BEE_Parser *inter, char *str);
void beeReferString(BEE_String *str);
void beeReleaseString(BEE_String *str);
BEE_String *beeSearchBeeString(BEE_Parser *inter, char *str);
BEE_String *beeCreateBeeString(BEE_Parser *inter, char *str);

/* util.c */
BEE_Parser *beeGetCurrentParser(void);
void beeSetCurrentParser(BEE_Parser *parser);
void *beeMalloc(size_t size);
void *beeExecuteMalloc(BEE_Parser *inter, size_t size);
Variable *beeSearchLocalVariable(LocalEnvironment *env, char *identifier);
Variable *beeSearchGlobalVariable(BEE_Parser *parser, char *identifier);
void beeAddLocalVariable(LocalEnvironment *env, char *identifier, BEE_Value *value);
BEE_NativeFunctionProc *beeSearchBuiltinFunction(BEE_Parser *inter, char *name);
FunctionDefinition *beeSearchFunction(char *name);
char *beeGetOperatorString(ExpressionType type);

/* error.c */
void beeCompileError(CompileError id, ...);
void beeRuntimeError(int line_number, RuntimeError id, ...);

/* native.c */
BEE_Value beeBuiltinPrintFunc(BEE_Parser *parser, int arg_count, BEE_Value *args);
BEE_Value beeBuiltinFopenFunc(BEE_Parser *parser, int arg_count, BEE_Value *args);
BEE_Value beeBuiltinFcloseFunc(BEE_Parser *parser, int arg_count, BEE_Value *args);
BEE_Value beeBuiltinFgetsFunc(BEE_Parser *parser, int arg_count, BEE_Value *args);
BEE_Value beeBuiltinFputsFunc(BEE_Parser *parser, int arg_count, BEE_Value *args);
void beeAddStdFp(BEE_Parser *inter);

#endif //BEE_BEE_DEF_H
