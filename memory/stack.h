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

        template <typename std_layout>
        struct maybe
        {
            bool present;
            std_layout value;

            explicit operator bool()
            {
                return this->present;
            }
        };

        class stack;

        class frame
        {
        public:
            typedef void *method;

        private:
            char *real;

            void construct(std::uint16_t size, std::uint16_t handle_count, std::uint16_t return_offset, std::uint16_t next_pc_offset, char *prev, frame::method method_info)
            {
                std::memcpy(this->real, &size, sizeof(std::uint16_t));
                std::memcpy(this->real + sizeof(std::uint16_t), &handle_count, sizeof(std::uint16_t));
                std::memcpy(this->real + sizeof(std::uint16_t) * 2, &return_offset, sizeof(std::uint16_t));
                std::memcpy(this->real + sizeof(std::uint16_t) * 3, &next_pc_offset, sizeof(std::uint16_t));
                std::memcpy(this->real + sizeof(std::uint16_t) * 4, &prev, sizeof(char *));
                std::memcpy(this->real + sizeof(std::uint16_t) * 4 + sizeof(char *), &method_info, sizeof(frame::method));
            }

            friend class stack;

        public:
            explicit frame(char *real) : real(real) {}

            char *unwrap()
            {
                return this->real;
            }

            std::uint16_t size() const
            {
                std::uint16_t sz;
                std::memcpy(&sz, this->real, sizeof(std::uint16_t));
                return sz;
            }

            std::uint16_t handle_count() const
            {
                std::uint16_t hc;
                std::memcpy(&hc, this->real + sizeof(std::uint16_t), sizeof(std::uint16_t));
                return hc;
            }

            std::uint16_t return_offset() const
            {
                std::uint16_t ra;
                std::memcpy(&ra, this->real + sizeof(std::uint16_t) * 2, sizeof(std::uint16_t));
                return ra;
            }

            std::uint16_t next_pc_offset() const
            {
                std::uint16_t npc;
                std::memcpy(&npc, this->real + sizeof(std::uint16_t) * 3, sizeof(std::uint16_t));
                return npc;
            }

            maybe<frame> prev_frame() const
            {
                char *prev;
                std::memcpy(&prev, this->real + sizeof(std::uint16_t) * 4, sizeof(char *));
                return {prev == nullptr, frame(prev)};
            }

            method method_info() const
            {
                method mti;
                std::memcpy(&mti, this->real + sizeof(std::uint16_t) * 4 + sizeof(char *), sizeof(method));
                return mti;
            }

            template <typename object>
            std::enable_if_t<std::is_same<object, objects::object>::value, object> read(std::uint16_t offset) const
            {
                char *obj;
                std::memcpy(&obj, this->real + sizeof(std::uint16_t) * 4 + sizeof(char *) + sizeof(void *) + offset, sizeof(char *));
                return object(obj);
            }

            template <typename primitive>
            std::enable_if_t<std::is_signed<primitive>::value, primitive> read(std::uint16_t offset) const
            {
                primitive p;
                std::memcpy(&p, this->real + sizeof(std::uint16_t) * 4 + sizeof(char *) + sizeof(void *) + offset, sizeof(primitive));
                return p;
            }

            template <typename object>
            std::enable_if_t<std::is_same<object, objects::object>::value, void> write(std::uint16_t offset, object obj)
            {
                char *ptr = obj.unwrap();
                std::memcpy(this->real + sizeof(std::uint16_t) * 4 + sizeof(char *) + sizeof(void *) + offset, &ptr, sizeof(char *));
            }

            template <typename primitive>
            std::enable_if_t<std::is_signed<primitive>::value, void> write(std::uint16_t offset, primitive p)
            {
                std::memcpy(this->real + sizeof(std::uint16_t) * 4 + sizeof(char *) + sizeof(void *) + offset, &p, sizeof(primitive));
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

            bool push_frame(std::uint16_t size, std::uint16_t handle_count, std::uint16_t return_address, std::uint16_t next_pc, frame::method method_info)
            {
                size += sizeof(std::uint16_t) * 4 + sizeof(frame *) + sizeof(frame::method);
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
                this->current_frame().construct(size, handle_count, return_address, next_pc, prev, method_info);
                return true;
            }

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