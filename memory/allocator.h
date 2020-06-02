#ifndef MEMORY_ALLOCATOR_H
#define MEMORY_ALLOCATOR_H

#include <optional>
#include <vector>

//Windows virtual memory headers
#include "windows.h"
#include "memoryapi.h"

#include "objects.h"

namespace oops
{
    namespace memory
    {
        struct heap_args
        {
            std::size_t bump_size;
            std::size_t min_size;
            std::size_t max_size;
        };

        class heap
        {
        private:
            char *base, *head, *committed, *cap;
            std::vector<char *> allocated;
            std::size_t bump_size;

            bool commit_memory(std::size_t amount)
            {
                if (static_cast<std::size_t>(this->cap - this->committed) < amount)
                {
                    return false;
                }
                char *next_page = static_cast<char *>(VirtualAlloc(this->committed, amount, MEM_COMMIT, PAGE_READWRITE));
                if (!next_page)
                {
                    return false;
                }
                this->committed = next_page + amount;
                return true;
            }

        public:
            
            int init(const heap_args& args) {
                this->bump_size = args.bump_size;
                this->head = this->base = static_cast<char *>(VirtualAlloc(nullptr, args.max_size, MEM_RESERVE, PAGE_READWRITE));
                if (!this->base)
                {
                    return -1;
                }
                this->cap = this->base + args.max_size;
                return this->commit_memory(args.min_size) ? 0 : -1;
            }

            void *try_allocate(std::size_t size)
            {
                if (static_cast<std::size_t>(this->committed - this->head) > size)
                {
                    allocated.push_back(this->head);
                    this->head += size;
                    return allocated.back();
                }
                return nullptr;
            }

            void *bump_allocate_unsafe(std::size_t size)
            {
                if (this->commit_memory(bump_size))
                {
                    allocated.push_back(this->head);
                    this->head += size;
                    return allocated.back();
                }
                else
                {
                    return nullptr;
                }
            }

            ~heap()
            {
                if (this->base)
                {
                    VirtualFree(this->base, 0, MEM_RELEASE);
                }
            }
        };
    } // namespace memory
} // namespace oops
#endif