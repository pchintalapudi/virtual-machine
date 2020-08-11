#ifndef MEMORY_STACK
#define MEMORY_STACK

#include "../objects/objects.h"

namespace oops
{

    namespace virtual_machine {
        class result;
    }
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

        public:

            struct args {
                std::uint64_t stack_size;
            };

            bool init(args& init_args);

            void deinit();

            void load_first_frame(frame &frame, objects::method method, objects::array args);

            bool init_frame(frame &frame, objects::method method, std::uint16_t return_offset, char* ip_next);

            bool init_from_native_frame(frame& frame, objects::method method, const std::vector<virtual_machine::result>& args);

            template<typename result_type>
            char* load_and_pop(frame &frame, result_type result);
        };
    } // namespace memory
} // namespace oops
#endif /* MEMORY_STACK */
