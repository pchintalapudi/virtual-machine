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

char *clazz::resolved_method_start() const
{
    return this->meta_start() + sizeof(std::uint32_t) * 6;
}

char *clazz::resolved_class_start() const
{
    return this->resolved_method_start() + static_cast<std::uintptr_t>(this->method_count()) * sizeof(char *);
}

char *clazz::static_variables_start() const
{
    return this->resolved_class_start() + static_cast<std::uintptr_t>(this->class_count()) * sizeof(char *);
}

char *clazz::method_symbol_table() const {
    return this->static_variables_start() + this->static_variables_size();
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

void clazz::dynamic_loaded_class(std::uint32_t offset, objects::clazz cls)
{
    utils::pun_write(this->resolved_class_start() + static_cast<std::uint64_t>(offset) * sizeof(char *), cls.unwrap());
}

void clazz::dynamic_loaded_method(std::uint32_t offset, objects::method method)
{
    utils::pun_write(this->resolved_method_start() + static_cast<std::uint64_t>(offset) * sizeof(char *), method.unwrap());
}

namespace
{
    class method_symbol_table_iterator
    {
    private:
        char *method_name_ptr;

        template<typename puntee>
        class punt {
            private:
            puntee p;
            public:
            punt(puntee p) : p(p) {}

            puntee* operator->() {
                return &this->p;
            }
        };

    public:
        typedef std::ptrdiff_t difference_type;
        typedef std::pair<oops::utils::ostring, std::uint32_t> value_type;
        typedef value_type *pointer;
        typedef value_type &reference;
        typedef std::random_access_iterator_tag iterator_category;

        method_symbol_table_iterator(char *method_name_ptr) : method_name_ptr(method_name_ptr) {}

        method_symbol_table_iterator &operator++()
        {
            this->method_name_ptr += sizeof(char *);
            return *this;
        }

        method_symbol_table_iterator &operator--()
        {
            this->method_name_ptr -= sizeof(char *);
            return *this;
        }

        value_type operator*() const {
            return {oops::utils::ostring(this->method_name_ptr + sizeof(std::uint32_t)), oops::utils::pun_read<std::uint32_t>(this->method_name_ptr - sizeof(std::uint32_t))};
        }

        punt<value_type> operator->() const {
            return punt(**this);
        }

        value_type operator[](std::ptrdiff_t n) {
            return *(*this + n);
        }

        method_symbol_table_iterator &operator+=(std::ptrdiff_t n)
        {
            this->method_name_ptr += sizeof(char *) * n;
            return *this;
        }

        method_symbol_table_iterator &operator-=(std::ptrdiff_t n)
        {
            this->method_name_ptr -= sizeof(char *) * n;
            return *this;
        }

        method_symbol_table_iterator operator+(std::ptrdiff_t n)
        {
            return method_symbol_table_iterator(this->method_name_ptr + n * sizeof(char *));
        }

        friend method_symbol_table_iterator operator+(std::ptrdiff_t n, method_symbol_table_iterator &start) {
            return start + n;
        }

        method_symbol_table_iterator operator-(std::ptrdiff_t n)
        {
            return method_symbol_table_iterator(this->method_name_ptr - n * sizeof(char *));
        }

        std::ptrdiff_t operator-(const method_symbol_table_iterator &other) {
            return this->method_name_ptr - other.method_name_ptr;
        }

        method_symbol_table_iterator operator++(int)
        {
            return ++*this - 1;
        }

        method_symbol_table_iterator operator--(int)
        {
            return --*this + 1;
        }

        bool operator<(const method_symbol_table_iterator &other) const
        {
            return this->method_name_ptr < other.method_name_ptr;
        }

        bool operator<=(const method_symbol_table_iterator &other) const
        {
            return this->method_name_ptr <= other.method_name_ptr;
        }

        bool operator>(const method_symbol_table_iterator &other) const
        {
            return this->method_name_ptr > other.method_name_ptr;
        }

        bool operator>=(const method_symbol_table_iterator &other) const
        {
            return this->method_name_ptr >= other.method_name_ptr;
        }

        bool operator==(const method_symbol_table_iterator &other) const
        {
            return this->method_name_ptr == other.method_name_ptr;
        }

        bool operator!=(const method_symbol_table_iterator &other) const
        {
            return this->method_name_ptr != other.method_name_ptr;
        }
    };
} // namespace

std::optional<std::uint32_t> clazz::lookup_interface_method(utils::ostring name) const {
    std::pair search(name, ~static_cast<std::uint32_t>(0));
    auto method_start = this->method_symbol_table();
    ::method_symbol_table_iterator begin(method_start), end(begin + this->method_count());
    auto found = std::lower_bound(begin, end, search);
    if (found < end and found->first == name) return found->second;
    return {};
}