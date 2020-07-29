#include "class_manager.h"
#include "../objects/objects.h"
#include "../memory/memutils.h"
#include "../platform_specific/memory.h"

#include <numeric>
#include <set>

using namespace oops::interfaze;

std::optional<std::uint32_t> class_manager::lookup_interface_method(objects::method imethod, objects::base_object src)
{
    typedef std::pair<char *, std::uint32_t> inlined_lookup;
    typedef std::unordered_map<char *, std::uint32_t> mega_lookup;
    auto cls = src.get_clazz();
    if (auto it = this->interface_method_cache.find(imethod.unwrap()); it != this->interface_method_cache.end())
    {
        auto variant = it->second;
        if (std::holds_alternative<inlined_lookup>(variant))
        {
            auto pair = std::get<inlined_lookup>(variant);
            if (cls.unwrap() == pair.first)
            {
                return pair.second;
            }
            else
            {
                auto method_idx = cls.get_real_method_offset(imethod.name());
                if (method_idx)
                    this->interface_method_cache[imethod.unwrap()] = mega_lookup{{pair, {cls.unwrap(), *method_idx}}};
                return method_idx;
            }
        }
        else
        {
            auto lookup = std::get<mega_lookup>(variant);
            if (auto it = lookup.find(cls.unwrap()); it != lookup.end())
            {
                return it->second;
            }
            else
            {
                auto method_idx = cls.get_real_method_offset(imethod.name());
                if (method_idx)
                    lookup[cls.unwrap()] = *method_idx;
                return method_idx;
            }
        }
    }
    else
    {
        auto method_idx = cls.get_real_method_offset(imethod.name());
        if (method_idx)
            this->interface_method_cache[imethod.unwrap()] = inlined_lookup{cls.unwrap(), *method_idx};
        return method_idx;
    }
}

bool class_manager:: instanceof (objects::clazz src, objects::clazz test) const
{
    if (src == test)
        return true;
    if (auto it = this->loaded_classes.find(src.unwrap()); it != this->loaded_classes.end())
    {
        auto impl_it = this->implemented.begin() + it->second.second;
        return std::binary_search(impl_it + 1, impl_it + *impl_it + 1, utils::pun_reinterpret<std::uintptr_t>(test.unwrap()));
    }
    return false;
}

namespace
{
    class class_file
    {
    private:
        char *memory_mapped_file;

        char *class_references_start() const
        {
            return this->memory_mapped_file + oops::utils::pun_read<std::uint64_t>(this->memory_mapped_file);
        }
        char *method_references_start() const
        {
            return this->memory_mapped_file + oops::utils::pun_read<std::uint64_t>(this->memory_mapped_file + sizeof(std::uint64_t));
        }
        char *static_references_start() const
        {
            return this->memory_mapped_file + oops::utils::pun_read<std::uint64_t>(this->memory_mapped_file + sizeof(std::uint64_t) * 2);
        }
        char *virtual_references_start() const
        {
            return this->memory_mapped_file + oops::utils::pun_read<std::uint64_t>(this->memory_mapped_file + sizeof(std::uint64_t) * 3);
        }
        char *bytecodes_start() const
        {
            return this->memory_mapped_file + oops::utils::pun_read<std::uint64_t>(this->memory_mapped_file + sizeof(std::uint64_t) * 4);
        }
        char *string_pool_start() const
        {
            return this->memory_mapped_file + oops::utils::pun_read<std::uint64_t>(this->memory_mapped_file + sizeof(std::uint64_t) * 5);
        }

        template <typename drill>
        struct punt
        {
        private:
            drill d;

        public:
            punt(drill d) : d(d) {}

            drill *operator->() const
            {
                return &this->d;
            }
        };
        template <typename fixed_width_type>
        class fixed_width_iterator
        {
        private:
            char *real;

        public:
            typedef std::random_access_iterator_tag iterator_category;
            typedef std::ptrdiff_t difference_type;
            typedef fixed_width_type value_type;
            typedef fixed_width_type *pointer;
            typedef fixed_width_type &reference;

            explicit fixed_width_iterator(char *real) : real(real) {}

            bool operator==(fixed_width_iterator other)
            {
                return other.real == this->real;
            }

            bool operator!=(fixed_width_iterator other)
            {
                return other.real != this->real;
            }

            bool operator<(fixed_width_iterator other)
            {
                return this->real < other.real;
            }

            bool operator<=(fixed_width_iterator other)
            {
                return this->real <= other.real;
            }

            bool operator>(fixed_width_iterator other)
            {
                return this->real > other.real;
            }

            bool operator>=(fixed_width_iterator other)
            {
                return this->real >= other.real;
            }

            fixed_width_type operator*()
            {
                return fixed_width_type(this->real);
            }

            punt<fixed_width_type> operator->() const
            {
                return **this;
            }

