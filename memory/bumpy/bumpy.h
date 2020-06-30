#ifndef MEMORY_BUMPY_BUMPY_H
#define MEMORY_BUMPY_BUMPY_H
#include "../objects.h"
namespace oops
{
    namespace memory
    {
        class memory_manager;
        class heap;
        class eden_heap
        {
        private:
            char *base, *head, *committed, *cap;
            friend class memory_manager;
            friend class heap;
        public:
            char *allocate_object(std::uint32_t compressed_size)
            {
                std::uint64_t expanded_size = objects::object::size32to64(expanded_size);
                std::uint64_t space = committed - head;
                if (space >= expanded_size + sizeof(std::uint32_t) * 2) {
                    char* ret = head;
                    std::memcpy(head - 4, &compressed_size, sizeof(compressed_size));
                    head += expanded_size;
                    std::memcpy(head, &compressed_size, sizeof(compressed_size));
                    head += sizeof(std::uint32_t) * 2;
                    return ret;
                }
                return nullptr;
            }
        };
    } // namespace memory
} // namespace oops
#endif