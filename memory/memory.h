//
// Created by hadoop on 17-3-25.
//

#ifndef BEE_MEMORY_H
#define BEE_MEMORY_H

#include "MEM.h"

typedef union Header_tag Header;


struct MEM_Controller_tag {
    FILE        *error_fp;
    MEM_ErrorHandler    error_handler;
    MEM_FailMode        fail_mode;
    Header      *block_header;
};

#endif //BEE_MEMORY_H
