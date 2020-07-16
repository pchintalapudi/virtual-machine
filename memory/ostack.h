#ifndef MEMORY_STACK
#define MEMORY_STACK

#include "../objects/objects.h"

namespace oops
{
    namespace memory
    {

        class stack;

        class frame
        {
        private:

            friend class stack;
            char *real;

            char* data_start() const;

            std::uint16_t return_offset() const;

            std::uint16_t return_address() const;

            std::uint16_t prev_size() const;

            bool prev_native() const;

        public:

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

            std::optional<frame> previous() const;
        };

        class stack
        {
        private:
            char *base, *head, *cap;
            std::uint16_t current_size;

        public:

            bool init_frame(frame &frame, objects::method method, std::uint16_t return_offset, bool native_root, char* ip_next);

            template<typename result_type>
            char* load_and_pop(frame &frame, result_type result);

            bool is_root(frame &frame);
        };
    } // namespace memory
} // namespace oops
#endif /* MEMORY_STACK */
