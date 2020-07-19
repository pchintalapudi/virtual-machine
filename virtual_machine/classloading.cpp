#include "vm.h"

using namespace oops::virtual_machine;

oops::objects::clazz virtual_machine::lookup_class_offset(oops::objects::clazz cls, std::uint32_t class_offset) {
    auto optimistic = cls.lookup_class_offset(class_offset);
    if (std::holds_alternative<objects::clazz>(optimistic)) {
        return std::get<objects::clazz>(optimistic);
    } else {
        auto clazz = this->class_manager.load_class(std::get<utils::ostring>(optimistic));
        cls.dynamic_loaded_class(class_offset, clazz);
        return clazz;
    }
}

std::optional<oops::objects::method> virtual_machine::lookup_method_offset(oops::objects::clazz cls, std::uint32_t method_offset) {
    auto optimistic = cls.lookup_method_offset(method_offset);
    if (std::holds_alternative<oops::objects::method>(optimistic)) {
        return std::get<objects::method>(optimistic);
    } else {
        auto method_data = std::get<std::pair<std::uint32_t, utils::ostring>>(optimistic);
        auto method_class = this->lookup_class_offset(cls, method_data.first);
        auto maybe_method_offset = method_class.lookup_interface_method(method_data.second);
        if (maybe_method_offset) {
            auto method = std::get<objects::method>(method_class.lookup_method_offset(*maybe_method_offset));
            cls.dynamic_loaded_method(method_offset, method);
            return method;
        }
        return {};
    }
}