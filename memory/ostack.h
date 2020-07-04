#ifndef MEMORY_STACK
#define MEMORY_STACK

#include "../objects/objects.h"

namespace oops
{
    namespace memory
    {

        class frame
        {
        private:
            std::uintptr_t r0, r1, r2, r3;
            char *real;

        public:
            template <typename primitive>
            std::enable_if_t<std::is_signed_v<primitive>, primitive> read(std::uint16_t offset)
            {
                switch (offset)
                {
                case 0:
                    return static_cast<primitive>(r0);
                case 1:
                    return static_cast<primitive>(r1);
                case 2:
                    return static_cast<primitive>(r2);
                case 3:
                    return static_cast<primitive>(r3);
                default:
                    //TODO calc offsets
                    break;
                }
            }

            template <typename pointer>
            std::enable_if_t<std::is_base_of_v<objects::base_object, pointer>, pointer> read(std::uint16_t offset)
            {
                switch (offset)
                {
                case 0:
                    return pointer(reinterpret_cast<char *>(r0));
                case 1:
                    return pointer(reinterpret_cast<char *>(r1));
                case 2:
                    return pointer(reinterpret_cast<char *>(r2));
                case 3:
                    return pointer(reinterpret_cast<char *>(r3));
                default:
                    //TODO calc offsets
                    break;
                }
            }

            template <typename primitive>
            std::enable_if_t<std::is_signed_v<primitive>, void> write(std::uint16_t offset, primitive value)
            {
                switch (offset)
                {
                case 0:
                    this->r0 = static_cast<primitive>(value);
                    break;
                case 1:
                    this->r1 = static_cast<primitive>(value);
                    break;
                case 2:
                    this->r2 = static_cast<primitive>(value);
                    break;
                case 3:
                    this->r3 = static_cast<primitive>(value);
                    break;
                default:
                    //TODO calc offsets
                    break;
                }
            }

            template <typename pointer>
            std::enable_if_t<std::is_base_of_v<objects::base_object, pointer>, void> write(std::uint16_t offset, pointer obj)
            {
                switch (offset)
                {
                case 0:
                    this->r0 = reinterpret_cast<std::uintptr_t>(obj.unwrap());
                    break;
                case 1:
                    this->r1 = reinterpret_cast<std::uintptr_t>(obj.unwrap());
                    break;
                case 2:
                    this->r2 = reinterpret_cast<std::uintptr_t>(obj.unwrap());
                    break;
                case 3:
                    this->r3 = reinterpret_cast<std::uintptr_t>(obj.unwrap());
                    break;
                default:
                    //TODO calc offsets
                    break;
                }
            }
        };

        class stack
        {
        private:
        public:
            frame init(std::uint16_t frame_size);

            bool save_and_push(frame &frame);

            bool load_and_pop(frame &frame);
        };
    } // namespace memory
} // namespace oops
#endif /* MEMORY_STACK */
