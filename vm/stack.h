#ifndef VM_STACK_H
#define VM_STACK_H

#include <cstdint>
#include <cstddef>
#include <type_traits>

#include <windows.h>

namespace oops
{
    namespace stack
    {
        namespace frame
        {
            /*
            struct frame {
                std::uint32_t frame_size;
                std::uint32_t handle_count;
                frame* previous;
                method* method;
                object* handles[handle_count];
                primitive primitives[frame_size - 2 * sizeof(std::uint32_t) - sizeof(frame*) - handle_count * sizeof(object*)];
            };
            */

        } // namespace frame
        struct stack_args
        {
            std::size_t bump_size;
            std::size_t max_size;
            std::size_t min_size;
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
                char *next_page = static_cast<char *>(VirtualAlloc(this->committed, amount, MEM_COMMIT, PAGE_READWRITE));
                if (!next_page)
                {
                    return false;
                }
                this->committed = next_page + amount;
                return true;
            }

        public:
            stack(const stack_args &)
            {
                //TODO
            }

            template <typename primitive>
            typename std::enable_if_t<std::is_signed<primitive>::value, primitive> read_primitive(std::uint16_t offset)
            {
                primitive p;
                std::memcpy(&p, this->head + offset, sizeof(primitive));
                return p;
            }

            template <typename primitive>
            typename std::enable_if_t<std::is_signed<primitive>::value, void> write_primitive(std::uint16_t offset, primitive p)
            {
                std::memcpy(this->head + offset, &p, sizeof(primitive));
            }

            char *read_pointer(std::uint16_t offset)
            {
                char *p;
                std::memcpy(&p, this->head + offset, sizeof(char *));
                return p;
            }

            void write_pointer(std::uint16_t offset, char *p)
            {
                std::memcpy(this->head + offset, &p, sizeof(char *));
            }

            template <typename primitive>
            typename std::enable_if_t<std::is_signed<primitive>::value, primitive> read(std::uint16_t offset)
            {
                return this->read_primitive<primitive>(offset);
            }

            template <typename pointer>
            typename std::enable_if_t<std::is_same<pointer, char *>::value, pointer> read(std::uint16_t offset)
            {
                return this->read_pointer(offset);
            }

            template <typename primitive>
            typename std::enable_if_t<std::is_signed<primitive>::value, void> write(std::uint16_t offset, primitive p)
            {
                this->write_primitive(offset, p);
            }

            template <typename pointer>
            typename std::enable_if_t<std::is_same<pointer, char *>::value, void> write(std::uint16_t offset, pointer p)
            {
                this->write_pointer(offset, p);
            }
        };
    } // namespace stack
} // namespace oops
#endif