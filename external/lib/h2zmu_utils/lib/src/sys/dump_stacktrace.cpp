//
// Created by jriessner on 15.06.23.
//

#include <vector>
#include "../../include/sys/dump_stacktrace.h"


namespace utils {
    namespace sys {

        std::string dump_stacktrace(unsigned int max_frames, uint8_t indentation) {
            // TODO Möglichkeit einbauen den ersten Eintrag zu überspringen -> bei Log message unwichtig dass die Funktion Log::error() im Stacktrace ist
            // TODO Funktion reparieren -> wenn in std::async aufgerufen -> Speicherfehler

            std::string ret;
            std::string indent;
            for (int i = 0; i < indentation; ++i) {
                indent += " ";
            }

            ret += "stack trace:\n    // TODO Funktion reparieren -> wenn in std::async aufgerufen -> Speicherfehler";
            return ret;

            // storage array for stack trace address data
            void *addrlist[max_frames + 1];

            // retrieve current stack addresses
            int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void *));

            if (addrlen == 0) {
                ret += "  <empty, possibly corrupt>\n";
                return ret;
            }

            // resolve addresses into strings containing "filename(function+address)",
            // this array must be free()-ed
            char **symbollist = backtrace_symbols(addrlist, addrlen);

            // allocate string which will be filled with the demangled function name
            size_t funcnamesize = 256;
            std::vector<char> funcname = std::vector<char>(funcnamesize);

            // iterate over the returned symbol lines. skip the first, it is the
            // address of this function.
            for (int i = 1; i < addrlen; i++) {
                char* buffer = (char*)(malloc(sizeof(char) * 1024));

                char *begin_name = 0, *begin_offset = 0, *end_offset = 0;

                // find parentheses and +address offset surrounding the mangled name:
                // ./module(function+0x15c) [0x8048a6d]
                for (char *p = symbollist[i]; *p; ++p) {
                    if (*p == '(')
                        begin_name = p;
                    else if (*p == '+')
                        begin_offset = p;
                    else if (*p == ')' && begin_offset) {
                        end_offset = p;
                        break;
                    }
                }

                if (begin_name && begin_offset && end_offset
                    && begin_name < begin_offset) {
                    *begin_name++ = '\0';
                    *begin_offset++ = '\0';
                    *end_offset = '\0';

                    // mangled name is now in [begin_name, begin_offset) and caller
                    // offset in [begin_offset, end_offset). now apply
                    // __cxa_demangle():

                    int status;
                    char *inner_ret = abi::__cxa_demangle(begin_name,
                                                          funcname.data(), &funcnamesize, &status);
                    if (status == 0) {
                        funcname = std::vector<char>(inner_ret, inner_ret + ( sizeof (char) * sizeof(inner_ret)) ); // use possibly realloc()-ed string
                        sprintf(buffer, "%s%s : %s+%s\n",
                                indent.c_str(), symbollist[i], funcname.data(), begin_offset);
                    } else {
                        // demangling failed. Output function name as a C function with
                        // no arguments.
                        sprintf(buffer, "%s%s : %s()+%s\n",
                                indent.c_str(), symbollist[i], begin_name, begin_offset);
                    }
                } else {
                    // couldn't parse the line? print the whole line.
                    sprintf(buffer, "  %s\n", symbollist[i]);
                }


                ret += std::string(buffer);

                free(buffer);

            }

            delete symbollist;


            return ret;
        }

        void print_stacktrace(FILE *out, unsigned int max_frames) {
            fprintf(out, "%s", dump_stacktrace(max_frames).c_str());
        }

    }
}