            fixed_width_iterator &operator++()
            {
                this->real += sizeof(fixed_width_type);
                return *this;
            }

            fixed_width_iterator &operator--()
            {
                this->real -= sizeof(fixed_width_type);
                return *this;
            }

            fixed_width_iterator operator+(difference_type n) const
            {
                return fixed_width_iterator(this->real + n * sizeof(fixed_width_type));
            }

            friend fixed_width_iterator operator+(difference_type n, fixed_width_iterator a)
            {
                return fixed_width_iterator(a.real + n);
            }

            fixed_width_iterator operator-(difference_type n) const
            {
                return fixed_width_iterator(this->real - n * sizeof(fixed_width_type));
            }

            difference_type operator-(fixed_width_iterator b)
            {
                return (this->real - b.real) / sizeof(fixed_width_type);
            }

            fixed_width_iterator operator++(int)
            {
                return ++*this - 1;
            }

            fixed_width_iterator operator--(int)
            {
                return --*this + 1;
            }

            fixed_width_iterator &operator+=(difference_type n)
            {
                this->real += n * sizeof(fixed_width_type);
                return *this;
            }

            fixed_width_iterator &operator-=(difference_type n)
            {
                this->real -= n * sizeof(fixed_width_type);
                return *this;
            }

            fixed_width_type operator[](difference_type n)
            {
                return fixed_width_type(this->real + n * sizeof(fixed_width_type));
            }
        };

        class class_reference
        {
        private:
            std::uint64_t string_offset;

        public:
            class_reference(char *reference) : string_offset(oops::utils::pun_read<std::uint64_t>(reference)) {}

            std::uint64_t operator*() const
            {
                return this->string_offset;
            }
        };

        class symbol_reference
        {
        private:
            std::uint32_t class_reference;
            std::uint32_t type_reference;
            std::uint64_t string_offset;

        public:
            symbol_reference(char *reference) : class_reference(oops::utils::pun_read<std::uint32_t>(reference)), type_reference(oops::utils::pun_read<std::uint32_t>(reference + sizeof(std::uint32_t))), string_offset(oops::utils::pun_read<std::uint64_t>(reference + sizeof(std::uint32_t) * 2)) {}

            std::uint64_t operator*() const
            {
                return this->string_offset;
            }

            std::uint32_t type_index() const
            {
                return this->type_reference;
            }

            std::uint32_t class_index() const
            {
                return this->class_reference;
            }
        };

        template <typename fixed_width_type>
        class fixed_width_range
        {
        private:
            char *start, *finish;

        public:
            fixed_width_range(char *begin, char *end) : start(begin), finish(end) {}

            typedef fixed_width_iterator<fixed_width_type> iterator_t;

            iterator_t begin() const
            {
                return iterator_t(this->start);
            }

            iterator_t end() const
            {
                return iterator_t(this->finish);
            }

            typename iterator_t::difference_type size() const
            {
                return this->end() - this->begin();
            }

            fixed_width_range slice(typename iterator_t::difference_type start, typename iterator_t::difference_type finish = -1) const
            {
                return fixed_width_range(this->start + start * sizeof(fixed_width_type), finish == -1 ? this->finish : this->start + finish * sizeof(fixed_width_type));
            }
        };

        class bytecode_iterator
        {
        private:
            char *real;

        public:
            bytecode_iterator(char *real) : real(real) {}

            bool operator==(const bytecode_iterator &other) const
            {
                return this->real == other.real;
            }
            bool operator!=(const bytecode_iterator &other) const
            {
                return this->real != other.real;
            }
            bool operator<(const bytecode_iterator &other) const
            {
                return this->real < other.real;
            }
            bool operator<=(const bytecode_iterator &other) const
            {
                return this->real <= other.real;
            }
            bool operator>(const bytecode_iterator &other) const
            {
                return this->real > other.real;
            }
            bool operator>=(const bytecode_iterator &other) const
            {
                return this->real >= other.real;
            }

            std::pair<char *, std::uint64_t> operator*() const
            {
                return {this->real, oops::utils::pun_read<std::uint64_t>(this->real)};
            }

            bytecode_iterator &operator++()
            {
                this->real += (**this).second;
                return *this;
            }
        };

    public:
        class_file(char *memory_mapped_file) : memory_mapped_file(memory_mapped_file) {}

        typedef fixed_width_range<class_reference> class_range;
        typedef fixed_width_range<symbol_reference> method_range;
        typedef fixed_width_range<symbol_reference> static_range;
        typedef fixed_width_range<symbol_reference> virtual_range;

        std::uint64_t implement_count() const
        {
            return oops::utils::pun_read<std::uint32_t>(this->class_references_start() + sizeof(std::uint32_t));
        }

        auto classes() const
        {
            char *start = this->class_references_start();
            std::uint64_t class_count = oops::utils::pun_read<std::uint32_t>(start);
            return class_range(start + sizeof(std::uint32_t) * 2, start + sizeof(std::uint32_t) * 2 + class_count * sizeof(class_reference));
        }

