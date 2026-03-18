#ifndef PANIC_H_
#define PANIC_H_

#include <cstdio>
#include <cstdlib>

#define PANIC(message)                            \
        do {                                     \
                std::fprintf(stderr, "%s\n", message); \
                std::abort();                    \
        } while (0)

#endif // PANIC_H_
