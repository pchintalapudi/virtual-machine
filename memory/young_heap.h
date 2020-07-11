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

        public:
            std::optional<objects::object> allocate_object(objects::clazz cls);

            std::optional<objects::array> allocate_array(objects::clazz acls, std::uint64_t memory_size);
        };
    } // namespace memory
} // namespace oops

#endif /* MEMORY_YOUNG_HEAP */
