//
// Created by jriessner on 05.06.23.
//

#ifndef H2ZMU_V1_DUMP_STACKTRACE_H
#define H2ZMU_V1_DUMP_STACKTRACE_H

#include <cstdio>
#include <cstdlib>
#include <string>
#include <execinfo.h>
#include <cxxabi.h>

namespace utils {
    namespace sys {

        std::string dump_stacktrace(unsigned int max_frames = 63, uint8_t indentation = 4);

        void dump_stacktrace1(char *out, unsigned int max_frames = 63, uint8_t indentation = 4);

        void print_stacktrace(FILE *out = stderr, unsigned int max_frames = 63);

    }
}

#endif //H2ZMU_V1_DUMP_STACKTRACE_H
