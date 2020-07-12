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
        public:
            enum class location
            {
                EDEN,
                SURVIVOR,
                TENURED
            };

        private:
            young_heap young_generation;
            old_heap old_generation;

            std::uint32_t max_young_object_size, max_young_gc_cycles;

            std::unordered_set<char *> forward_references, back_references;

        public:
            std::optional<objects::object> allocate_object(objects::clazz cls);

            std::optional<objects::array> allocate_array(objects::clazz acls, std::uint64_t memory_size);

            void write_barrier(char *dest, char *obj);

            std::pair<std::optional<objects::base_object>, location> gc_move_young(objects::base_object);
            void gc_move_old(objects::base_object);
            bool is_old_object(objects::base_object);
            auto &gc_forward_references() const
            {
                return this->forward_references;
            }
            auto &gc_backward_references() const
            {
                return this->forward_references;
            }
        };
    } // namespace memory
} // namespace oops

#endif /* MEMORY_HEAP */
