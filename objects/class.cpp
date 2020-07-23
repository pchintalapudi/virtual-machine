#include "class.h"

#include "../memory/memutils.h"

using namespace oops::objects;

std::uint64_t clazz::object_malloc_required_size() const
{
    return memory::size32to64(utils::pun_read<std::uint32_t>(this->meta_start()) >> 1);
}

bool clazz::requires_finalization() const
{
    return utils::pun_read<std::uint32_t>(this->meta_start()) & 1;
}

std::uint64_t clazz::static_variables_size() const
{
    return memory::size32to64(utils::pun_read<std::uint32_t>(this->meta_start() + sizeof(std::uint32_t)));
}

std::uint32_t clazz::static_handle_count() const
{
    return utils::pun_read<std::uint32_t>(this->meta_start() + sizeof(std::uint32_t) * 2);
}

std::uint32_t clazz::virtual_handle_count() const
{
    return utils::pun_read<std::uint32_t>(this->meta_start() + sizeof(std::uint32_t) * 3);
}

std::uint32_t clazz::method_count() const
{
    return utils::pun_read<std::uint32_t>(this->meta_start() + sizeof(std::uint32_t) * 4);
}
std::uint32_t clazz::class_count() const
{
    return utils::pun_read<std::uint32_t>(this->meta_start() + sizeof(std::uint32_t) * 5);
}
std::uint32_t clazz::static_variable_count() const
{
    return utils::pun_read<std::uint32_t>(this->meta_start() + sizeof(std::uint32_t) * 6);
}
std::uint32_t clazz::virtual_variable_count() const
{
    return utils::pun_read<std::uint32_t>(this->meta_start() + sizeof(std::uint32_t) * 7);
}
std::uint32_t clazz::symbol_count() const
{
    return this->method_count() + this->static_variable_count() + this->virtual_variable_count();
}

char *clazz::resolved_method_start() const
{
    return this->meta_start() + sizeof(std::uint32_t) * 9;
}

char *clazz::resolved_class_start() const
{
    return this->resolved_method_start() + static_cast<std::uintptr_t>(this->method_count()) * sizeof(char *);
}

char *clazz::static_memory_start() const
{
    return this->resolved_class_start() + static_cast<std::uintptr_t>(this->class_count()) * sizeof(char *);
}

char *clazz::resolved_static_variable_start() const
{
    return this->static_memory_start() + this->static_variables_size();
}

char *clazz::resolved_virtual_variable_start() const
{
    return this->resolved_static_variable_start() + sizeof(char *) * this->static_variable_count();
}

char *clazz::symbol_table() const
{
    return this->resolved_virtual_variable_start() + sizeof(char *) * this->virtual_variable_count();
}

std::optional<char *> clazz::lookup_static_interface_field(utils::ostring name) const
{
    std::optional<std::uint64_t> maybe_offset = this->lookup_symbol(name);
    if (maybe_offset)
    {
        return this->static_memory_start() + *maybe_offset;
    }
    return {};
}

std::variant<clazz, oops::utils::ostring> clazz::lookup_class_offset(std::uint32_t offset) const
{
    char *value = utils::pun_read<char *>(this->resolved_class_start() + static_cast<std::uint64_t>(offset) * sizeof(char *));
    if (utils::pun_reinterpret<std::uintptr_t>(value) & 1)
    {
        return utils::ostring(value - 1);
    }
    else
    {
        return clazz(value);
    }
}

std::variant<method, std::pair<std::uint32_t, oops::utils::ostring>> clazz::lookup_method_offset(std::uint32_t offset) const
{
    char *value = utils::pun_read<char *>(this->resolved_method_start() + static_cast<std::uint64_t>(offset) * sizeof(char *));
    if (utils::pun_reinterpret<std::uintptr_t>(value) & 1)
    {
        return std::make_pair(utils::pun_read<std::uint32_t>(value - 1), utils::ostring(value + sizeof(std::uint32_t) - 1 + sizeof(std::uint32_t)));
    }
    else
    {
        return method(value);
    }
}

std::variant<std::pair<std::uint32_t, oops::utils::ostring>, char *> clazz::lookup_static_field_offset(std::uint32_t offset) const
{
    char *optimistic = utils::pun_read<char *>(this->resolved_static_variable_start() + static_cast<std::uint64_t>(offset) * sizeof(char *));
    if (utils::pun_reinterpret<std::uintptr_t>(optimistic) & 1)
    {
        return std::pair{utils::pun_read<std::uint32_t>(optimistic - 1), utils::ostring(optimistic + sizeof(std::uint32_t) * 2 - 1)};
    }
    else
    {
        return optimistic;
    }
}

std::variant<std::uint32_t, std::pair<std::uint32_t, oops::utils::ostring>> clazz::lookup_virtual_field_offset(std::uint32_t offset) const
{
    std::uint64_t index = offset;
    index *= sizeof(char *);
    std::uint64_t pessimistic = utils::pun_read<std::uint64_t>(this->resolved_virtual_variable_start() + index);
    if (pessimistic & 1)
    {
        return {static_cast<std::uint32_t>(pessimistic >> (sizeof(std::uint32_t) * CHAR_BIT))};
    }
    else
    {
        char *fat_string = utils::pun_reinterpret<char *>(pessimistic);
        return std::make_pair(utils::pun_read<std::uint32_t>(fat_string), utils::ostring(fat_string + sizeof(std::uint32_t) * 2));
    }
}

