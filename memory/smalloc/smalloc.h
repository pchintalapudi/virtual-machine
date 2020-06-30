#ifndef MEMORY_SMALLOC_SMALLOC_H
#define MEMORY_SMALLOC_SMALLOC_H

#include <cstdint>
#include <cstring>
#include <array>
#include <utility>
#include <vector>

#include "../../punning/puns.h"
#include "../objects.h"

namespace oops
{
    namespace memory
    {
        class memory_manager;
        class heap;
        class old_heap
        {
        private:
            constexpr static std::size_t min_block_count = 3, max_fixed_size = 0x80ull, max_lz = 0x1f; //24 byte minimum object size
            friend class memory_manager;
            friend class heap;

        private:
            class _rb_tree_free_node
            {
            private:
                char *real;
                bool red;

                std::uintptr_t get_real() const
                {
                    PUN(std::uintptr_t, ireal, &this->real);
                    return ireal;
                }

                void set_real(std::uintptr_t ireal)
                {
                    std::memcpy(&this->real, &ireal, sizeof(std::uintptr_t));
                }

            public:
                _rb_tree_free_node() {}

                explicit _rb_tree_free_node(char *real)
                {
                    PUN(std::uintptr_t, ireal, &real);
                    this->red = ireal & 1;
                    this->set_real(ireal & ~static_cast<std::uintptr_t>(1));
                }

                operator bool() const
                {
                    return this->real != nullptr;
                }

                char *unwrap() const
                {
                    std::uintptr_t partial = this->get_real();
                    partial |= this->red;
                    PUN(char *, unwrapped, &partial);
                    return unwrapped;
                }

                bool is_red() const
                {
                    return this->red;
                }

                void recolor(bool red)
                {
                    this->red = red;
                }

                _rb_tree_free_node left() const
                {
                    PUN(char *, left, this->real);
                    return _rb_tree_free_node(left);
                }

                _rb_tree_free_node right() const
                {
                    PUN(char *, right, this->real + sizeof(char *));
                    return _rb_tree_free_node(right);
                }

                void set_left(_rb_tree_free_node left)
                {
                    char *luw = left.unwrap();
                    std::memcpy(this->real, &luw, sizeof(char *));
                }

                void set_right(_rb_tree_free_node right)
                {
                    char *ruw = right.unwrap();
                    std::memcpy(this->real + sizeof(char *), &ruw, sizeof(char *));
                }

                bool is_left(_rb_tree_free_node child)
                {
                    return this->left() == child;
                }

                void reset(_rb_tree_free_node child)
                {
                    auto left = this->left();
                    if (child.real == left.real)
                    {
                        this->set_left(child);
                    }
                    else
                    {
                        this->set_right(child);
                    }
                }

                void replace(_rb_tree_free_node old_child, _rb_tree_free_node new_child)
                {
                    auto left = this->left();
                    if (old_child.real == left.real)
                    {
                        this->set_left(new_child);
                    }
                    else
                    {
                        this->set_right(new_child);
                    }
                }

                bool operator==(const _rb_tree_free_node &other)
                {
                    return other.real == this->real;
                }

                void clear_left()
                {
#pragma GCC diagnostic ignored "-Wsizeof-pointer-memaccess"
                    std::memset(this->real, 0, sizeof(char *));
#pragma GCC diagnostic pop
                }

                void clear_right()
                {
#pragma GCC diagnostic ignored "-Wsizeof-pointer-memaccess"
                    std::memset(this->real + sizeof(char *), 0, sizeof(char *));
#pragma GCC diagnostic pop
                }

                std::uint32_t compressed_size() const
                {
                    PUN(std::uint32_t, sz, this->real - 4);
                    return sz;
                }

                char *mem_pointer()
                {
                    return this->real;
                }
            };

            static void _rb_insert_node(_rb_tree_free_node head, _rb_tree_free_node to_insert);

            static void _rb_delete_node_double_black(_rb_tree_free_node *nodes, unsigned int length);

            static void _rb_delete_node(_rb_tree_free_node *nodes, unsigned int length);

