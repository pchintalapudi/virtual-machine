#include "vm.h"

using namespace oops::virtual_machine;

oops::objects::clazz virtual_machine::lookup_class_offset(oops::objects::clazz cls, std::uint32_t class_offset)
{
    auto optimistic = cls.lookup_class_offset(class_offset);
    if (std::holds_alternative<objects::clazz>(optimistic))
    {
        return std::get<objects::clazz>(optimistic);
    }
    else
    {
        auto clazz = this->class_manager.load_class(std::get<utils::ostring>(optimistic));
        cls.dynamic_loaded_class(class_offset, clazz);
        return clazz;
    }
}

std::optional<oops::objects::method> virtual_machine::lookup_method_offset(oops::objects::clazz cls, std::uint32_t method_offset)
{
    auto optimistic = cls.lookup_method_offset(method_offset);
    if (std::holds_alternative<oops::objects::method>(optimistic))
    {
        return std::get<objects::method>(optimistic);
    }
    else
    {
        auto method_data = std::get<std::pair<std::uint32_t, utils::ostring>>(optimistic);
        auto method_class = this->lookup_class_offset(cls, method_data.first);
        auto maybe_method_offset = method_class.lookup_interface_method(method_data.second);
        if (maybe_method_offset)
        {
            auto method = std::get<objects::method>(method_class.lookup_method_offset(*maybe_method_offset));
            cls.dynamic_loaded_method(method_offset, method);
            return method;
        }
        return {};
    }
}

template <typename type>
bool virtual_machine::virtual_store(std::uint16_t object_offset, std::uint16_t src_offset, std::uint32_t idx24)
{
    auto cls = this->current_class();
    std::uint32_t virtual_field;
    auto optimistic = cls.lookup_virtual_field_offset(idx24);
    if (std::holds_alternative<std::uint32_t>(optimistic)) {
        virtual_field = std::get<std::uint32_t>(optimistic);
    } else {
        auto field_data = std::get<std::pair<std::uint32_t, utils::ostring>>(optimistic);
        auto field_class = this->lookup_class_offset(cls, field_data.first);
        auto maybe_field = field_class.lookup_interface_field(field_data.second);
        if (!maybe_field) return false;
        virtual_field = *maybe_field;
        cls.dynamic_loaded_virtual_field(idx24, virtual_field);
    }
    auto object = this->frame.read<objects::object>(object_offset);
    if constexpr (std::is_same_v<type, objects::base_object>)
    {
        auto value = this->frame.read<objects::base_object>(src_offset);
        object.write(virtual_field, value);
        this->write_barrier(object, value);
    }
    else
    {
        auto value = this->frame.read<std::common_type_t<std::int32_t, type>>(src_offset);
        object.write<type>(virtual_field, value);
    }
    return true;
}

template <typename type>
bool virtual_machine::virtual_load(std::uint16_t object_offset, std::uint16_t dest_offset, std::uint32_t idx24)
{
    auto cls = this->current_class();
    std::uint32_t virtual_field;
    auto optimistic = cls.lookup_virtual_field_offset(idx24);
    if (std::holds_alternative<std::uint32_t>(optimistic)) {
        virtual_field = std::get<std::uint32_t>(optimistic);
    } else {
        auto field_data = std::get<std::pair<std::uint32_t, utils::ostring>>(optimistic);
        auto field_class = this->lookup_class_offset(cls, field_data.first);
        auto maybe_field = field_class.lookup_interface_field(field_data.second);
        if (!maybe_field) return false;
        virtual_field = *maybe_field;
        cls.dynamic_loaded_virtual_field(idx24, virtual_field);
    }
    auto object = this->frame.read<objects::object>(object_offset);
    if constexpr (std::is_same_v<type, objects::base_object>)
    {
        auto value = object.read<objects::base_object>(virtual_field);
        this->frame.write(dest_offset, value);
    }
    else
    {
        auto value = this->frame.read<type>(virtual_field);
        this->frame.write<std::common_type_t<std::int32_t, type>>(dest_offset, value);
    }
    return true;
}

