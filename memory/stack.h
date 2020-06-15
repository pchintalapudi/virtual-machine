#ifndef MEMORY_STACK_H
#define MEMORY_STACK_H

#include <cstdint>
#include <cstddef>
#include <type_traits>

#include <windows.h>

#include "objects.h"

namespace oops
{
    namespace memory
    {

        class memory_manager;
        struct stack_args
        {
            std::size_t bump_size;
            std::size_t max_size;
            std::size_t min_size;
        };

        class stack;

        class frame : public objects::aliased<std::uint16_t>
        {
        private:
            void construct(std::uint16_t size, std::uint16_t handle_count, std::uint16_t return_offset, objects::field::field_type return_type, frame prev, objects::method method)
            {
                std::memcpy(this->real, &size, sizeof(size));
                std::memcpy(this->real + sizeof(std::uint16_t), &handle_count, sizeof(handle_count));
                std::memcpy(this->real + sizeof(std::uint16_t) * 2, &return_offset, sizeof(return_offset));
                std::uint16_t rt = static_cast<std::uint16_t>(return_type);
                std::memcpy(this->real + sizeof(std::uint16_t) * 3, &rt, sizeof(std::uint16_t));
                //Return address set later
                char *prev_frame = prev.unwrap();
                std::memcpy(this->real + sizeof(std::uint16_t) * 4 + sizeof(char *), &prev_frame, sizeof(char *));
                char* mtd = method.unwrap();
                std::memcpy(this->real + sizeof(std::uint16_t) * 4 + sizeof(char *) * 2, &mtd, sizeof(char *));
            }

            void set_return_address(objects::instruction return_address) {
                char* ra = return_address.unwrap();
                std::memcpy(this->real + sizeof(std::uint16_t) * 4, &ra, sizeof(char*));
            }

            friend class stack;
            friend class memory_manager;

        public:
            explicit frame(char *real) : aliased(real) {}

            std::uint16_t return_offset() const
            {
                PUN(std::uint16_t, ra, this->real + sizeof(std::uint16_t) * 2);
                return ra;
            }

            objects::field::field_type return_type() {
                PUN(std::uint16_t, rt, this->real + sizeof(std::uint16_t) * 3);
                return static_cast<objects::field::field_type>(rt);
            }

            objects::instruction return_instruction() const
            {
                char *npc;
                std::memcpy(&npc, this->real + sizeof(std::uint16_t) * 4, sizeof(char *));
                return objects::instruction(npc);
            }

            maybe<frame> prev_frame() const
            {
                char *prev;
                std::memcpy(&prev, this->real + sizeof(std::uint16_t) * 4 + sizeof(char *), sizeof(char *));
                return {prev == nullptr, frame(prev)};
            }

            objects::method lookup_method() const
            {
                PUN(char*, mtd, this->real + sizeof(std::uint16_t) * 4 + sizeof(char*) * 2);
                return objects::method(mtd);
            }

            template <typename object>
            std::enable_if_t<std::is_same<object, objects::object>::value, object> read(std::uint16_t offset) const
            {
                char *obj;
                std::memcpy(&obj, this->real + sizeof(std::uint16_t) * 4 + sizeof(char *) * 3 + offset, sizeof(char *));
                return object(obj);
            }

            template <typename primitive>
            std::enable_if_t<std::is_signed<primitive>::value, primitive> read(std::uint16_t offset) const
            {
                primitive p;
                std::memcpy(&p, this->real + sizeof(std::uint16_t) * 4 + sizeof(char *) * 3 + offset, sizeof(primitive));
                return p;
            }

            template <typename object>
            std::enable_if_t<std::is_same<object, objects::object>::value, void> write(std::uint16_t offset, object obj)
            {
                char *ptr = obj.unwrap();
                std::memcpy(this->real + sizeof(std::uint16_t) * 4 + sizeof(char *) * 3 + offset, &ptr, sizeof(char *));
            }

            template <typename primitive>
            std::enable_if_t<std::is_signed<primitive>::value, void> write(std::uint16_t offset, primitive p)
            {
                std::memcpy(this->real + sizeof(std::uint16_t) * 4 + sizeof(char *) * 3 + offset, &p, sizeof(primitive));
            }
        };

        class stack
        {
        private:
            std::size_t bump_size;
            char *base, *head, *committed, *cap;

            bool commit_memory(std::size_t amount)
            {
                if (static_cast<std::size_t>(this->cap - this->committed) < amount)
                {
                    return false;
                }
                std::size_t allocation_size = ((amount + this->bump_size - 1) / this->bump_size) * this->bump_size;
                char *next_page = static_cast<char *>(VirtualAlloc(this->committed, allocation_size, MEM_COMMIT, PAGE_READWRITE));
                if (!next_page)
                {
                    return false;
                }
                this->committed = next_page + allocation_size;
                return true;
            }

            void pop_frame()
            {
                if (this->head != this->base + 1)
                {
                    this->head = this->head == this->base ? this->base + 1 : this->current_frame().prev_frame().value.unwrap();
                }
            }

            bool push_frame(std::uint16_t size, std::uint16_t handle_count, std::uint16_t return_offset, objects::field::field_type return_type, objects::method method)
            {
                size += sizeof(std::uint16_t) * 4 + sizeof(char*) * 3;
                char *prev = this->head;
                if (this->head - 1 == this->base)
                {
                    this->head = this->base;
                    prev = nullptr;
                }
                const std::uint16_t jump = this->current_frame().size();
                if (this->committed - this->head < size)
                {
                    if (!this->commit_memory(size))
                        return false;
                }
                this->head += jump;
                this->current_frame().construct(size, handle_count, return_offset, return_type, frame(prev), method);
                return true;
            }

            friend class memory_manager;

        public:
            int init(const stack_args &args)
            {
                this->bump_size = args.bump_size;
                this->head = this->base = static_cast<char *>(VirtualAlloc(nullptr, args.max_size, MEM_RESERVE, PAGE_READWRITE));
                if (!this->base)
                {
                    return -1;
                }
                this->cap = this->base + args.max_size;
                return this->commit_memory(args.min_size) ? 0 : -1;
            }

            frame current_frame()
            {
                return frame(this->head);
            }

            void unwind()
            {
                this->pop_frame();
            }
        };
    } // namespace memory
} // namespace oops
#endif