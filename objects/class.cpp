#include "class.h"

#include "../memory/memutils.h"

using namespace oops::objects;

std::uint64_t clazz::object_malloc_required_size() const
{
    return memory::size32to64(utils::pun_read<std::uint32_t>(this->meta_start()));
}

std::uint64_t clazz::static_variables_size() const
{
    return memory::size32to64(utils::pun_read<std::uint32_t>(this->meta_start() + sizeof(std::uint32_t)));
}

std::uint32_t clazz::static_handle_count() const {
    return utils::pun_read<std::uint32_t>(this->meta_start() + sizeof(std::uint32_t) * 2);
}

std::uint32_t clazz::virtual_handle_count() const {
    return utils::pun_read<std::uint32_t>(this->meta_start() + sizeof(std::uint32_t) * 3);
}

std::uint32_t clazz::method_count() const {
    return utils::pun_read<std::uint32_t>(this->meta_start() + sizeof(std::uint32_t) * 4);
}
std::uint32_t clazz::class_count() const {
    return utils::pun_read<std::uint32_t>(this->meta_start() + sizeof(std::uint32_t) * 5);
}

char *clazz::resolved_method_start() const
{
    return this->real + sizeof(std::uint32_t) * 6;
}

char *clazz::resolved_class_start() const
{
    return this->resolved_method_start() + static_cast<std::uintptr_t>(this->method_count()) * sizeof(char*);
}

char *clazz::static_variables_start() const
{
    return this->resolved_class_start() + static_cast<std::uintptr_t>(this->class_count()) * sizeof(char *);
}

clazz clazz::lookup_class(std::uint32_t class_offset) const
{
    auto resolved_class_location = this->resolved_class_start() + static_cast<std::uint64_t>(class_offset) * sizeof(char *);
    auto class_ptr = utils::pun_read<char *>(resolved_class_location);
    if (auto bitptr = utils::pun_reinterpret<std::uintptr_t>(class_ptr); bitptr & 1)
    {
        class_ptr = this->resolve_symbolic_class(utils::pun_reinterpret<char *>(bitptr >> 1 << 1)).unwrap();
        utils::pun_write(resolved_class_location, class_ptr);
    }
    return clazz(class_ptr);
}

method clazz::lookup_method(std::uint32_t method_offset) const
{
    auto resolved_method_location = this->resolved_method_start() + static_cast<std::uint64_t>(method_offset) * sizeof(char *);
    auto method_ptr = utils::pun_read<char *>(resolved_method_location);
    if (auto bitptr = utils::pun_reinterpret<std::uintptr_t>(method_ptr); bitptr & 1)
    {
        method_ptr = this->resolve_symbolic_method(utils::pun_reinterpret<char *>(bitptr >> 1 << 1)).unwrap();
        utils::pun_write(resolved_method_location, method_ptr);
    }
    return method(method_ptr);
}