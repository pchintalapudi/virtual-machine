#ifndef MEMORY_HEAP_H
#define MEMORY_HEAP_H

#include <set>
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

        class heap
        {
        private:
            //Basic heap variables
            char *base;
            char *cap;
            std::size_t bump_size;
            std::size_t massive_cutoff;

            struct miniheap
            {
                char *head, *cap;
            };

            enum struct generation
            {
                METASPACE,
                TENURED,
                SURVIVOR,
                EDEN,
                __COUNT__
            };

            friend class memory_manager;

            miniheap generations[static_cast<int>(generation::__COUNT__)];
            std::set<objects::object> forward_references;

            miniheap &_generation(generation g)
            {
                return this->generations[static_cast<int>(g)];
            }

            char *generation_cap(char *pointer)
            {
                return pointer < generations[1].cap ? pointer < generations[0].cap ? generations[0].cap : generations[1].cap : generations[3].cap;
            }

            bool _commit(std::size_t amount)
            {
                return VirtualAlloc(this->_generation(generation::EDEN).cap, amount, MEM_COMMIT, PAGE_READWRITE) != NULL;
            }

            bool request(std::size_t necessary)
            {
                return this->_commit((necessary + this->bump_size - 1) / this->bump_size);
            }

            //Does the heavy lifting of garbage collection as necessary to fulfill the request (clears memory if gc happens)
            bool provision(generation location, std::size_t amount);

        public:
            bool init(const heap_args &args)
            {
                this->bump_size = args.bump_size;
                this->massive_cutoff = args.massive_cutoff;
                this->base = static_cast<char *>(VirtualAlloc(nullptr, args.max_size, MEM_RESERVE, PAGE_READWRITE));
                if (!this->base)
                    return false;
                this->cap = this->base + args.max_size;
                std::size_t next_size = args.min_size - args.tenured_size - args.survivor_size - args.eden_size;
                this->_generation(generation::METASPACE) = {this->base, this->base + next_size};
                this->_generation(generation::TENURED) = {this->base + next_size, this->base + (next_size += args.tenured_size)};
                this->_generation(generation::SURVIVOR) = {this->base + next_size, this->base + (next_size += args.survivor_size)};
                this->_generation(generation::EDEN) = {this->base + next_size, this->base};
                this->_commit(args.min_size);
                this->_generation(generation::EDEN).cap += args.min_size;
                return true;
            }

            void write_barrier(objects::object object, objects::object write)
            {
                if (write > object and write.unwrap() > this->generation_cap(object.unwrap()))
                    this->forward_references.insert(object);
            }

            bool allocate(objects::clazz clz)
            {
                std::uint64_t size = clz.size();
                generation location = size < this->massive_cutoff ? generation::EDEN : generation::TENURED;
                if (!this->provision(location, size)) return false;
                auto object = objects::object(this->_generation(location).head);
                this->_generation(location).head += size;
                object.construct(clz);
                return true;
            }
        };
    } // namespace memory
} // namespace oops
#endif