//
// Created by hadoop on 17-3-22.
//

#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include "bee_def.h"
#include "DBG.h"
#include "MEM.h"


MessageFormat beeCompileErrorMessageFormat[] = {
        {"dummy"},
        {"在($(token))附近发生语法错误"},
        {"不正确的字符($(bad_char))"},
        {"函数名重复($(name))"},
        {"dummy"},
};

MessageFormat beeRuntimeErrorMessageFormat[] = {
        {"dummy"},
        {"找不到变量($(name))。"},
        {"找不到函数($(name))。"},
        {"传入的参数数量多于函数定义。"},
        {"传入的参数数量少于函数定义。"},
        {"条件表达式的值必须是boolean型。"},
        {"减法运算的操作数必须是数值类型。"},
        {"双目操作符$(operator)的操作数类型不正确。"},
        {"$(operator)操作符不能用于boolean型。"},
        {"请为fopen()函数传入文件的路径和打开方式（两者都是字符串类型的）。"},
        {"请为fclose()函数传入文件指针。"},
        {"请为fgets()函数传入文件指针。"},
        {"请为fputs()函数传入文件指针和字符串。"},
        {"null只能用于运算符 == 和 !=(不能进行$(operator)操作)。"},
        {"不能被0除。"},
        {"全局变量$(name)不存在。"},
        {"不能在函数外使用global语句。"},
        {"运算符$(operator)不能用于字符串类型。"},
        {"dummy"},
};

//extern char * yytext;

typedef struct {
    char    *string;
}VString;

static void clearVString(VString *v)
{
    v->string = NULL;
}

size_t myStrLen(char *str) {
    if (str == NULL)
    {
        return 0;
    }
    return strlen(str);
}

static void addString(VString *v, char *str)
{
    size_t new_size;
    size_t old_len;

    if (str == NULL)
    {
        return;
    }
    old_len = myStrLen(v->string);
    new_size = old_len + strlen(str);
    v->string = (char *)MEM_realloc(v->string, new_size);
    strcpy(&v->string[old_len], str);
}

static void addCharacter(VString *v, char ch)
{
    size_t current_len;

    current_len = myStrLen(v->string);
    v->string = (char *)MEM_realloc(v->string, current_len+2);
    v->string[current_len] = ch;
    v->string[current_len+1] = '\0';
}

typedef struct {
    MessageArgumentType type;
    char        *name;
    union {
        int     int_val;
        double  double_val;
        char    *string_val;
        void    *pointer_val;
        int     character_val;
    } u;
} MessageArgument;

static void createMessageArgument(MessageArgument *arg, va_list ap)
{
    int index = 0;
    MessageArgumentType type;

    while ((type = va_arg(ap, MessageArgumentType)) != MESSAGE_ARGUMENT_END)
    {
        arg[index].type = type;
        arg[index].name = va_arg(ap, char*);
        switch (type)
        {
            case INT_MESSAGE_ARGUMENT:
                arg[index].u.int_val = va_arg(ap, int);
                break;
            case DOUBLE_MESSAGE_ARGUMENT:
                arg[index].u.double_val = va_arg(ap, double);
                break;
            case STRING_MESSAGE_ARGUMENT:
                arg[index].u.string_val = va_arg(ap, char*);
                break;
            case POINTER_MESSAGE_ARGUMENT:
                arg[index].u.pointer_val = va_arg(ap, void*);
                break;
            case CHARACTER_MESSAGE_ARGUMENT:
                arg[index].u.character_val = va_arg(ap, int);
                break;
            case MESSAGE_ARGUMENT_END:
                assert(0);
                break;
            default:
                assert(0);
        }
        index++;
        assert(index < MESSAGE_ARGUMENT_MAX);
    }
}

static void searchArgument(MessageArgument *arg_list, char *arg_name, MessageArgument *arg)
{
    int i;

    for (i = 0; arg_list[i].type != MESSAGE_ARGUMENT_END; i++)
    {
        if (!strcmp(arg_list[i].name, arg_name))
        {
            *arg = arg_list[i];
            return;
        }
    }
    assert(0);
}

