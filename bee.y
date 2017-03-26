%{
#include <stdio.h>
#include "bee_def.h"
#define YYDEBUG 1
%}
%union {
    char                *identifier;
    ParameterList       *parameter_list;
    ArgumentList        *argument_list;
    Expression          *expression;
    Statement           *statement;
    StatementList       *statement_list;
    Block               *block;
    Elsif               *elsif;
    IdentifierList      *identifier_list;
}
%token <expression>     INT_LITERAL
%token <expression>     DOUBLE_LITERAL
%token <expression>     STRING_LITERAL
%token <identifier>     IDENTIFIER
%token FUNCTION IF ELSE ELSIF WHILE FOR RETURN_T BREAK CONTINUE NULL_T
        LP RP LC RC SEMICOLON COMMA ASSIGN LOGICAL_AND LOGICAL_OR
        EQ NE GT GE LT LE ADD SUB MUL DIV MOD TRUE_T FALSE_T GLOBAL_T
%type   <parameter_list> parameter_list
%type   <argument_list> argument_list
%type   <expression> expression expression_opt
        logical_and_expression logical_or_expression
        equality_expression relational_expression
        additive_expression multiplicative_expression
        unary_expression primary_expression
%type   <statement> statement global_statement
        if_statement while_statement for_statement
        return_statement break_statement continue_statement
%type   <statement_list> statement_list
%type   <block> block
%type   <elsif> elsif elsif_list
%type   <identifier_list> identifier_list
%%
translation_unit
        : definition_or_statement
        | translation_unit definition_or_statement
        ;
definition_or_statement
        : function_definition
        | statement
        {
            BEE_Parser *inter = beeGetCurrentParser();

            inter->statement_list
                = beeChainStatementList(inter->statement_list, $1);
        }
        ;
function_definition
        : FUNCTION IDENTIFIER LP parameter_list RP block
        {
            beeFunctionDefine($2, $4, $6);
        }
        | FUNCTION IDENTIFIER LP RP block
        {
            beeFunctionDefine($2, NULL, $5);
        }
        ;
parameter_list
        : IDENTIFIER
        {
            $$ = beeCreateParameter($1);
        }
        | parameter_list COMMA IDENTIFIER
        {
            $$ = beeChainParameter($1, $3);
        }
        ;
argument_list
        : expression
        {
            $$ = beeCreateArgumentList($1);
        }
        | argument_list COMMA expression
        {
            $$ = beeChainArgumentList($1, $3);
        }
        ;
statement_list
        : statement
        {
            $$ = beeCreateStatementList($1);
        }
        | statement_list statement
        {
            $$ = beeChainStatementList($1, $2);
        }
        ;
expression
        : logical_or_expression
        | IDENTIFIER ASSIGN expression
        {
            $$ = beeCreateAssignExpression($1, $3);
        }
        ;
logical_or_expression
        : logical_and_expression
        | logical_or_expression LOGICAL_OR logical_and_expression
        {
            $$ = beeCreateBinaryExpression(LOGICAL_OR_EXPRESSION, $1, $3);
        }
        ;
logical_and_expression
        : equality_expression
        | logical_and_expression LOGICAL_AND equality_expression
        {
            $$ = beeCreateBinaryExpression(LOGICAL_AND_EXPRESSION, $1, $3);
        }
        ;
equality_expression
        : relational_expression
        | equality_expression EQ relational_expression
        {
            $$ = beeCreateBinaryExpression(EQ_EXPRESSION, $1, $3);
        }
        | equality_expression NE relational_expression
        {
            $$ = beeCreateBinaryExpression(NE_EXPRESSION, $1, $3);
        }
        ;
relational_expression
        : additive_expression
        | relational_expression GT additive_expression
        {
            $$ = beeCreateBinaryExpression(GT_EXPRESSION, $1, $3);
        }
        | relational_expression GE additive_expression
        {
            $$ = beeCreateBinaryExpression(GE_EXPRESSION, $1, $3);
        }
        | relational_expression LT additive_expression
        {
            $$ = beeCreateBinaryExpression(LT_EXPRESSION, $1, $3);
        }
        | relational_expression LE additive_expression
        {
            $$ = beeCreateBinaryExpression(LE_EXPRESSION, $1, $3);
        }
        ;
