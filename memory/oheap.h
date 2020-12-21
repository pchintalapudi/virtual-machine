#ifndef OOPS_MEMORY_OHEAP_H
#define OOPS_MEMORY_OHEAP_H

#include "../classes/class.h"
#include "../classes/object.h"
#include "../classes/datatypes.h"

namespace oops {
    namespace memory {
        class heap {
            private:
            std::optional<void*> allocate_memory(std::uintptr_t amount);
            public:

            std::optional<classes::object> allocate_object(classes::clazz);
            std::optional<classes::array> allocate_array(classes::datatype dt, std::int32_t length);
        };
    }
}

#endif