static void formatMessage(MessageFormat *format, VString *v, va_list ap)
{
    int         i;
    char        buf[LINE_BUF_SIZE];
    int         arg_name_index;
    char        arg_name[LINE_BUF_SIZE];
    MessageArgument     arg[MESSAGE_ARGUMENT_MAX];
    MessageArgument     cur_arg;

    createMessageArgument(arg, ap);

    for (i = 0; format->format[i] != '\0'; i++)
    {
        if (format->format[i] != '$')
        {
            addCharacter(v, format->format[i]);
            continue;
        }
        assert(format->format[i+1] == '(');
        i += 2;
        for (arg_name_index = 0; format->format[i] != ')';
             arg_name_index++, i++)
        {
            arg_name[arg_name_index] = format->format[i];
        }
        arg_name[arg_name_index] = '\0';
        assert(format->format[i] == ')');

        searchArgument(arg, arg_name, &cur_arg);
        switch (cur_arg.type)
        {
            case INT_MESSAGE_ARGUMENT:
                sprintf(buf, "%d", cur_arg.u.int_val);
                addString(v, buf);
                break;
            case DOUBLE_MESSAGE_ARGUMENT:
                sprintf(buf, "%f", cur_arg.u.double_val);
                addString(v, buf);
                break;
            case STRING_MESSAGE_ARGUMENT:
                strcpy(buf, cur_arg.u.string_val);
                addString(v, cur_arg.u.string_val);
                break;
            case POINTER_MESSAGE_ARGUMENT:
                sprintf(buf, "%p", cur_arg.u.pointer_val);
                addString(v, buf);
                break;
            case CHARACTER_MESSAGE_ARGUMENT:
                sprintf(buf, "%c", cur_arg.u.character_val);
                addString(v, buf);
                break;
            case MESSAGE_ARGUMENT_END:
                assert(0);
                break;
            default:
                assert(0);
        }
    }
}

void selfCheck()
{
    if (strcmp(beeCompileErrorMessageFormat[0].format, "dummy") != 0)
    {
        DBG_panic(("compile error message format error.\n"));
    }
    if (strcmp(beeCompileErrorMessageFormat
               [COMPILE_ERROR_COUNT_PLUS_1].format,
               "dummy") != 0)
    {
        DBG_panic(("compile error message format error. "
                "COMPILE_ERROR_COUNT_PLUS_1..%d\n",
                COMPILE_ERROR_COUNT_PLUS_1));
    }
    if (strcmp(beeRuntimeErrorMessageFormat[0].format, "dummy") != 0)
    {
        DBG_panic(("runtime error message format error.\n"));
    }
    if (strcmp(beeRuntimeErrorMessageFormat
               [RUNTIME_ERROR_COUNT_PLUS_1].format,
               "dummy") != 0) 
    {
        DBG_panic(("runtime error message format error. "
                "RUNTIME_ERROR_COUNT_PLUS_1..%d\n",
                RUNTIME_ERROR_COUNT_PLUS_1));
    }
}

void beeCompileError(CompileError id, ...)
{
    va_list     ap;
    VString     message;
    int         line_number;

    selfCheck();
    va_start(ap, id);
    line_number = beeGetCurrentParser()->current_line_number;
    clearVString(&message);
    formatMessage(&beeCompileErrorMessageFormat[id], &message, ap);
    fprintf(stderr, "%3d:%s\n", line_number, message.string);
    va_end(ap);

    exit(1);
}

void beeRuntimeError(int line_number, RuntimeError id, ...)
{
    va_list     ap;
    VString     message;

    selfCheck();
    va_start(ap, id);
    clearVString(&message);
    formatMessage(&beeRuntimeErrorMessageFormat[id], &message, ap);
    fprintf(stderr, "%3d:%s\n", line_number, message.string);
    va_end(ap);

    exit(1);
}


int yyerror(char const *str)
{
    char *near_token;
    near_token = "EOF";
//
//    if (yytext[0] == '\0') {
//        near_token = "EOF";
//    } else {
//        near_token = yytext;
//    }
    beeCompileError(PARSE_ERR, STRING_MESSAGE_ARGUMENT, "token", near_token, MESSAGE_ARGUMENT_END);

    return 0;
}