template<typename type>
bool virtual_machine::static_store(std::uint16_t src_offset, std::uint32_t dest31) {
    auto cls = this->current_class();
    std::uint32_t static_field;
    auto optimistic = cls.lookup_static_field_offset(dest31);
    auto field_class = this->lookup_class_offset(cls, optimistic.first);
    if (std::holds_alternative<std::uint32_t>(optimistic.second)) {
        static_field = std::get<std::uint32_t>(optimistic.second);
    } else {
        auto field_data = std::get<utils::ostring>(optimistic.second);
        auto maybe_field = field_class.lookup_static_interface_field(field_data);
        if (!maybe_field) return false;
        static_field = *maybe_field;
        cls.dynamic_loaded_static_field(dest31, static_field);
    }
    if constexpr (std::is_same_v<objects::base_object, type>) {
        auto value = this->frame.read<objects::base_object>(src_offset);
        cls.write(static_field, value);
        this->write_barrier(cls, value);
    } else {
        auto value = this->frame.read<std::common_type_t<std::int32_t, type>>(src_offset);
        cls.write<type>(static_field, value);
    }
    return true;
}

template<typename type>
bool virtual_machine::static_load(std::uint16_t dest_offset, std::uint32_t dest31) {
    auto cls = this->current_class();
    std::uint32_t static_field;
    auto optimistic = cls.lookup_static_field_offset(dest31);
    auto field_class = this->lookup_class_offset(cls, optimistic.first);
    if (std::holds_alternative<std::uint32_t>(optimistic.second)) {
        static_field = std::get<std::uint32_t>(optimistic.second);
    } else {
        auto field_data = std::get<utils::ostring>(optimistic.second);
        auto maybe_field = field_class.lookup_static_interface_field(field_data);
        if (!maybe_field) return false;
        static_field = *maybe_field;
        cls.dynamic_loaded_static_field(dest31, static_field);
    }
    if constexpr (std::is_same_v<objects::base_object, type>) {
        auto value = cls.read<type>(static_field);
        this->frame.write(dest_offset, value);
    } else {
        auto value = cls.read<type>(static_field);
        this->frame.write<std::common_type_t<type, std::int32_t>>(dest_offset, value);
    }
    return true;
}

#define fwd_vlsr(type) \
    template bool virtual_machine::virtual_store<type>(std::uint16_t object_offset, std::uint16_t src_offset, std::uint32_t idx24);
fwd_vlsr(std::int8_t);
fwd_vlsr(std::int16_t);
fwd_vlsr(std::int32_t);
fwd_vlsr(std::int64_t);
fwd_vlsr(float);
fwd_vlsr(double);
fwd_vlsr(oops::objects::base_object);
#undef fwd_vlsr

#define fwd_vlld(type) \
    template bool virtual_machine::virtual_load<type>(std::uint16_t object_offset, std::uint16_t src_offset, std::uint32_t idx24);
fwd_vlld(std::int8_t);
fwd_vlld(std::int16_t);
fwd_vlld(std::int32_t);
fwd_vlld(std::int64_t);
fwd_vlld(float);
fwd_vlld(double);
fwd_vlld(oops::objects::base_object);
#undef fwd_vlld

#define fwd_stsr(type) \
    template bool virtual_machine::static_store<type>(std::uint16_t src_offset, std::uint32_t field_idx);
fwd_stsr(std::int8_t);
fwd_stsr(std::int16_t);
fwd_stsr(std::int32_t);
fwd_stsr(std::int64_t);
fwd_stsr(float);
fwd_stsr(double);
fwd_stsr(oops::objects::base_object);
#undef fwd_stsr

#define fwd_stld(type) \
    template bool virtual_machine::static_load<type>(std::uint16_t dest_offset, std::uint32_t field_idx);
fwd_stld(std::int8_t);
fwd_stld(std::int16_t);
fwd_stld(std::int32_t);
fwd_stld(std::int64_t);
fwd_stld(float);
fwd_stld(double);
fwd_stld(oops::objects::base_object);
#undef fwd_stld