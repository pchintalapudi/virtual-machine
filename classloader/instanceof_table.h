#ifndef OOPS_CLASSLOADER_INSTANCEOF_TABLE_H
#define OOPS_CLASSLOADER_INSTANCEOF_TABLE_H

#include <unordered_map>
#include <vector>
#include "../classes/class.h"

namespace oops {
    namespace classloading {
        class instanceof_table {
            private:
            std::unordered_map<void*, std::pair<std::int32_t, std::int32_t>> class_indeces;
            std::vector<void*> classes;
            public:

            bool is_superclass(classes::clazz super, classes::clazz sub);
        };
    }
}

#endif