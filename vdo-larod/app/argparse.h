/**
 * Copyright (C) 2018-2020, Axis Communications AB, Lund, Sweden
 */

/**
 * This header file parses the arguments to the application.
 */

#pragma once

#include <stddef.h>

#include "larod.h"

typedef struct args_t {
    size_t outputBytes;
    char* modelFile;
    unsigned width;
    unsigned height;
    unsigned numFrames;
    larodChip chip;
} args_t;

bool parseArgs(int argc, char** argv, args_t* args);
