#ifndef MEMORY_OLD_HEAP
#define MEMORY_OLD_HEAP

#include "../objects/objects.h"

namespace oops {
    namespace memory {
        class old_heap {
            private:
            public:
            std::optional<objects::object> allocate_object(objects::clazz cls);

            std::optional<objects::array> allocate_array(objects::field::type array_type, std::uint32_t array_length);
        };
    }
}
#endif /* MEMORY_OLD_HEAP */
