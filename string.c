//
// Created by hadoop on 17-3-22.
//

#include <stdio.h>
#include <string.h>
#include "MEM.h"
#include "bee_def.h"

#define STRING_ALLOC_SIZE       (256)

static char *st_string_literal_buffer = NULL;
static size_t  st_string_literal_buffer_size = 0;
static size_t  st_string_literal_buffer_alloc_size = 0;


void beeOpenStringLiteral(void)
{
    st_string_literal_buffer_size = 0;
}

void beeAddStringLiteral(int letter)
{
    if (st_string_literal_buffer_size == st_string_literal_buffer_alloc_size) {
        st_string_literal_buffer_alloc_size += STRING_ALLOC_SIZE;
        st_string_literal_buffer
                = MEM_realloc(st_string_literal_buffer,
                              st_string_literal_buffer_alloc_size);
    }
    st_string_literal_buffer[st_string_literal_buffer_size] = letter;
    st_string_literal_buffer_size++;
}

void beeResetStringLiteralBuffer(void)
{
    MEM_free(st_string_literal_buffer);
    st_string_literal_buffer = NULL;
    st_string_literal_buffer_size = 0;
    st_string_literal_buffer_alloc_size = 0;
}

char * beeCloseStringLiteral(void)
{
    char *new_str;

    new_str = beeMalloc(st_string_literal_buffer_size + 1);
    memcpy(new_str, st_string_literal_buffer, st_string_literal_buffer_size);
    new_str[st_string_literal_buffer_size] = '\0';

    return new_str;
}

char * beeCreateIdentifier(char *str)
{
    char *new_str;

    new_str = beeMalloc(strlen(str) + 1);
    strcpy(new_str, str);

    return new_str;
}