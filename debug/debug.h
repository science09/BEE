//
// Created by hadoop on 17-3-25.
//

#ifndef BEE_DEBUG_H
#define BEE_DEBUG_H

#include <stdio.h>
#include "../DBG.h"

struct DBG_Controller_tag {
    FILE        *debug_write_fp;
    int         current_debug_level;
};


#endif //BEE_DEBUG_H
