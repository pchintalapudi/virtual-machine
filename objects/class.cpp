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
    return static_cast<std::uint64_t>(utils::pun_read<std::uint32_t>(this->meta_region() + sizeof(std::uint32_t))) << __builtin_ctz(sizeof(std::uint64_t));
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

std::uint32_t clazz::virtual_method_count() const
{
    return utils::pun_read<std::uint32_t>(this->meta_region() + sizeof(std::uint32_t) * 8);
}

std::uint32_t clazz::self_method_count() const
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

char *clazz::ctable() const
{
    return this->vtable() + sizeof(char *) * this->virtual_method_count();
}

char *clazz::static_memory() const
{
    return this->ctable() + sizeof(char *) * this->total_class_count();
}

char *clazz::method_pointers() const
{
    return this->static_memory() + this->static_variables_size();
}

char *clazz::virtual_pointers() const
{
    return this->method_pointers() + static_cast<std::uint64_t>(this->total_method_count()) * sizeof(char *);
}

char *clazz::static_pointers() const
{
    return this->virtual_pointers() + static_cast<std::uint64_t>(this->total_virtual_variable_count()) * sizeof(char *);
}

char *clazz::mtable() const
{
    return this->static_pointers() + static_cast<std::uint64_t>(this->total_static_variable_count()) * sizeof(char *);
}

char* clazz::itable() const {
    return this->mtable() + static_cast<std::uint64_t>(this->self_method_count()) * sizeof(char*);
}

char* clazz::stable() const {
    return this->itable() + static_cast<std::uint64_t>(this->self_virtual_variable_count()) * sizeof(char*);
}

std::variant<clazz, oops::utils::ostring> clazz::lookup_class_offset(std::uint32_t offset) const
{
    char *ptr = utils::pun_read<char *>(this->ctable() + static_cast<std::uint64_t>(offset) * sizeof(char *));
    if (utils::pun_reinterpret<std::uintptr_t>(ptr) & 1)
    {
        return utils::ostring(ptr + sizeof(std::uint32_t) - 1);
    }
    else
    {
        return clazz(ptr);
    }
}

void clazz::dynamic_loaded_class(std::uint32_t offset, objects::clazz cls)
{
    utils::pun_write(this->ctable() + static_cast<std::uint64_t>(offset) * sizeof(char *), cls.unwrap());
}

method clazz::direct_method_lookup(std::uint32_t external_offset) const
{
    return utils::pun_read<char *>(this->vtable() + static_cast<std::uint64_t>(external_offset) * sizeof(char *));
}

std::variant<std::pair<oops::utils::ostring, std::uint32_t>, std::uint32_t> clazz::get_external_method_offset(std::uint32_t internal_offset) const
{
    std::uintptr_t optimistic = utils::pun_read<std::uintptr_t>(this->method_pointers() + static_cast<std::uint64_t>(internal_offset) * sizeof(char *));
    if (optimistic & 1)
    {
        char *string_ptr = utils::pun_reinterpret<char *>(optimistic);
        return std::pair{utils::ostring(string_ptr + sizeof(std::uint32_t) * 2 - 1), utils::pun_read<std::uint32_t>(string_ptr - 1)};
    }
    else
    {
        return static_cast<std::uint32_t>(optimistic >> ((sizeof(std::uintptr_t) - sizeof(std::uint32_t)) * CHAR_BIT));
    }
}

std::variant<std::pair<oops::utils::ostring, std::uint32_t>, std::uint32_t> clazz::get_external_virtual_field_offset(std::uint32_t internal_offset) const
{
    std::uintptr_t optimistic = utils::pun_read<std::uintptr_t>(this->virtual_pointers() + static_cast<std::uint64_t>(internal_offset) * sizeof(char *));
    if (optimistic & 1)
    {
        char *string_ptr = utils::pun_reinterpret<char *>(optimistic);
        return std::pair{utils::ostring(string_ptr + sizeof(std::uint32_t) * 2 - 1), utils::pun_read<std::uint32_t>(string_ptr - 1)};
    }
    else
    {
        return static_cast<std::uint32_t>(optimistic >> ((sizeof(std::uintptr_t) - sizeof(std::uint32_t)) * CHAR_BIT));
    }
}

std::variant<std::pair<oops::utils::ostring, std::uint32_t>, char *> clazz::get_external_static_field(std::uint32_t internal_offset) const
{
    std::uintptr_t optimistic = utils::pun_read<std::uintptr_t>(this->static_pointers() + static_cast<std::uint64_t>(internal_offset) * sizeof(char *));
    if (optimistic & 1)
    {
        char *string_ptr = utils::pun_reinterpret<char *>(optimistic);
        return std::pair{utils::ostring(string_ptr + sizeof(std::uint32_t) * 2 - 1), utils::pun_read<std::uint32_t>(string_ptr - 1)};
    }
    else
    {
        return utils::pun_reinterpret<char *>(optimistic);
    }
}

void clazz::set_external_method_offset(std::uint32_t internal_offset, std::uint32_t external_offset)
{
    utils::pun_write(this->method_pointers() + static_cast<std::uint64_t>(internal_offset) * sizeof(char *), static_cast<std::uintptr_t>(external_offset) << ((sizeof(std::uintptr_t) - sizeof(std::uint32_t)) * CHAR_BIT));
}