void clazz::dynamic_loaded_class(std::uint32_t offset, objects::clazz cls)
{
    utils::pun_write(this->resolved_class_start() + static_cast<std::uint64_t>(offset) * sizeof(char *), cls.unwrap());
}

void clazz::dynamic_loaded_method(std::uint32_t offset, objects::method method)
{
    utils::pun_write(this->resolved_method_start() + static_cast<std::uint64_t>(offset) * sizeof(char *), method.unwrap());
}

void clazz::dynamic_loaded_static_field(std::uint32_t offset31, char* field)
{
    std::uint64_t index = offset31;
    index *= sizeof(char *);
    utils::pun_write(this->resolved_static_variable_start() + index, field);
}

void clazz::dynamic_loaded_virtual_field(std::uint32_t offset, std::uint32_t field24)
{
    std::uint64_t index = offset;
    index *= sizeof(char *);
    std::uint64_t value = field24;
    value <<= (sizeof(std::uint32_t) * CHAR_BIT);
    value |= 1;
    utils::pun_write(this->resolved_virtual_variable_start() + index, value);
}

namespace
{
    class interface_symbol_iterator
    {
    private:
        char *method_name_ptr;

        template <typename puntee>
        class punt
        {
        private:
            puntee p;

        public:
            punt(puntee p) : p(p) {}

            puntee *operator->()
            {
                return &this->p;
            }
        };

    public:
        typedef std::ptrdiff_t difference_type;
        typedef std::pair<oops::utils::ostring, std::uint32_t> value_type;
        typedef value_type *pointer;
        typedef value_type &reference;
        typedef std::random_access_iterator_tag iterator_category;

        interface_symbol_iterator(char *method_name_ptr) : method_name_ptr(method_name_ptr) {}

        interface_symbol_iterator &operator++()
        {
            this->method_name_ptr += sizeof(char *);
            return *this;
        }

        interface_symbol_iterator &operator--()
        {
            this->method_name_ptr -= sizeof(char *);
            return *this;
        }

        value_type operator*() const
        {
            return {oops::utils::ostring(this->method_name_ptr + sizeof(std::uint32_t)), oops::utils::pun_read<std::uint32_t>(this->method_name_ptr - sizeof(std::uint32_t))};
        }

        punt<value_type> operator->() const
        {
            return punt(**this);
        }

        value_type operator[](std::ptrdiff_t n)
        {
            return *(*this + n);
        }

        interface_symbol_iterator &operator+=(std::ptrdiff_t n)
        {
            this->method_name_ptr += sizeof(char *) * n;
            return *this;
        }

        interface_symbol_iterator &operator-=(std::ptrdiff_t n)
        {
            this->method_name_ptr -= sizeof(char *) * n;
            return *this;
        }

        interface_symbol_iterator operator+(std::ptrdiff_t n)
        {
            return interface_symbol_iterator(this->method_name_ptr + n * sizeof(char *));
        }

        friend interface_symbol_iterator operator+(std::ptrdiff_t n, interface_symbol_iterator &start)
        {
            return start + n;
        }

        interface_symbol_iterator operator-(std::ptrdiff_t n)
        {
            return interface_symbol_iterator(this->method_name_ptr - n * sizeof(char *));
        }

        std::ptrdiff_t operator-(const interface_symbol_iterator &other)
        {
            return this->method_name_ptr - other.method_name_ptr;
        }

        interface_symbol_iterator operator++(int)
        {
            return ++*this - 1;
        }

        interface_symbol_iterator operator--(int)
        {
            return --*this + 1;
        }

        bool operator<(const interface_symbol_iterator &other) const
        {
            return this->method_name_ptr < other.method_name_ptr;
        }

        bool operator<=(const interface_symbol_iterator &other) const
        {
            return this->method_name_ptr <= other.method_name_ptr;
        }

        bool operator>(const interface_symbol_iterator &other) const
        {
            return this->method_name_ptr > other.method_name_ptr;
        }

        bool operator>=(const interface_symbol_iterator &other) const
        {
            return this->method_name_ptr >= other.method_name_ptr;
        }

        bool operator==(const interface_symbol_iterator &other) const
        {
            return this->method_name_ptr == other.method_name_ptr;
        }

        bool operator!=(const interface_symbol_iterator &other) const
        {
            return this->method_name_ptr != other.method_name_ptr;
        }
    };
} // namespace

std::optional<std::uint32_t> clazz::lookup_symbol(utils::ostring name) const
{
    std::pair search(name, ~static_cast<std::uint32_t>(0));
    auto method_start = this->symbol_table();
    ::interface_symbol_iterator begin(method_start), end(begin + this->symbol_count());
    auto found = std::lower_bound(begin, end, search);
    if (found < end and found->first == name)
        return found->second;
    return {};
}