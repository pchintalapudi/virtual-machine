#include "class.h"

#include "../memory/memutils.h"

using namespace oops::objects;

char *clazz::meta_region() const
{
    return this->real;
}

std::uint64_t clazz::object_malloc_required_size() const
{
    return memory::size64to32(utils::pun_read<std::uint32_t>(this->meta_region()) >> 1);
}

std::uint64_t clazz::static_variables_size() const
{
    return memory::size64to32(utils::pun_read<std::uint32_t>(this->meta_region() + sizeof(std::uint32_t)));
}

bool clazz::requires_finalization() const
{
    return utils::pun_read<std::uint32_t>(this->meta_region()) & 1u;
}

std::uint32_t clazz::virtual_handle_count() const
{
    return utils::pun_read<std::uint32_t>(this->meta_region() + sizeof(std::uint32_t) * 2);
}

std::uint32_t clazz::static_handle_count() const
{
    return utils::pun_read<std::uint32_t>(this->meta_region() + sizeof(std::uint32_t) * 3);
}

std::uint32_t clazz::total_method_count() const
{
    return utils::pun_read<std::uint32_t>(this->meta_region() + sizeof(std::uint32_t) * 4);
}

std::uint32_t clazz::total_virtual_variable_count() const
{
    return utils::pun_read<std::uint32_t>(this->meta_region() + sizeof(std::uint32_t) * 5);
}

std::uint32_t clazz::total_static_variable_count() const
{
    return utils::pun_read<std::uint32_t>(this->meta_region() + sizeof(std::uint32_t) * 6);
}

std::uint32_t clazz::total_class_count() const
{
    return utils::pun_read<std::uint32_t>(this->meta_region() + sizeof(std::uint32_t) * 7);
}

std::uint32_t clazz::self_virtual_method_count() const
{
    return utils::pun_read<std::uint32_t>(this->meta_region() + sizeof(std::uint32_t) * 8);
}

std::uint32_t clazz::self_static_method_count() const
{
    return utils::pun_read<std::uint32_t>(this->meta_region() + sizeof(std::uint32_t) * 9);
}

std::uint32_t clazz::self_virtual_variable_count() const
{
    return utils::pun_read<std::uint32_t>(this->meta_region() + sizeof(std::uint32_t) * 10);
}

std::uint32_t clazz::self_static_variable_count() const
{
    return utils::pun_read<std::uint32_t>(this->meta_region() + sizeof(std::uint32_t) * 11);
}

std::optional<clazz> clazz::superclass() const
{
    char *superclass = utils::pun_read<char *>(this->meta_region() + sizeof(std::uint32_t) * 12);
    return superclass ? std::optional(superclass) : std::optional<clazz>();
}

char *clazz::vtable() const
{
    return this->meta_region() + sizeof(std::uint32_t) * 12 + sizeof(char *);
}

char* clazz::ctable() const {
    return this->vtable() + sizeof(char*) * this->total_method_count();
}

char* clazz::static_memory() const {
    return this->ctable() + sizeof(char*) * this->total_class_count();
}

std::variant<clazz, oops::utils::ostring> clazz::lookup_class_offset(std::uint32_t offset) const {
    char* ptr = this->ctable() + static_cast<std::uint64_t>(offset) * sizeof(char*);
    if (utils::pun_reinterpret<std::uintptr_t>(ptr) & 1) {
        return utils::ostring(ptr + sizeof(std::uint32_t) - 1);
    } else {
        return clazz(ptr);
    }
}

void clazz::dynamic_loaded_class(std::uint32_t offset, objects::clazz cls) {
    utils::pun_write(this->ctable() + static_cast<std::uint64_t>(offset) * sizeof(char*), cls.unwrap());
}