additive_expression
        : multiplicative_expression
        | additive_expression ADD multiplicative_expression
        {
            $$ = beeCreateBinaryExpression(ADD_EXPRESSION, $1, $3);
        }
        | additive_expression SUB multiplicative_expression
        {
            $$ = beeCreateBinaryExpression(SUB_EXPRESSION, $1, $3);
        }
        ;
multiplicative_expression
        : unary_expression
        | multiplicative_expression MUL unary_expression
        {
            $$ = beeCreateBinaryExpression(MUL_EXPRESSION, $1, $3);
        }
        | multiplicative_expression DIV unary_expression
        {
            $$ = beeCreateBinaryExpression(DIV_EXPRESSION, $1, $3);
        }
        | multiplicative_expression MOD unary_expression
        {
            $$ = beeCreateBinaryExpression(MOD_EXPRESSION, $1, $3);
        }
        ;
unary_expression
        : primary_expression
        | SUB unary_expression
        {
            $$ = beeCreateMinusExpression($2);
        }
        ;
primary_expression
        : IDENTIFIER LP argument_list RP
        {
            $$ = beeCreateFunctionCallExpression($1, $3);
        }
        | IDENTIFIER LP RP
        {
            $$ = beeCreateFunctionCallExpression($1, NULL);
        }
        | LP expression RP
        {
            $$ = $2;
        }
        | IDENTIFIER
        {
            $$ = beeCreateIdentifierExpression($1);
        }
        | INT_LITERAL
        | DOUBLE_LITERAL
        | STRING_LITERAL
        | TRUE_T
        {
            $$ = beeCreateBooleanExpression(BEE_TRUE);
        }
        | FALSE_T
        {
            $$ = beeCreateBooleanExpression(BEE_FALSE);
        }
        | NULL_T
        {
            $$ = beeCreateNullExpression();
        }
        ;
statement
        : expression SEMICOLON
        {
          $$ = beeCreateExpressionStatement($1);
        }
        | global_statement
        | if_statement
        | while_statement
        | for_statement
        | return_statement
        | break_statement
        | continue_statement
        ;
global_statement
        : GLOBAL_T identifier_list SEMICOLON
        {
            $$ = beeCreateGlobalStatement($2);
        }
        ;
identifier_list
        : IDENTIFIER
        {
            $$ = beeCreateGlobalIdentifier($1);
        }
        | identifier_list COMMA IDENTIFIER
        {
            $$ = beeChainIdentifier($1, $3);
        }
        ;
if_statement
        : IF LP expression RP block
        {
            $$ = beeCreateIfStatement($3, $5, NULL, NULL);
        }
        | IF LP expression RP block ELSE block
        {
            $$ = beeCreateIfStatement($3, $5, NULL, $7);
        }
        | IF LP expression RP block elsif_list
        {
            $$ = beeCreateIfStatement($3, $5, $6, NULL);
        }
        | IF LP expression RP block elsif_list ELSE block
        {
            $$ = beeCreateIfStatement($3, $5, $6, $8);
        }
        ;
elsif_list
        : elsif
        | elsif_list elsif
        {
            $$ = beeChainElseIfList($1, $2);
        }
        ;
elsif
        : ELSIF LP expression RP block
        {
            $$ = beeCreateElseIf($3, $5);
        }
        ;
while_statement
        : WHILE LP expression RP block
        {
            $$ = beeCreateWhileStatement($3, $5);
        }
        ;
for_statement
        : FOR LP expression_opt SEMICOLON expression_opt SEMICOLON
          expression_opt RP block
        {
            $$ = beeCreateForStatement($3, $5, $7, $9);
        }
        ;
expression_opt
        : /* empty */
        {
            $$ = NULL;
        }
        | expression
        ;
return_statement
        : RETURN_T expression_opt SEMICOLON
        {
            $$ = beeCreateReturnStatement($2);
        }
        ;
break_statement
        : BREAK SEMICOLON
        {
            $$ = beeCreateBreakStatement();
        }
        ;
continue_statement
        : CONTINUE SEMICOLON
        {
            $$ = beeCreateContinueStatement();
        }
        ;
block
        : LC statement_list RC
        {
            $$ = beeCreateBlock($2);
        }
        | LC RC
        {
            $$ = beeCreateBlock(NULL);
        }
        ;
%%
