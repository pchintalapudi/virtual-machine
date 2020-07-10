#ifndef MEMORY_HEAP
#define MEMORY_HEAP

#include "../objects/objects.h"
#include "young_heap.h"
#include "old_heap.h"

#include <unordered_set>

namespace oops
{
    namespace memory
    {
        class heap
        {
        private:
            young_heap young_generation;
            old_heap old_generation;

            std::size_t max_young_object_size;

            std::unordered_set<char*> cross_generational_relationships;

        public:
            std::optional<objects::object> allocate_object(objects::clazz cls);

            std::optional<objects::array> allocate_array(objects::clazz acls, std::uint64_t memory_size);

            void write_barrier(char *from, char *obj)
            {
                if (from >= old_generation.cap) {
                    cross_generational_relationships.insert(obj);
                }
            }
        };
    } // namespace memory
} // namespace oops

#endif /* MEMORY_HEAP */
