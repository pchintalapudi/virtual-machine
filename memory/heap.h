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

            std::uint32_t max_young_object_size, max_young_gc_cycles;

            std::unordered_set<char*> cross_generational_relationships;

            enum class location {
                EDEN, SURVIVOR, TENURED, FORWARDED
            };

            bool survivor(objects::base_object);
            std::pair<std::optional<objects::base_object>, location> gc_move_young(objects::base_object);

        public:
            std::optional<objects::object> allocate_object(objects::clazz cls);

            std::optional<objects::array> allocate_array(objects::clazz acls, std::uint64_t memory_size);

            void write_barrier(char *dest, char *obj)
            {
                if (dest < old_generation.cap and obj >= old_generation.cap) {
                    cross_generational_relationships.insert(dest);
                }
            }
        };
    } // namespace memory
} // namespace oops

#endif /* MEMORY_HEAP */
