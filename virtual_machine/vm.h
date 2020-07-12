#ifndef VIRTUAL_MACHINE_VM
#define VIRTUAL_MACHINE_VM

#include "../memory/memory.h"
#include "../interface/interfaces.h"

namespace oops
{
    namespace virtual_machine
    {

        class result
        {
        private:
            template <typename test, typename... allowed>
            constexpr static bool contains()
            {
                return (std::is_same_v<test, allowed> or ...);
            }
            union {
                std::int32_t int_value;
                std::int64_t long_value;
                float float_value;
                double double_value;
                char *object_value;
            };
            int status;
            objects::field::type type;

        public:
            template <typename real_type, typename =
                                              std::enable_if_t<result::contains<real_type, std::int32_t, std::int64_t, float, double, objects::base_object>(), result>>
            result(real_type value, int status) : status(status)
            {
                if constexpr (std::is_same_v<real_type, objects::base_object>)
                {
                    this->object_value = value.unwrap();
                    this->type = objects::field::type::OBJECT;
                }
                else if constexpr (std::is_same_v<real_type, std::int32_t>)
                {
                    this->int_value = value;
                    this->type = objects::field::type::INT;
                }
                else if constexpr (std::is_same_v<real_type, std::int64_t>)
                {
                    this->long_value = value;
                    this->type = objects::field::type::LONG;
                }
                else if constexpr (std::is_same_v<real_type, float>)
                {
                    this->float_value = value;
                    this->type = objects::field::type::FLOAT;
                }
                else
                {
                    this->double_value = value;
                    this->type = objects::field::type::DOUBLE;
                }
            }

            int get_status() const
            {
                return status;
            }

            objects::field::type get_type() const
            {
                return type;
            }

            template <typename return_type>
            std::enable_if_t<result::contains<return_type, std::int32_t, std::int64_t, float, double, objects::base_object>(), result> get_value() const
            {
                if constexpr (std::is_same_v<return_type, objects::base_object>)
                {
                    return objects::base_object(this->object_value);
                }
                else if constexpr (std::is_same_v<return_type, std::int32_t>)
                {
                    return this->int_value;
                }
                else if constexpr (std::is_same_v<return_type, std::int64_t>)
                {
                    return this->long_value;
                }
                else if constexpr (std::is_same_v<return_type, float>)
                {
                    return this->float_value;
                }
                else
                {
                    return this->double_value;
                }
            }
        };
        class virtual_machine
        {
        private:
            char *ip;
            memory::frame frame;
            memory::stack stack;
            memory::heap heap;
            interfaze::class_manager class_manager;
            objects::clazz array_classes[static_cast<std::uint8_t>(objects::field::type::VOID)];

            std::unordered_set<char*> static_references;

            result exec_loop();

            objects::clazz current_class();

            bool instanceof(objects::clazz src, objects::clazz test);

            std::optional<objects::method> lookup_interface_method(objects::method imethod, objects::base_object src);

            std::optional<objects::object> new_object(objects::clazz cls);

            std::optional<objects::array> new_array(objects::field::type array_type, std::uint32_t length);

            template<typename to, typename move>
            void write_barrier(to dest, move obj) {
                if constexpr (std::is_same_v<to, objects::clazz> and std::is_base_of_v<objects::base_object, move>) {
                    static_references.insert(dest.unwrap());
                } else if constexpr (std::is_base_of_v<objects::base_object, to> and std::is_base_of_v<objects::base_object, move>) {
                    heap.write_barrier(dest.unwrap(), obj.unwrap());
                }
            }

            void gc(bool force_old=false);

            bool young_gc();

            void old_gc();

        public:
            int execute(objects::method method);
        };
    } // namespace virtual_machine
} // namespace oops

#endif /* VIRTUAL_MACHINE_VM */