        auto methods() const
        {
            char *start = this->method_references_start();
            std::uint64_t method_count = oops::utils::pun_read<std::uint32_t>(start);
            return method_range(start + sizeof(std::uint32_t) * 2, start + sizeof(std::uint32_t) * 2 + method_count * (sizeof(std::uint32_t) * 2 + sizeof(std::uint64_t)));
        }

        auto statics() const
        {
            char *start = this->static_references_start();
            std::uint64_t static_count = oops::utils::pun_read<std::uint32_t>(start);
            return static_range(start + sizeof(std::uint32_t) * 2, start + sizeof(std::uint32_t) * 2 + static_count * (sizeof(std::uint32_t) * 2 + sizeof(std::uint64_t)));
        }

        auto virtuals() const
        {
            char *start = this->virtual_references_start();
            std::uint64_t virtual_count = oops::utils::pun_read<std::uint32_t>(start);
            return virtual_range(start + sizeof(std::uint32_t) * 2, start + sizeof(std::uint32_t) * 2 + virtual_count * (sizeof(std::uint32_t) * 2 + sizeof(std::uint64_t)));
        }

        std::uint64_t bytecode_size() const
        {
            return oops::utils::pun_read<std::uint64_t>(this->bytecodes_start());
        }

        bytecode_iterator bytecode_start() const
        {
            return bytecode_iterator(this->bytecodes_start() + sizeof(std::uint64_t));
        }

        bytecode_iterator bytecode_end() const
        {
            return bytecode_iterator(this->bytecodes_start() + sizeof(std::uint64_t) + this->bytecode_size());
        }

        std::uint64_t string_pool_size() const
        {
            return oops::utils::pun_read<std::uint64_t>(this->string_pool_start());
        }

        std::uint64_t commit_size() const
        {
            return this->string_pool_size() + this->bytecode_size() + this->classes().size() * sizeof(char *) + (this->methods().size() + this->statics().size() + this->virtuals().size()) * (sizeof(char *) + sizeof(std::uint32_t)) + 8 * sizeof(std::uint32_t);
        }

        oops::utils::ostring get_string(std::uint64_t offset) const
        {
            return this->memory_mapped_file + offset;
        }
    };

    std::optional<class_file> mmap_file(oops::utils::ostring name)
    {
        //TODO
        return {};
    }

    char *load_ostring(char *start, oops::utils::ostring string, unsigned int alignment = 4)
    {
        auto length = string.length();
        oops::utils::pun_write(start, length);
        std::memcpy(start + sizeof(std::uint32_t), string.unwrap(), length);
        start += length + sizeof(std::uint32_t);
        auto ret_val = oops::utils::pun_reinterpret<char *>(oops::memory::align_up<std::uintptr_t>(oops::utils::pun_reinterpret<std::uintptr_t>(start), alignment));
        std::memset(start, 0, ret_val - start);
        return ret_val;
    }
} // namespace

oops::objects::clazz class_manager::load_class(utils::ostring name)
{
    constexpr std::uint32_t self_index = static_cast<unsigned>(objects::field::type::DOUBLE) + 1;
    auto loaded = this->loaded_classes.find(name);
    if (loaded != this->loaded_classes.end())
    {
        return objects::clazz(loaded->second.first);
    }
    if (auto maybe_cls = ::mmap_file(name))
    {
        auto cls = *maybe_cls;

        auto classes = cls.classes();
        auto methods = cls.methods();
        auto statics = cls.statics();
        auto virtuals = cls.virtuals();

        //Preload inherited classes
        std::vector<objects::clazz> inherited;
        inherited.reserve(cls.implement_count());
        auto inherit_range = classes.slice(self_index + 1, self_index + 1 + cls.implement_count());
        std::transform(inherit_range.begin(), inherit_range.end(), std::back_inserter(inherited), [this, cls](auto cls_ref) { return this->load_class(cls.get_string(*cls_ref)); });

        //Handle instanceof invariants
        std::set<char *> implemented;
        for (auto impl : inherited)
        {
            implemented.insert(impl.unwrap());
            auto impl_index = this->loaded_classes[impl.unwrap()].second;
            std::transform(this->implemented.begin() + impl_index + 1, this->implemented.begin() + impl_index + 1 + this->implemented[impl_index], std::inserter(implemented, implemented.begin()), [](std::uintptr_t ptr) { return utils::pun_reinterpret<char *>(ptr); });
        }
        this->implemented.push_back(utils::pun_reinterpret<std::uintptr_t>(this->head));
        std::transform(implemented.begin(), implemented.end(), std::back_inserter(this->implemented), [](char* ptr) { return utils::pun_reinterpret<std::uintptr_t>(ptr); });
    }
    return objects::clazz(nullptr);
}