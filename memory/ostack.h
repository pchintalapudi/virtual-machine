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
            char *real;

            char* data_start() const;

        public:
            

            void set_return_address(std::uint16_t return_address);

            template <typename primitive>
            std::enable_if_t<std::is_signed_v<primitive>, primitive> read(std::uint16_t offset) {
                return utils::pun_read<primitive>(this->data_start() + static_cast<std::uint32_t>(offset) * sizeof(std::uint32_t));
            }

            template <typename pointer>
            std::enable_if_t<std::is_base_of_v<objects::base_object, pointer>, pointer> read(std::uint16_t offset) {
                return utils::pun_read<char*>(this->data_start() + static_cast<std::uint32_t>(offset) * sizeof(std::uint32_t));
            }

            template <typename primitive>
            std::enable_if_t<std::is_signed_v<primitive>, void> write(std::uint16_t offset, primitive value) {
                utils::pun_write(this->data_start() + static_cast<std::uint32_t>(offset) * sizeof(std::uint32_t), value);
            }

            template <typename pointer>
            std::enable_if_t<std::is_base_of_v<objects::base_object, pointer>, void> write(std::uint16_t offset, pointer obj) {
                utils::pun_write(this->data_start() + static_cast<std::uint32_t>(offset) * sizeof(std::uint32_t), obj.unwrap());
            }

            objects::method get_method() const;

            std::uint16_t return_offset() const;

            std::uint16_t return_address() const;

            std::uint16_t size() const;
        };

        class stack
        {
        private:
            char *base, *head, *cap;

        public:
            frame init(std::uint16_t frame_size);

            bool init_frame(frame &frame, objects::method method, std::uint16_t return_offset);

            bool load_and_pop(frame &frame);
        };
    } // namespace memory
} // namespace oops
#endif /* MEMORY_STACK */
