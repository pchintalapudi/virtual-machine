#include "class_manager.h"
#include "../objects/objects.h"
#include "../memory/memutils.h"
#include "../platform_specific/memory.h"
#include "../platform_specific/files.h"

#include <numeric>
#include <set>
#include <tuple>

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

        std::uint64_t commit_size(std::uint64_t static_memory_size, std::uint64_t self_scount, std::uint64_t self_vcount, std::uint64_t self_mcount, std::uint64_t vmcount) const
        {
            return this->string_pool_size() + this->bytecode_size() + self_scount * sizeof(char *) + self_vcount * sizeof(char *) + self_mcount * sizeof(char *) + this->statics().size() * sizeof(char *) + this->virtuals().size() * sizeof(char *) + this->methods().size() * sizeof(char *) + static_memory_size + this->classes().size() * sizeof(char *) + vmcount * sizeof(char *) + sizeof(char *) + sizeof(std::uint32_t) * 12 + (self_mcount + self_scount + self_vcount) * sizeof(std::uint32_t);
        }

        oops::utils::ostring get_string(std::uint64_t offset) const
        {
            return this->memory_mapped_file + offset;
        }
    };

    std::optional<std::pair<class_file, oops::platform::file_mapping>> mmap_file(oops::utils::ostring name)
    {
        auto mapping = oops::platform::open_file_mapping(name);
        if (mapping) {
            return {{mapping->mmapped_file, *mapping}};
        } else {
            return {};
        }
    }

    void munmap_file(oops::platform::file_mapping mapping)
    {
        oops::platform::close_file_mapping(mapping);
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

    std::size_t size(oops::objects::field::type type)
    {
        switch (type)
        {
        case oops::objects::field::type::CHAR:
            return sizeof(std::int8_t);
        case oops::objects::field::type::SHORT:
            return sizeof(std::int16_t);
        case oops::objects::field::type::INT:
            return sizeof(std::int32_t);
        case oops::objects::field::type::LONG:
            return sizeof(std::int64_t);
        case oops::objects::field::type::FLOAT:
            return sizeof(float);
        case oops::objects::field::type::DOUBLE:
            return sizeof(double);
        default:
            return sizeof(void *);
        }
    }
} // namespace

