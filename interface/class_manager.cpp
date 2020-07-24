#include "class_manager.h"
#include "../objects/objects.h"
#include "../platform_specific/memory.h"

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
                auto method_idx = cls.lookup_interface_method(imethod.name());
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
                auto method_idx = cls.lookup_interface_method(imethod.name());
                if (method_idx)
                    lookup[cls.unwrap()] = *method_idx;
                return method_idx;
            }
        }
    }
    else
    {
        auto method_idx = cls.lookup_interface_method(imethod.name());
        if (method_idx)
            this->interface_method_cache[imethod.unwrap()] = inlined_lookup{cls.unwrap(), *method_idx};
        return method_idx;
    }
}

bool class_manager:: instanceof (objects::clazz src, objects::clazz test) const
{
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
            typedef std::random_access_iterator_tag iterator_type;
            typedef std::ptrdiff_t difference_type;
            typedef fixed_width_type value_type;
            typedef fixed_width_type *pointer_type;
            typedef fixed_width_type &reference_type;

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

        class method_reference
        {
        private:
            std::uint32_t class_reference;
            std::uint32_t zeros;
            std::uint64_t string_offset;

        public:
            method_reference(char *reference) : class_reference(oops::utils::pun_read<std::uint32_t>(reference)), zeros(oops::utils::pun_read<std::uint32_t>(reference + sizeof(std::uint32_t))), string_offset(oops::utils::pun_read<std::uint64_t>(reference + sizeof(std::uint32_t) * 2)) {}

            std::uint64_t operator*() const
            {
                return this->string_offset;
            }

            std::uint32_t class_index() const
            {
                return this->class_reference;
            }
        };

        class static_reference
        {
        private:
            std::uint32_t class_reference;
            std::uint32_t type_reference;
            std::uint64_t string_offset;

        public:
            static_reference(char *reference) : class_reference(oops::utils::pun_read<std::uint32_t>(reference)), type_reference(oops::utils::pun_read<std::uint32_t>(reference + sizeof(std::uint32_t))), string_offset(oops::utils::pun_read<std::uint64_t>(reference + sizeof(std::uint32_t) * 2)) {}

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

        class virtual_reference
        {
        private:
            std::uint32_t class_reference;
            std::uint32_t type_reference;
            std::uint64_t string_offset;

        public:
            virtual_reference(char *reference) : class_reference(oops::utils::pun_read<std::uint32_t>(reference)), type_reference(oops::utils::pun_read<std::uint32_t>(reference + sizeof(std::uint32_t))), string_offset(oops::utils::pun_read<std::uint64_t>(reference + sizeof(std::uint32_t) * 2)) {}

            std::uint64_t operator*() const
            {
                return this->string_offset;
            }

            std::uint32_t class_index() const
            {
                return this->class_reference;
            }

            std::uint32_t type_index() const
            {
                return this->type_reference;
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
        };

    public:
        class_file(char *memory_mapped_file) : memory_mapped_file(memory_mapped_file) {}

        typedef fixed_width_range<class_reference> class_range;
        typedef fixed_width_range<method_reference> method_range;
        typedef fixed_width_range<static_reference> static_range;
        typedef fixed_width_range<virtual_reference> virtual_range;

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
            return method_range(start + sizeof(std::uint32_t) * 2, start + sizeof(std::uint32_t) * 2 + method_count * sizeof(method_reference));
        }

        auto statics() const
        {
            char *start = this->static_references_start();
            std::uint64_t static_count = oops::utils::pun_read<std::uint32_t>(start);
            return static_range(start + sizeof(std::uint32_t) * 2, start + sizeof(std::uint32_t) * 2 + static_count * sizeof(static_reference));
        }

        auto virtuals() const
        {
            char *start = this->virtual_references_start();
            std::uint64_t virtual_count = oops::utils::pun_read<std::uint32_t>(start);
            return virtual_range(start + sizeof(std::uint32_t) * 2, start + sizeof(std::uint32_t) * 2 + virtual_count * sizeof(virtual_reference));
        }

        std::uint64_t bytecode_size() const
        {
            return oops::utils::pun_read<std::uint64_t>(this->bytecodes_start());
        }

        std::uint64_t string_pool_size() const
        {
            return oops::utils::pun_read<std::uint64_t>(this->string_pool_start());
        }

        std::uint64_t commit_size() const
        {
            return this->string_pool_size() + this->bytecode_size() + this->classes().size() * sizeof(char *) + (this->methods().size() + this->statics().size() + this->virtuals().size()) * (sizeof(char *) + sizeof(std::uint32_t)) + 8 * sizeof(std::uint32_t);
        }
    };

    std::optional<char *> mmap_file(oops::utils::ostring name)
    {
        //TODO
        return {};
    }
} // namespace

oops::objects::clazz class_manager::load_class(utils::ostring name)
{
    auto loaded = this->loaded_classes.find(name);
    if (loaded != this->loaded_classes.end())
    {
        return objects::clazz(loaded->second.first);
    }
    auto maybe_file = ::mmap_file(name);
    if (maybe_file)
    {
        ::class_file file(*maybe_file);
        std::array<std::uint32_t, static_cast<unsigned int>(objects::field::type::VOID)> self_counts = {};

        std::unordered_map<objects::field::type, std::uint32_t> self_counts;
        for (auto static_var : file.statics())
        {
            if (static_var.class_index() == static_cast<std::uint32_t>(objects::field::type::DOUBLE) + 1)
            {
                ++self_counts[std::min(static_cast<std::uint32_t>(objects::field::type::OBJECT), static_var.type_index())];
            }
        }
        std::uint64_t static_size = self_counts[static_cast<std::uint32_t>(objects::field::type::CHAR)];
        static_size += static_cast<std::uint64_t>(self_counts[static_cast<std::uint32_t>(objects::field::type::SHORT)]) * sizeof(std::uint16_t);
        static_size += static_cast<std::uint64_t>(self_counts[static_cast<std::uint32_t>(objects::field::type::INT)]) * sizeof(std::uint32_t);
        static_size += static_cast<std::uint64_t>(self_counts[static_cast<std::uint32_t>(objects::field::type::LONG)]) * sizeof(std::uint64_t);
        static_size += static_cast<std::uint64_t>(self_counts[static_cast<std::uint32_t>(objects::field::type::FLOAT)]) * sizeof(float);
        static_size += static_cast<std::uint64_t>(self_counts[static_cast<std::uint32_t>(objects::field::type::DOUBLE)]) * sizeof(double);
        static_size += static_cast<std::uint64_t>(self_counts[static_cast<std::uint32_t>(objects::field::type::OBJECT)]) * sizeof(char *);
        auto commit_size = std::min(static_cast<std::uint64_t>(this->cap - this->head), file.commit_size() + ((static_size + alignof(std::uint64_t) - 1) & ~(alignof(std::uint64_t) - 1)));
        if (platform::commit(this->head, commit_size))
        {
            this->head += commit_size;
        }
    }
    //Shouldn't happen yet cause lazy
    return objects::clazz(nullptr);
}