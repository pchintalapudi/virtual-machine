#ifndef MEMORY_OLD_HEAP
#define MEMORY_OLD_HEAP

#include <array>
#include "../objects/objects.h"

namespace oops {
    namespace memory {
        class heap;
        class old_heap {
            private:
            friend class heap;
            char *base, *head, *cap;
            static constexpr std::size_t linked_list_count = 128;
            std::array<char*, linked_list_count> linked_lists;
            std::array<char*, sizeof(std::uint64_t) * CHAR_BIT> rb_trees;

            double requested_free_ratio = 0.4;
            std::uint64_t allocation_granularity;

            std::optional<char*> allocate_memory(std::uint64_t memory_size);

            void prep_for_gc();

            void finish_old_gc(std::uint64_t free_memory);

            bool guarantee(std::uint64_t object_count, std::uint64_t object_size);

            bool is_old_object(objects::base_object);

            void sweep();

            public:

            struct args {
                std::uint64_t min_size, max_size;
                double requested_free_ratio;
                std::uint64_t allocation_granularity;
            };

            bool init(args& init_args);

            void deinit();

            std::optional<objects::object> allocate_object(objects::clazz cls);

            std::optional<objects::array> allocate_array(objects::clazz acls, std::uint64_t required_size);
        };
    }
}
#endif /* MEMORY_OLD_HEAP */
