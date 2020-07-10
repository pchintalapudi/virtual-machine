#ifndef INTERFACE_INSTANCEOF
#define INTERFACE_INSTANCEOF

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "../objects/class.h"

namespace oops {
    namespace interfaze {
        class instanceof {
            private:
            std::vector<std::uintptr_t> implemented;
            std::unordered_map<char*, std::size_t> class_index;
            public:
            bool operator()(objects::clazz src, objects::clazz test) const;

            void insert_new_class(objects::clazz cls, std::size_t length, void *classes);
        };
    }
}
#endif /* INTERFACE_INSTANCEOF */
