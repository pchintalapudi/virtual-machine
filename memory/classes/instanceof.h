#ifndef MEMORY_CLASSES_INSTANCEOF_H
#define MEMORY_CLASSES_INSTANCEOF_H

#include <cstdint>
#include <unordered_map>

#include "../../utils/ostring.h"
namespace oops {
    namespace classes {

        class instanceof_table {
            private:
            std::unordered_map<utils::ostring, std::uint32_t> class_indexer;
        }
    }
}

#endif