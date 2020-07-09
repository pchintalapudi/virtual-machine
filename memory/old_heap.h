#ifndef MEMORY_OLD_HEAP
#define MEMORY_OLD_HEAP

#include <array>
#include "../objects/objects.h"

namespace oops {
    namespace memory {
        class old_heap {
            private:
            static constexpr std::size_t linked_list_count = 128;
            std::array<char*, linked_list_count> linked_lists;
            std::array<char*, sizeof(std::uint64_t) * CHAR_BIT> rb_trees;

            std::optional<char*> allocate_memory(std::uint64_t memory_size);

            public:
            std::optional<objects::object> allocate_object(objects::clazz cls);

            std::optional<objects::array> allocate_array(objects::clazz acls, std::uint64_t required_size);
        };
    }
}
#endif /* MEMORY_OLD_HEAP */
