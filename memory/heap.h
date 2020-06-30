#ifndef MEMORY_HEAP_H
#define MEMORY_HEAP_H
#include <set>
#include "bumpy/bumpy.h"
#include "smalloc/smalloc.h"
//Windows virtual memory headers
#include "windows.h"
#include "memoryapi.h"

#include "objects.h"

namespace oops
{
    namespace memory
    {
        class memory_manager;

        struct heap_args
        {
            std::size_t bump_size;
            std::size_t min_size;
            std::size_t max_size;

            //Initial sizes
            std::size_t eden_size;
            std::size_t survivor_size;
            std::size_t tenured_size;
            //Metaspace size is determined by clamping to min size

            //Massive objects are immediately tenured
            std::size_t massive_cutoff;
        };

        class heap {
            private:
            old_heap tenured_generation;
            eden_heap eden_generation;
            std::set<objects::object> forward_references;

            public:
            int init(const heap_args& heap_args);

            void write_barrier(objects::object object, objects::object write) {
                if (write > object and write.unwrap() > this->eden_generation.base) this->forward_references.insert(object);
            }

            maybe<objects::object> allocate(objects::clazz clz);
        };
    } // namespace memory
} // namespace oops
#endif