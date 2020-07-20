#ifndef MEMORY_HEAP
#define MEMORY_HEAP

#include "../objects/objects.h"
#include "young_heap.h"
#include "old_heap.h"

#include <unordered_set>

namespace oops
{
    namespace virtual_machine {
        class virtual_machine;
    }
    namespace memory
    {
        class heap
        {
        public:
            enum class location
            {
                EDEN,
                SURVIVOR,
                TENURED,
                FORWARD_TENURED
            };

        private:
            friend class virtual_machine::virtual_machine;

            young_heap young_generation;
            old_heap old_generation;

            std::uint32_t max_young_object_size;

            std::unordered_set<char *> forward_references, back_references;
            std::vector<char*> finalizable;

            void prep_for_old_gc();

            bool prep_for_young_gc();

            void finish_young_gc();

            std::pair<std::optional<objects::base_object>, location> gc_move_young(objects::base_object);
            void gc_move_old(objects::base_object);
            bool is_old_object(objects::base_object);
            std::optional<objects::base_object> moved(objects::base_object obj) {
                return this->young_generation.gc_forwarded(obj);
            }

            typedef young_heap::walker young_walker;

            young_walker ybegin() {
                return this->young_generation.begin();
            }

            young_walker yend() {
                return this->young_generation.end();
            }

            void sweep();

        public:
            std::optional<objects::object> allocate_object(objects::clazz cls);

            std::optional<objects::array> allocate_array(objects::clazz acls, std::uint64_t memory_size);

            void write_barrier(char *dest, char *obj);
        };
    } // namespace memory
} // namespace oops

#endif /* MEMORY_HEAP */
