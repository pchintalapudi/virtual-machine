#include "class.h"

using namespace oops::objects;

clazz clazz::lookup_class(std::uint32_t class_offset) const
{
    auto resolved_class_location = this->resolved_class_start() + static_cast<std::uint64_t>(class_offset) * sizeof(char *);
    if (auto preresolved = utils::pun_read<char *>(resolved_class_location); preresolved)
    {
        return clazz(preresolved);
    }
    auto resolved = this->resolve_symbolic_class(class_offset);
    utils::pun_write(resolved_class_location, resolved.unwrap());
    return resolved;
}

method clazz::lookup_method(std::uint32_t method_offset) const {
    auto resolved_method_location = this->resolved_method_start() + static_cast<std::uint64_t>(method_offset) * sizeof(char*);
    if (auto preresolved = utils::pun_read<char*>(resolved_method_location); preresolved) {
        return method(preresolved);
    }
    auto resolved = this->resolve_symbolic_method(method_offset);
    utils::pun_write(resolved_method_location, resolved.unwrap());
    return resolved;
}