            static unsigned int _rb_find_single_child_node(_rb_tree_free_node *nodes, unsigned int length);

            static void _rb_delete_node_single_child(_rb_tree_free_node *nodes, unsigned int length);

            static std::pair<std::pair<char *, std::uint32_t>, std::pair<char *, std::uint32_t>> _rb_find_and_delete_mem_region(char **mem_tree, std::uint32_t compressed_size)
            {
                _rb_tree_free_node head(*mem_tree);
                _rb_tree_free_node nodes[128];
                unsigned int length = 0;
                while (head)
                {
                    nodes[length++] = head;
                    if (head.compressed_size() < compressed_size)
                    {
                        head = head.right();
                    }
                    else if (head.compressed_size() > compressed_size)
                    {
                        head = head.left();
                    }
                    else
                    {
                        char *exact_match = head.mem_pointer();
                        if (length == 1)
                        {
                            *mem_tree = nullptr;
                        }
                        else
                        {
                            _rb_delete_node(nodes, length);
                        }
                        std::memset(exact_match, 0, objects::object::size32to64(compressed_size));
                        return {{exact_match, compressed_size}, {nullptr, 0}};
                    }
                }
                if (length)
                {
                    auto node = nodes[length - 1];
                    if (node.compressed_size() < compressed_size)
                    {
                        if (length > 1)
                        {
                            auto parent = nodes[--length - 1];
                            if (parent.is_left(node))
                            {
                                //Parent is bigger than compressed_size, so we can operate on that
                                node = parent;
                            }
                            else
                            {
                                //The parent is implicitly smaller than this node
                                //This is the literal maximum node, so we're not getting any better now
                                return {{nullptr, 0}, {nullptr, 0}};
                            }
                        }
                        else
                        {
                            return {{nullptr, 0}, {nullptr, 0}};
                        }
                    }
                    if (node.compressed_size() > compressed_size + min_block_count)
                    {
                        //The smallest node bigger than the requested size can be split, so let's do so
                        auto node_size = node.compressed_size();
                        auto split_size = node_size - compressed_size - 1;
                        auto ret = node.mem_pointer();
                        if (length == 1)
                        {
                            *mem_tree = nullptr;
                        }
                        else
                        {
                            _rb_delete_node(nodes, length);
                        }
                        std::memcpy(ret - 4, &compressed_size, sizeof(compressed_size));
                        auto expanded_size = objects::object::size32to64(compressed_size);
                        auto node_expanded_size = objects::object::size32to64(node_size);
                        std::memset(ret, 0, expanded_size);
                        std::memcpy(ret + expanded_size, &compressed_size, sizeof(compressed_size));
                        std::memcpy(ret + expanded_size + sizeof(compressed_size), &split_size, sizeof(split_size));
                        std::memcpy(ret + node_expanded_size, &split_size, sizeof(split_size));
                        return {{ret, compressed_size}, {ret + expanded_size + sizeof(compressed_size) + sizeof(split_size), split_size}};
                    }
                    else
                    {
                        //The smallest node bigger than the requested size cannot be split, so let's just give it back
                        auto size = node.compressed_size();
                        auto ret = node.mem_pointer();
                        if (length == 1)
                        {
                            *mem_tree = nullptr;
                        }
                        else
                        {
                            _rb_delete_node(nodes, length);
                        }
                        std::memset(ret, 0, objects::object::size32to64(size));
                        return {{ret, size}, {nullptr, 0}};
                    }
                }
                else
                {
                    return {{nullptr, 0}, {nullptr, 0}};
                }
            }

        private:
            char *begin, *committed, *end;
            std::array<char *, max_fixed_size> mem_lists;
            std::array<char *, max_lz> mem_trees;

