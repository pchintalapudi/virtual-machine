#ifndef MEMORY_HEAP
#define MEMORY_HEAP

#include "../objects/objects.h"
#include "young_heap.h"
#include "old_heap.h"

namespace oops {
    namespace memory {
        class heap {
            private:
            young_heap young_generation;
            old_heap old_generation;

            std::size_t max_young_object_size;

            std::optional<objects::object> allocate_object_old(objects::clazz cls);

            std::optional<objects::array> allocate_array_old(objects::clazz acls, std::uint64_t memory_size);

            std::optional<objects::object> allocate_object_young(objects::clazz cls);

            std::optional<objects::array> allocate_array_young(objects::clazz acls, std::uint64_t memory_size);

            public:
            std::optional<objects::object> allocate_object(objects::clazz cls);

            std::optional<objects::array> allocate_array(objects::field::type array_type, std::uint32_t array_length);
        };
    }
}

#endif /* MEMORY_HEAP */
