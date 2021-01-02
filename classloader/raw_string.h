#ifndef OOPS_CLASSLOADER_RAW_STRING_H
#define OOPS_CLASSLOADER_RAW_STRING_H

#include <cstdint>

namespace oops {
    namespace classloading {
        struct raw_string {
            const char* string;
            const std::int32_t length;
        };
    }
}

#endif