        public:
            old_heap(char *begin, char *end) : begin(begin), committed(begin), end(end)
            {
                mem_lists.fill(nullptr);
                mem_trees.fill(nullptr);
            }
            char *allocate_object(std::uint32_t compressed_size)
            {
                std::uint64_t cmp_compressed_size = compressed_size;
                char *null = nullptr;
                for (auto list_index = cmp_compressed_size; list_index < max_fixed_size && list_index < 4 + cmp_compressed_size; list_index++)
                {
                    if (mem_lists[list_index])
                    {
                        //Yay easy just pop the node
                        PUN(char *, next, mem_lists[list_index]);
                        std::memcpy(next + sizeof(char *), &null, sizeof(null));
                        std::swap(next, mem_lists[list_index]);
                        std::memset(next, 0, objects::object::size32to64(list_index));
                        return next;
                    }
                }
                for (auto list_index = cmp_compressed_size + 4; list_index < max_fixed_size; list_index++)
                {
                    if (mem_lists[list_index])
                    {
                        PUN(char *, next, mem_lists[list_index]);
                        std::memcpy(next + sizeof(char *), &null, sizeof(null));
                        std::swap(next, mem_lists[list_index]);
                        //Need to split node
                        auto expanded_node_size = objects::object::size32to64(list_index);
                        auto expanded_size = objects::object::size32to64(compressed_size);
                        auto split_size = list_index - compressed_size - 1;
                        std::memcpy(next - sizeof(compressed_size), &compressed_size, sizeof(compressed_size));
                        std::memcpy(next + expanded_size, &compressed_size, sizeof(compressed_size));
                        char *split_node = next + expanded_size + sizeof(compressed_size) + sizeof(split_size);
                        std::memcpy(split_node - 4, &split_size, sizeof(split_size));
                        std::memcpy(next + expanded_node_size, &split_size, sizeof(split_size));
                        auto split_ll_head_node = mem_lists[split_size];
                        if (mem_lists[split_size])
                        {
                            std::memcpy(split_ll_head_node + sizeof(char *), &split_node, sizeof(split_node));
                        }
                        std::memcpy(split_node, &split_ll_head_node, sizeof(char *));
                        std::memcpy(next + sizeof(char *), &null, sizeof(null));
                        mem_lists[split_size] = split_node;
                        std::memset(next, 0, expanded_size);
                        return next;
                    }
                }
                for (unsigned int tree_index = __builtin_clz(compressed_size) + 1; tree_index-- > 0;)
                {
                    char **mem_tree = mem_trees.data() + tree_index;
                    if (mem_tree)
                    {
                        auto split = _rb_find_and_delete_mem_region(mem_tree, compressed_size);
                        if (split.first.first)
                        {
                            if (split.second.first)
                            {
                                //Need to reinsert free block
                                if (split.second.second < max_fixed_size)
                                {
                                    auto ll_node = split.second.first;
                                    auto list_index = split.second.second;
                                    if (mem_trees[list_index])
                                    {
                                        std::memcpy(ll_node, &mem_trees[list_index], sizeof(char *));
                                        std::memcpy(mem_trees[list_index] + sizeof(char *), &ll_node, sizeof(ll_node));
                                    }
                                    else
                                    {
                                        std::memcpy(ll_node, &null, sizeof(null));
                                    }
                                    std::memcpy(ll_node + sizeof(char *), &null, sizeof(null));
                                    mem_trees[list_index] = ll_node;
                                }
                                else
                                {
                                    char *rb_pointer = split.second.first;
                                    std::memcpy(rb_pointer, &null, sizeof(null));
                                    std::memcpy(rb_pointer + sizeof(char *), &null, sizeof(null));
                                    unsigned int tree_index = __builtin_clz(compressed_size);
                                    if (mem_trees[tree_index])
                                    {
                                        _rb_tree_free_node rb_node{rb_pointer};
                                        rb_node.recolor(true);
                                        _rb_insert_node(_rb_tree_free_node(mem_trees[tree_index]), rb_node);
                                    }
                                    else
                                    {
                                        mem_trees[tree_index] = rb_pointer;
                                    }
                                }
                            }
                            return split.first.first;
                        }
                    }
                }
                return nullptr;
            }
        };
    } // namespace memory
} // namespace oops
#endif