//
// Created by hadoop on 17-3-22.
//

#include <stdio.h>
#include <string.h>
#include "MEM.h"
#include "bee_def.h"

#define STRING_ALLOC_SIZE       (256)


void beeOpenStringLiteral(void)
{
    BEE_Parser *parser;
    parser = beeGetCurrentParser();
    parser->st_string_literal_buffer_size = 0;
}

void beeAddStringLiteral(int letter)
{
    BEE_Parser *parser = beeGetCurrentParser();
    if (parser->st_string_literal_buffer_size == parser->st_string_literal_buffer_alloc_size)
    {
        parser->st_string_literal_buffer_alloc_size += STRING_ALLOC_SIZE;
        parser->st_string_literal_buffer = (char *) MEM_realloc(parser->st_string_literal_buffer,
                                                                parser->st_string_literal_buffer_alloc_size);
    }
    parser->st_string_literal_buffer[parser->st_string_literal_buffer_size] = (char)letter;
    parser->st_string_literal_buffer_size++;
}

void beeResetStringLiteralBuffer(void)
{
    BEE_Parser *parser = beeGetCurrentParser();
    MEM_free(parser->st_string_literal_buffer);
    parser->st_string_literal_buffer = NULL;
    parser->st_string_literal_buffer_size = 0;
    parser->st_string_literal_buffer_alloc_size = 0;
}

char * beeCloseStringLiteral(void)
{
    char *new_str;

    BEE_Parser *parser = beeGetCurrentParser();
    new_str = (char *)beeMalloc(parser->st_string_literal_buffer_size + 1);
    memcpy(new_str, parser->st_string_literal_buffer, parser->st_string_literal_buffer_size);
    new_str[parser->st_string_literal_buffer_size] = '\0';

    return new_str;
}

char * beeCreateIdentifier(char *str)
{
    char *new_str;

    new_str = (char *)beeMalloc(strlen(str) + 1);
    strcpy(new_str, str);

    return new_str;
}