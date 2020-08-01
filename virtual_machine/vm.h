#ifndef VIRTUAL_MACHINE_VM
#define VIRTUAL_MACHINE_VM

#include "../memory/memory.h"
#include "../interface/class_manager.h"

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

            objects::clazz lookup_class_offset(objects::clazz cls, std::uint32_t class_offset);

            std::optional<objects::method> lookup_method_offset(objects::clazz cls, std::uint32_t method_offset);

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

            template<typename container>
            bool young_gc_base_object(objects::base_object, container&);

            template<typename container>
            std::optional<objects::base_object> young_gc_obj(objects::base_object, container&);

            void mark(objects::base_object);

            template<typename type>
            bool virtual_load(std::uint16_t object_offset, std::uint16_t dest_offset, std::uint32_t idx24);
            template<typename type>
            bool virtual_store(std::uint16_t object_offset, std::uint16_t src_offset, std::uint32_t dest24);
            template<typename type>
            bool static_load(std::uint16_t dest_offset, std::uint32_t idx31);
            template<typename type>
            bool static_store(std::uint16_t src_offset, std::uint32_t dest31);

        public:

            struct args {
                memory::heap::args heap_args;
                memory::stack::args stack_args;
                interfaze::class_manager::args class_manager_args;
            };

            bool init(args& init_args) {
                if (this->heap.init(init_args.heap_args)) {
                    if (this->stack.init(init_args.stack_args)) {
                        if (this->class_manager.init(init_args.class_manager_args)) {
                            return true;
                        }
                        this->stack.deinit();
                    }
                    this->heap.deinit();
                }
                return false;
            }

            void deinit() {
                this->class_manager.deinit();
                this->stack.deinit();
                this->heap.deinit();
            }

            int vm_core_startup(const std::vector<utils::ostring>& args);

            result execute(objects::method method);

            void dump();
        };
    } // namespace virtual_machine
} // namespace oops

#endif /* VIRTUAL_MACHINE_VM */