void clazz::set_external_virtual_field_offset(std::uint32_t internal_offset, std::uint32_t external_offset)
{
    utils::pun_write(this->virtual_pointers() + static_cast<std::uint64_t>(internal_offset) * sizeof(char *), static_cast<std::uintptr_t>(external_offset) << ((sizeof(std::uintptr_t) - sizeof(std::uint32_t)) * CHAR_BIT));
}

void clazz::set_external_static_field(std::uint32_t internal_offset, char *external_method)
{
    utils::pun_write(this->static_pointers() + static_cast<std::uint64_t>(internal_offset) * sizeof(char *), external_method);
}

namespace
{
    class symbol_iterator
    {
    private:
        char *real;

        template <typename punt>
        class drilldown
        {
        private:
            punt p;

        public:
            drilldown(punt p) : p(p) {}

            punt *operator->()
            {
                return &this->p;
            }
        };

    public:
        typedef std::ptrdiff_t difference_type;
        typedef std::random_access_iterator_tag iterator_category;
        struct value_type
        {
            oops::utils::ostring string;
            std::uint32_t real_offset;

            bool operator==(value_type other) const
            {
                return this->string == other.string;
            }
            bool operator!=(value_type other) const
            {
                return this->string != other.string;
            }
            bool operator<(value_type other) const
            {
                return this->string < other.string;
            }

            bool operator<=(value_type other) const
            {
                return this->string <= other.string;
            }

            bool operator>(value_type other) const
            {
                return this->string > other.string;
            }

            bool operator>=(value_type other) const
            {
                return this->string >= other.string;
            }
        };
        typedef value_type *pointer;
        typedef value_type &reference;

        symbol_iterator(char *real) : real(real) {}

        bool operator==(symbol_iterator other) const
        {
            return this->real == other.real;
        }

        bool operator!=(symbol_iterator other) const
        {
            return this->real != other.real;
        }

        bool operator<(symbol_iterator other) const
        {
            return this->real < other.real;
        }

        bool operator<=(symbol_iterator other) const
        {
            return this->real <= other.real;
        }

        bool operator>(symbol_iterator other) const
        {
            return this->real > other.real;
        }

        bool operator>=(symbol_iterator other) const
        {
            return this->real >= other.real;
        }

        value_type operator*() const
        {
            char *str = oops::utils::pun_read<char *>(this->real);
            return {oops::utils::ostring(str + sizeof(std::uint32_t) * 2), oops::utils::pun_read<std::uint32_t>(str)};
        }

        drilldown<value_type> operator->() const
        {
            return **this;
        }

        symbol_iterator &operator+=(difference_type n)
        {
            this->real += n * sizeof(char *);
            return *this;
        }

        symbol_iterator &operator-=(difference_type n)
        {
            this->real -= n * sizeof(char *);
            return *this;
        }

        symbol_iterator &operator++()
        {
            return *this += 1;
        }

        symbol_iterator &operator--()
        {
            return *this -= 1;
        }

        symbol_iterator operator+(difference_type n) const
        {
            return symbol_iterator(this->real + n * sizeof(char *));
        }

        friend symbol_iterator operator+(difference_type n, symbol_iterator a)
        {
            return symbol_iterator(n * sizeof(char *) + a.real);
        }

        symbol_iterator operator-(difference_type n) const
        {
            return symbol_iterator(this->real - n * sizeof(char *));
        }

        difference_type operator-(symbol_iterator other) const
        {
            return (this->real - other.real) / sizeof(char *);
        }

        symbol_iterator operator++(int)
        {
            return ++*this - 1;
        }

        symbol_iterator operator--(int)
        {
            return --*this + 1;
        }

        value_type operator[](difference_type n) const
        {
            return *(*this + n);
        }
    };
} // namespace

std::optional<std::uint32_t> clazz::get_real_method_offset(utils::ostring name) const
{
    ::symbol_iterator begin(this->mtable()), end(begin + this->self_method_count()), low = std::lower_bound(begin, end, symbol_iterator::value_type{name, 0});
    if (low->string == name) {
        return low->real_offset;
    } else if (auto super = this->superclass()) {
        return super->get_real_method_offset(name);
    } else return {};
}

std::optional<std::uint32_t> clazz::get_real_virtual_field_offset(utils::ostring name) const {
    ::symbol_iterator begin(this->itable()), end(begin + this->self_virtual_variable_count()), low = std::lower_bound(begin, end, symbol_iterator::value_type{name, 0});
    if (low->string == name) {
        return low->real_offset;
    } else if (auto super = this->superclass()) {
        return super->get_real_virtual_field_offset(name);
    } else return {};
}

std::optional<char*> clazz::get_real_static_field(utils::ostring name) const {
    ::symbol_iterator begin(this->stable()), end(begin + this->self_static_variable_count()), low = std::lower_bound(begin, end, symbol_iterator::value_type{name, 0});
    return low->string == name ? std::optional{this->static_memory() + low->real_offset} : std::optional<char*>{};
}