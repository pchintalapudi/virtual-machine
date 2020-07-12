#ifndef MEMORY_YOUNG_HEAP
#define MEMORY_YOUNG_HEAP

#include "../objects/objects.h"

namespace oops
{
    namespace memory
    {
        class heap;

        class young_heap
        {
        private:
            friend class heap;
            char *real_base, *real_cap;
            char *live_survivor_boundary, *dead_survivor_boundary;
            char *write_head;

            std::uint32_t survival_count(objects::base_object);
            std::pair<std::optional<objects::base_object>, bool> gc_save_young(objects::base_object obj);

        public:
            std::optional<objects::object> allocate_object(objects::clazz cls);

            std::optional<objects::array> allocate_array(objects::clazz acls, std::uint64_t memory_size);
        };
    } // namespace memory
} // namespace oops

#endif /* MEMORY_YOUNG_HEAP */