oops::objects::clazz class_manager::load_class(utils::ostring name)
{
    constexpr std::uint64_t self_index = static_cast<unsigned>(objects::field::type::DOUBLE) + 1;
    auto loaded = this->loaded_classes.find(name);
    if (loaded != this->loaded_classes.end())
    {
        return objects::clazz(loaded->second.first);
    }
    if (auto maybe_cls = ::mmap_file(name))
    {
        auto [cls, mapping] = *maybe_cls;

        auto classes = cls.classes();
        auto methods = cls.methods();
        auto statics = cls.statics();
        auto virtuals = cls.virtuals();

        //Preload inherited classes
        std::vector<objects::clazz> inherited;
        inherited.reserve(cls.implement_count());
        auto inherit_range = classes.slice(self_index + 1, self_index + 1 + cls.implement_count());
        std::transform(inherit_range.begin(), inherit_range.end(), std::back_inserter(inherited), [this, cls = cls](auto cls_ref) { return this->load_class(cls.get_string(*cls_ref)); });

        std::array<std::uint32_t, __builtin_ctz(std::max({alignof(double), alignof(std::uint64_t), alignof(void *)})) + 2> static_sizes{};
        for (auto static_var : statics)
        {
            if (static_var.class_index() == self_index)
            {
                static_sizes[__builtin_ctz(::size(static_cast<objects::field::type>(std::min(static_cast<unsigned>(static_var.type_index()), static_cast<unsigned>(objects::field::type::OBJECT)))))]++;
                static_sizes[static_sizes.size() - 1] += static_var.type_index() >= self_index;
            }
        }
        auto static_memory_size = memory::align_up(std::accumulate(static_sizes.begin(), static_sizes.end(), static_cast<std::uint64_t>(0), [index = 0](std::uint64_t old, std::uint64_t next) mutable { return old + (next << index++); }));

        std::array<std::uint32_t, __builtin_ctz(std::max({alignof(double), alignof(std::uint64_t), alignof(void *)})) + 2> virtual_sizes{};
        for (auto virtual_var : virtuals)
        {
            if (virtual_var.class_index() == self_index)
            {
                virtual_sizes[__builtin_ctz(::size(static_cast<objects::field::type>(std::min(static_cast<unsigned>(virtual_var.type_index()), static_cast<unsigned>(objects::field::type::OBJECT)))))]++;
                virtual_sizes[virtual_sizes.size() - 1] += virtual_var.type_index() >= self_index;
            }
        }
        std::uint64_t inherited_size = inherited.empty() ? 0 : inherited[0].object_malloc_required_size() - sizeof(std::uint32_t) * 2 - sizeof(char *), virtual_memory_size = inherited_size + sizeof(std::uint32_t) * 2 + sizeof(char *) + memory::align_up(std::accumulate(static_sizes.begin(), static_sizes.end(), static_cast<std::uint64_t>(0), [index = 0](std::uint64_t old, std::uint64_t next) mutable { return old + (next << index++); }));

        std::uint64_t self_method_count = 0, self_static_method_count = 0;
        std::unordered_map<std::uint64_t, std::uint32_t> overrides;
        for (auto method : methods)
        {
            if (method.class_index() == self_index)
            {
                if (method.type_index() != static_cast<std::uint32_t>(objects::method::type::STATIC))
                {
                    if (!inherited.empty())
                    {
                        if (auto override_offset = inherited[0].get_real_method_offset(cls.get_string(*method)))
                        {
                            overrides.emplace(*method, *override_offset);
                        }
                    }
                }
                else
                {
                    self_static_method_count++;
                }
                self_method_count++;
            }
        }
        std::uint32_t inherited_method_count = inherited.empty() ? 0 : inherited[0].virtual_method_count(), vm_count = self_method_count - self_static_method_count - overrides.size() + inherited_method_count;

        std::uint64_t static_count = std::accumulate(static_sizes.begin(), static_sizes.end(), 0ull), virtual_count = std::accumulate(virtual_sizes.begin(), virtual_sizes.end(), 0ull);
        auto commit_size = cls.commit_size(static_memory_size, static_count, virtual_count, self_method_count, vm_count);
        if (platform::commit(this->head, commit_size))
        {
            //TODO finalizable classes
            //Copy class head data
            utils::pun_write(this->head, memory::size64to32(virtual_memory_size) << 1);
            utils::pun_write(this->head + sizeof(std::uint32_t), static_memory_size >> __builtin_ctz(sizeof(std::uint64_t)));
            utils::pun_write(this->head + sizeof(std::uint32_t) * 2, static_cast<std::uint32_t>(virtual_sizes[virtual_sizes.size() - 1]));
            utils::pun_write(this->head + sizeof(std::uint32_t) * 3, static_cast<std::uint32_t>(static_sizes[static_sizes.size() - 1]));
            utils::pun_write(this->head + sizeof(std::uint32_t) * 4, static_cast<std::uint32_t>(methods.size()));
            utils::pun_write(this->head + sizeof(std::uint32_t) * 5, static_cast<std::uint32_t>(virtuals.size()));
            utils::pun_write(this->head + sizeof(std::uint32_t) * 6, static_cast<std::uint32_t>(statics.size()));
            utils::pun_write(this->head + sizeof(std::uint32_t) * 7, static_cast<std::uint32_t>(classes.size()));
            utils::pun_write(this->head + sizeof(std::uint32_t) * 8, static_cast<std::uint32_t>(vm_count));
            utils::pun_write(this->head + sizeof(std::uint32_t) * 9, static_cast<std::uint32_t>(self_method_count));
            utils::pun_write(this->head + sizeof(std::uint32_t) * 10, static_cast<std::uint32_t>(virtual_count));
            utils::pun_write(this->head + sizeof(std::uint32_t) * 11, static_cast<std::uint32_t>(static_count));
            utils::pun_write(this->head + sizeof(std::uint32_t) * 12, inherited.empty() ? nullptr : inherited[0].unwrap());

            char *vtable = this->head + sizeof(std::uint32_t) * 12 + sizeof(char *);
            std::uint64_t vtable_offset = 0;

            if (!inherited.empty())
            {
                std::memcpy(vtable, inherited[0].vtable(), vtable_offset = static_cast<std::uint64_t>(inherited[0].virtual_method_count()) * sizeof(char *));
            }

            char *ctable = vtable + sizeof(char *) * (self_static_method_count + vm_count);
            for (std::uint64_t i = 0; i < self_index; i++)
            {
                utils::pun_write(ctable + i * sizeof(char *), nullptr);
            }
            utils::pun_write(ctable + self_index * sizeof(char *), this->head);
            std::for_each(inherited.begin(), inherited.end(), [i = static_cast<std::uint64_t>(0), ctable](objects::clazz clz) mutable { utils::pun_write(ctable + ++i * sizeof(char *), clz.unwrap()); });
            ctable += sizeof(char *) + sizeof(char *) * inherited.size();

            char *static_memory_start = ctable + sizeof(char *) * classes.size();

            char *methods_start = static_memory_start + static_memory_size;

            char *virtual_variables_start = methods_start + sizeof(char *) * methods.size();

            char *static_variables_start = virtual_variables_start + sizeof(char *) * virtuals.size();

            char *mtable_start = static_variables_start + sizeof(char *) * statics.size();

            char *itable_start = mtable_start + sizeof(char *) * self_method_count;

            char *stable_start = itable_start + sizeof(char *) * virtual_count;

            char *string_start = stable_start + sizeof(char *) * static_count;

            char *bytecode_start = string_start + (self_method_count + virtual_count + static_count) * sizeof(std::uint32_t) + cls.string_pool_size();

            for (auto cref : classes.slice(self_index + implemented.size()))
            {
                utils::pun_write(ctable, string_start + 1);
                string_start = ::load_ostring(string_start, cls.get_string(*cref));
                ctable += sizeof(char *);
            }

            std::vector<utils::ostring> self_methods;
            auto bytecode_it = cls.bytecode_start();
            std::uint32_t static_method_offset = vtable_offset + self_method_count - self_static_method_count - overrides.size();
            for (auto mref : methods)
            {
                if (mref.class_index() == self_index)
                {
                    std::uint32_t method_index;
                    if (mref.type_index() == static_cast<std::uint32_t>(objects::method::type::STATIC))
                    {
                        method_index = static_method_offset++;
                    }
                    else if (auto it = overrides.find(*mref); it != overrides.end())
                    {
                        method_index = it->second;
                    }
                    else
                    {
                        method_index = vtable_offset++;
                    }
                    self_methods.emplace_back(string_start + sizeof(std::uint32_t) * 2);
                    utils::pun_write(methods_start, bytecode_start);
                    auto [code, size] = *bytecode_it;
                    utils::pun_write(bytecode_start, this->head);
                    std::memcpy(bytecode_start + sizeof(char *), code, size);
                    utils::pun_write(vtable + static_cast<std::uintptr_t>(method_index) * sizeof(char*), bytecode_start);
                    bytecode_start += sizeof(char *) + size;
                    ++bytecode_it;
                    utils::pun_write(string_start, method_index);
                }
                else
                {
                    utils::pun_write(methods_start, string_start + 1);
                    utils::pun_write(string_start, mref.class_index());
                }
                string_start = ::load_ostring(string_start + sizeof(std::uint32_t), cls.get_string(*mref));
                methods_start += sizeof(char *);
            }

            decltype(virtual_sizes) virtual_heads;
            virtual_heads[virtual_heads.size() - 1] = inherited_size;
            virtual_heads[virtual_heads.size() - 2] = inherited_size + sizeof(char *) * virtual_sizes[virtual_sizes.size() - 1];
            for (unsigned i = virtual_heads.size() - 2; i-- > 1;)
            {
                virtual_heads[i - 1] = (virtual_sizes[i] << i) + virtual_heads[i];
            }
            std::vector<utils::ostring> self_virtuals;
            for (auto vref : virtuals)
            {
                if (vref.class_index() == self_index)
                {
                    auto vsize = ::size(static_cast<objects::field::type>(std::min(static_cast<unsigned>(vref.type_index()), static_cast<unsigned>(objects::field::type::OBJECT))));
                    bool pointer = vref.type_index() >= self_index;
                    std::uint32_t offset;
                    if (vsize < sizeof(char *))
                    {
                        offset = virtual_heads[__builtin_ctz(vsize)];
                        virtual_heads[__builtin_ctz(vsize)] += vsize;
                    }
                    else if (pointer)
                    {
                        offset = virtual_heads[virtual_heads.size() - 1];
                        virtual_heads[virtual_heads.size() - 1] += sizeof(char *);
                    }
                    else
                    {
                        offset = virtual_heads[__builtin_ctz(sizeof(char *))];
                        virtual_heads[__builtin_ctz(sizeof(char *))] += sizeof(char *);
                    }
                    utils::pun_write(virtual_variables_start, (static_cast<std::uintptr_t>(offset) << ((sizeof(std::uintptr_t) - sizeof(std::uint32_t)) * CHAR_BIT)) | 1);
                    utils::pun_write(string_start, offset);
                    self_virtuals.emplace_back(string_start + sizeof(std::uint32_t));
                }
                else
                {
                    utils::pun_write(virtual_variables_start, string_start + 1);
                    utils::pun_write(string_start, vref.class_index());
                }
                string_start = ::load_ostring(string_start + sizeof(std::uint32_t), cls.get_string(*vref));
                virtual_variables_start += sizeof(char *);
            }

            decltype(static_sizes) static_heads;
            static_heads[static_heads.size() - 1] = 0;
            static_heads[static_heads.size() - 2] = static_sizes[static_sizes.size() - 1] * sizeof(char *);
            for (unsigned i = static_heads.size() - 2; i-- > 1;)
            {
                static_heads[i - 1] = (static_sizes[i] << i) + static_heads[i];
            }
            std::vector<utils::ostring> self_statics;
            for (auto sref : statics)
            {
                if (sref.class_index() == self_index)
                {
                    auto ssize = ::size(static_cast<objects::field::type>(std::min(static_cast<unsigned>(sref.type_index()), static_cast<unsigned>(objects::field::type::OBJECT))));
                    bool pointer = sref.type_index() >= self_index;
                    std::uint32_t offset;
                    if (ssize < sizeof(char *))
                    {
                        offset = static_heads[__builtin_ctz(ssize)];
                        static_heads[__builtin_ctz(ssize)] += ssize;
                    }
                    else if (pointer)
                    {
                        offset = static_heads[static_heads.size() - 1];
                        static_heads[static_heads.size() - 1] += sizeof(char *);
                    }
                    else
                    {
                        offset = static_heads[__builtin_ctz(sizeof(char *))];
                        static_heads[__builtin_ctz(sizeof(char *))] += sizeof(char *);
                    }
                    utils::pun_write(static_variables_start, static_memory_start + offset);
                    utils::pun_write(string_start, offset);
                    self_statics.emplace_back(string_start + sizeof(std::uint32_t));
                }
                else
                {
                    utils::pun_write(static_variables_start, string_start + 1);
                    utils::pun_write(string_start, sref.class_index());
                }
                string_start = ::load_ostring(string_start + sizeof(std::uint32_t), cls.get_string(*sref));
                static_variables_start += sizeof(char *);
            }

            std::sort(self_methods.begin(), self_methods.end());
            for (auto string : self_methods)
            {
                utils::pun_write(methods_start, string.unwrap() - sizeof(std::uint32_t) * 2);
                methods_start += sizeof(char *);
            }

            std::sort(self_virtuals.begin(), self_virtuals.end());
            for (auto string : self_virtuals)
            {
                utils::pun_write(virtual_variables_start, string.unwrap() - sizeof(std::uint32_t) * 2);
                virtual_variables_start += sizeof(char *);
            }

            std::sort(self_statics.begin(), self_statics.end());
            for (auto string : self_statics)
            {
                utils::pun_write(static_variables_start, string.unwrap() - sizeof(std::uint32_t) * 2);
                static_variables_start += sizeof(char *);
            }

            this->head = bytecode_start;

            //Handle instanceof invariants
            std::set<char *> implemented;
            for (auto impl : inherited)
            {
                implemented.insert(impl.unwrap());
                auto impl_index = this->loaded_classes[impl.unwrap()].second;
                std::transform(this->implemented.begin() + impl_index + 1, this->implemented.begin() + impl_index + 1 + this->implemented[impl_index], std::inserter(implemented, implemented.begin()), [](std::uintptr_t ptr) { return utils::pun_reinterpret<char *>(ptr); });
            }
            this->implemented.push_back(utils::pun_reinterpret<std::uintptr_t>(this->head));
            std::transform(implemented.begin(), implemented.end(), std::back_inserter(this->implemented), [](char *ptr) { return utils::pun_reinterpret<std::uintptr_t>(ptr); });
        }
        ::munmap_file(mapping);
    }
    return objects::clazz(nullptr);
}