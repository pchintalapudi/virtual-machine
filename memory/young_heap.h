#ifndef MEMORY_YOUNG_HEAP
#define MEMORY_YOUNG_HEAP

#include "../objects/objects.h"

namespace oops
{
    namespace memory
    {
        class eden_heap
        {
        private:
            char *base, *head, *committed, *end;
            std::size_t page_size;

            bool grow(std::size_t page_count);

        public:
            std::optional<objects::object> allocate_object(objects::clazz cls);

            std::optional<objects::array> allocate_array(objects::clazz acls, std::uint64_t memory_size);

            ~eden_heap();
        };

        class survivor_heap
        {
        private:
            char *source_begin, *sink_begin;

        public:
        };

        class young_heap
        {
        private:
            eden_heap eden;
            survivor_heap survivors;

        public:
            std::optional<objects::object> allocate_object(objects::clazz cls);

            std::optional<objects::array> allocate_array(objects::clazz acls, std::uint64_t memory_size);
        };
    } // namespace memory
} // namespace oops

#endif /* MEMORY_YOUNG_HEAP */
