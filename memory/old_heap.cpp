#include "old_heap.h"

#include "memutils.h"
#include "../utils/utils.h"

#include "../platform_specific/memory.h"

using namespace oops::memory;

constexpr std::size_t min_free_memory_bytes = 24; //2 free pointers + size

namespace
{
    class rb_tree_node
    {
    private:
        char *real;
        bool red;

    public:
        rb_tree_node(char *rb_pointer) : real(oops::utils::pun_reinterpret<char *>(oops::utils::pun_reinterpret<uintptr_t>(rb_pointer) >> 1 << 1)), red(oops::utils::pun_reinterpret<std::uintptr_t>(rb_pointer) & 1) {}

        std::size_t size() const
        {
            return size32to64(oops::utils::pun_read<std::uint32_t>(this->real - sizeof(std::uint32_t)));
        }

        void recolor(bool red)
        {
            this->red = red;
        }

        bool is_red() const
        {
            return this->red;
        }

        char *unwrap() const
        {
            return this->real;
        }

        char *rb_pointer() const
        {
            return oops::utils::pun_reinterpret<char *>(oops::utils::pun_reinterpret<std::uintptr_t>(this->real) | red);
        }

        rb_tree_node left() const
        {
            return rb_tree_node(oops::utils::pun_read<char *>(this->real));
        }

        rb_tree_node right() const
        {
            return rb_tree_node(oops::utils::pun_read<char *>(this->real + sizeof(char *)));
        }

        static bool is_left(char *parent, char *colored)
        {
            return ((oops::utils::pun_read<std::uintptr_t>(parent) ^ oops::utils::pun_reinterpret<std::uintptr_t>(colored)) >> 1) == 0;
        }

        bool is_left(rb_tree_node child) const
        {
            return is_left(this->real, child.real);
        }

        operator bool() const
        {
            return this->real != nullptr;
        }

        void recolor_unknown_child(rb_tree_node child)
        {
            this->replace(child, child);
        }

        void set_left_child(rb_tree_node child)
        {
            oops::utils::pun_write(this->real, child.rb_pointer());
        }

        void set_right_child(rb_tree_node child)
        {
            oops::utils::pun_write(this->real + sizeof(char *), child.rb_pointer());
        }

        void replace(rb_tree_node child, rb_tree_node replacement)
        {
            oops::utils::pun_write(this->real + this->is_left(child) * sizeof(char *), replacement.rb_pointer());
        }

        rb_tree_node sibling(rb_tree_node child)
        {
            return this->is_left(child) ? this->right() : this->left();
        }

        void swap_pointers(rb_tree_node other)
        {
            using std::swap;
            swap(other.real, this->real);
        }
    };

    constexpr std::size_t max_bits_per_pointer = 47, rb_stack_size = (max_bits_per_pointer + 1) * 2;
    constexpr bool red = true, black = false;

    //tree_root must not be nullptr
    void rb_tree_insert(char **tree_root, char *free_node)
    {
        char *stack[rb_stack_size];
        std::size_t length = 0;
        //Insert
        rb_tree_node head(*tree_root), to_insert(free_node);
        to_insert.recolor(red);
        std::uint64_t to_insert_size = to_insert.size();
        do
        {
            stack[length++] = head.rb_pointer();
            auto head_size = head.size();
            head = to_insert_size < head_size or (head_size == to_insert_size and to_insert.unwrap() < head.unwrap()) ? head.left() : head.right();
        } while (head);
        head = stack[length - 1];
        stack[length++] = to_insert.rb_pointer();
        if (head.size() < to_insert_size)
        {
            head.set_left_child(to_insert);
        }
        else
        {
            head.set_right_child(to_insert);
        }
        //Repair
        auto node = to_insert;
        auto parent = head;
        while (length > 2)
        {
            if (parent.is_red())
            {
                rb_tree_node grandparent(stack[length - 2]);
                auto uncle = grandparent.sibling(parent);
                if (uncle.is_red())
                {
                    parent.recolor(black);
                    uncle.recolor(black);
                    grandparent.recolor(red);
                    if (grandparent.is_left(parent))
                    {
                        grandparent.set_left_child(parent);
                        grandparent.set_right_child(uncle);
                    }
                    else
                    {
                        grandparent.set_right_child(parent);
                        grandparent.set_left_child(uncle);
                    }
                    node = grandparent;
                    parent = stack[length - 1];
                    parent.recolor_unknown_child(grandparent);
                    length -= 2;
                    continue;
                }
            }
            break;
        }
        switch (length)
        {
        case 1:
            //Only node left is the root, which can't be recolored :)
            break;
        default:
#define reroot(replacement)                                                \
    if (length > 3)                                                        \
        rb_tree_node(stack[length - 3]).replace(grandparent, replacement); \
    else                                                                   \
        *tree_root = replacement.unwrap()
            //length is greater than 1, so either the parent is black or the uncle is black
            if (parent.is_red())
            {
                //Uncle is black, so now let's start rotating
                rb_tree_node grandparent(stack[length - 2]);
                if (grandparent.is_left(parent))
                {
                    if (parent.is_left(node))
                    {
                        //Left-left
                        grandparent.set_left_child(parent.right());
                        grandparent.recolor(red);
                        parent.set_right_child(grandparent);
                        parent.recolor(black);
                        reroot(parent);
                        //Rotations complete
                    }
                    else
                    {
                        //Left-right
                        parent.set_right_child(node.left());
                        grandparent.set_left_child(node.right());
                        grandparent.recolor(red);
                        node.set_left_child(parent);
                        node.set_right_child(grandparent);
                        node.recolor(black);
                        reroot(node);
                        //Rotations complete
                    }
                }
                else
                {
                    if (parent.is_left(node))
                    {
                        //Right-left
                        grandparent.set_right_child(node.left());
                        parent.set_left_child(node.right());
                        grandparent.recolor(red);
                        node.set_left_child(grandparent);
                        node.set_right_child(parent);
                        node.recolor(black);
                        reroot(node);
                    }
                    else
                    {
                        //Right-right
                        grandparent.set_right_child(parent.left());
                        grandparent.recolor(red);
                        parent.set_left_child(grandparent);
                        parent.recolor(black);
                        reroot(parent);
                    }
                }
            }
            else
            {
                //Parent is black, so RB property is restored and no rotations must be done
                break;
            }
        }
    }
    std::optional<char *> rb_tree_fixup_double_black(rb_tree_node double_black, char **stack, std::size_t length)
    {
        while (length)
        {
            //While we can't kill a red node to exhaust the double black we must propagate upwards
            rb_tree_node parent(stack[length - 1]);
            if (!parent.is_red())
            {
                auto sibling = parent.sibling(double_black);
                if (!sibling.is_red())
                {
                    if (!sibling.right().is_red() and !sibling.left().is_red())
                    {
                        double_black = parent;
                        length--;
                        sibling.recolor(red);
                        parent.recolor_unknown_child(sibling);
                        continue;
                    }
                }
            }
            break;
        }
        if (!length)
            return {};

#define replace_root(replacement)                                 \
    if (length == 1)                                              \
        return {replacement.rb_pointer()};                        \
    rb_tree_node(stack[length - 2]).replace(parent, replacement); \
    return {}
        //There exists some killable red node nearby, or we'd still be in the loop
        rb_tree_node parent(stack[length - 1]);
        if (parent.is_left(double_black))
        {
            rb_tree_node sibling = parent.right();
            rb_tree_node niece = sibling.left();
            rb_tree_node nephew = sibling.right();
            if (sibling.is_red())
            {
                //Sibling is red, so parent is black and sibling's children are black
                auto niece_daughter = niece.left();
                auto niece_son = niece.right();
                if (niece_son.is_red())
                {
                    parent.set_right_child(niece_daughter);
                    niece.set_left_child(parent);
                    niece_son.recolor(black);
                    niece.set_right_child(niece_son);
                    niece.recolor(red);
                    sibling.set_left_child(niece);
                }
                else if (niece_daughter.is_red())
                {
                    parent.set_right_child(niece_daughter.left());
                    niece.set_left_child(niece_daughter.right());
                    niece_daughter.set_left_child(parent);
                    niece_daughter.set_right_child(niece);
                    sibling.set_left_child(niece_daughter);
                }
                else
                {
                    //Only sibling is red in extended family
                    niece.recolor(red);
                    parent.set_right_child(niece);
                    sibling.set_left_child(parent);
                }
                sibling.recolor(black);
                replace_root(sibling);
            }
            else
            {
                //Sibling is black
                if (parent.is_red())
                {
                    if (nephew.is_red())
                    {
                        sibling.recolor(red);
                        parent.set_right_child(niece);
                        sibling.set_left_child(parent);
                        nephew.recolor(black);
                        sibling.set_right_child(nephew);
                        replace_root(sibling);
                    }
                    else if (niece.is_red())
                    {
                        parent.recolor(black);
                        parent.set_right_child(niece.left());
                        sibling.set_left_child(niece.right());
                        niece.set_left_child(parent);
                        niece.set_right_child(sibling);
                        replace_root(niece);
                    }
                    else
                    {
                        //Only parent is black
                        parent.recolor(black);
                        sibling.recolor(red);
                        parent.set_right_child(sibling);
                        replace_root(parent);
                    }
                }
                else
                {
                    //Sibling and parent is black
                    if (nephew.is_red())
                    {
                        parent.set_right_child(niece);
                        sibling.set_left_child(parent);
                        nephew.recolor(black);
                        sibling.set_right_child(nephew);
                        replace_root(sibling);
                    }
                    else
                    {
                        //Niece is red and all others are black
                        parent.set_right_child(niece.left());
                        sibling.set_left_child(niece.right());
                        niece.set_left_child(parent);
                        niece.set_right_child(sibling);
                        niece.recolor(black);
                        replace_root(niece);
                    }
                }
            }
        }
        else
        {
            auto sibling = parent.left();
            auto niece = sibling.left();
            auto nephew = sibling.right();
            if (sibling.is_red())
            {
                if (auto nephew_daughter = nephew.left(); nephew_daughter.is_red())
                {
                    //Sibling and nephew daughter are red
                    nephew.set_right_child(parent);
                    parent.set_left_child(nephew.right());
                    nephew_daughter.recolor(black);
                    nephew.set_left_child(nephew_daughter);
                    nephew.recolor(red);
                    sibling.set_right_child(nephew);
                }
                else if (auto nephew_son = nephew.right(); nephew_son.is_red())
                {
                    //Sibling and nephew son are red
                    nephew.set_right_child(nephew_son.left());
                    parent.set_left_child(nephew_son.right());
                    nephew_son.set_left_child(nephew);
                    nephew_son.set_right_child(parent);
                    sibling.set_right_child(nephew_son);
                }
                else
                {
                    nephew.recolor(red);
                    parent.set_left_child(nephew);
                    sibling.set_right_child(parent);
                }
                sibling.recolor(black);
                replace_root(sibling);
            }
            else
            {
                if (parent.is_red())
                {
                    if (niece.is_red())
                    {
                        parent.recolor(black);
                        parent.set_left_child(nephew);
                        sibling.set_right_child(parent);
                        niece.recolor(black);
                        sibling.set_left_child(niece);
                        replace_root(sibling);
                    }
                    else if (nephew.is_red())
                    {
                        parent.set_left_child(nephew.right());
                        sibling.set_right_child(nephew.left());
                        parent.recolor(black);
                        nephew.set_right_child(parent);
                        nephew.set_left_child(sibling);
                        replace_root(nephew);
                    }
                    else
                    {
                        sibling.recolor(red);
                        parent.set_left_child(sibling);
                        parent.recolor(black);
                        replace_root(parent);
                    }
                }
                else
                {
                    if (niece.is_red())
                    {
                        parent.set_left_child(nephew);
                        sibling.set_right_child(parent);
                        niece.recolor(black);
                        sibling.set_left_child(niece);
                        replace_root(sibling);
                    }
                    else
                    {
                        //Nephew is the sacrificial red node
                        sibling.set_right_child(nephew.left());
                        parent.set_left_child(nephew.right());
                        nephew.set_left_child(sibling);
                        nephew.set_right_child(parent);
                        nephew.recolor(black);
                        replace_root(nephew);
                    }
                }
            }
        }
#undef replace_root
    }
    std::optional<char *> rb_tree_delete_single_child(rb_tree_node single_child, char **stack, std::size_t length)
    {
        auto left = single_child.left();
        if (left)
        {
            //Only black nodes can have a single child, and that child must be a red leaf
            left.recolor(black);
            rb_tree_node(stack[length - 1]).replace(single_child, left);
            return {};
        }
        auto right = single_child.right();
        if (right)
        {
            right.recolor(black);
            rb_tree_node(stack[length - 1]).replace(single_child, right);
            return {};
        }
        //Both left and right are null
        rb_tree_node null = left;
        if (single_child.is_red())
        {
            rb_tree_node(stack[length - 1]).replace(single_child, null);
            return {};
        }
        else
        {
            //Oh no we have a double black situation :(
            rb_tree_node(stack[length - 1]).replace(single_child, null);
            return rb_tree_fixup_double_black(null, stack, length);
        }
    }
    std::optional<char *> rb_tree_delete(rb_tree_node node, char **stack, std::size_t length)
    {
        if (!node.left())
        {
            if (length)
            {
                return rb_tree_delete_single_child(node, stack, length);
            }
            else
            {
                auto right = node.right();
                if (right)
                {
                    return {right.unwrap()};
                }
                else
                {
                    return {nullptr};
                }
            }
        }
        else if (!node.right())
        {
            if (length)
            {
                return rb_tree_delete_single_child(node, stack, length);
            }
            else
            {
                auto left = node.left();
                return left.unwrap();
            }
        }
        else
        {
            auto old_length = length;
            auto old_node = node;
            stack[length++] = node.rb_pointer();
            node = node.right();
            while (node)
            {
                stack[length++] = node.rb_pointer();
                node = node.left();
            }
            node = stack[--length];
            rb_tree_node node_parent(stack[length - 1]);
            node.swap_pointers(old_node);
            using std::swap;
            swap(node, old_node);
            if (old_length)
            {
                rb_tree_node(stack[old_length - 1]).replace(old_node, node);
            }
            rb_tree_node(stack[length - 1]).replace(node, old_node);
            stack[old_length] = node.rb_pointer();
            return rb_tree_delete_single_child(old_node, stack, length);
        }
    }
    std::pair<std::optional<char *>, std::optional<char *>> rb_find_and_delete_by_size(char **tree_root, std::size_t uncompressed_size)
    {
        rb_tree_node root(*tree_root);
        char *stack[rb_stack_size];
        std::size_t length = 0;
        auto root_size = root.size();
        while (root && root_size != uncompressed_size)
        {
            stack[length++] = root.rb_pointer();
            root = uncompressed_size < root.size() ? root.left() : root.right();
            if (root)
                root_size = root.size();
        }
#define delete_and_recover                               \
    auto new_root = rb_tree_delete(root, stack, length); \
    if (new_root)                                        \
        *tree_root = *new_root;                          \
    return                                               \
    {                                                    \
        root.unwrap(), {}                                \
    }
        if (root)
        {
            delete_and_recover;
        }
        else
        {
            root = stack[--length];
            if (root_size > uncompressed_size)
            {
                if (root_size > uncompressed_size + min_free_memory_bytes)
                {
                    auto new_root = rb_tree_delete(root, stack, length);
                    if (new_root)
                        *tree_root = *new_root;
                    auto compressed_size = oops::memory::size64to32(uncompressed_size);
                    oops::utils::pun_write(root.unwrap() - sizeof(std::uint32_t), compressed_size);
                    oops::utils::pun_write(root.unwrap() + uncompressed_size, compressed_size);
                    auto compressed_free = oops::memory::size64to32(root_size - uncompressed_size - sizeof(std::uint32_t) * 2);
                    oops::utils::pun_write(root.unwrap() + uncompressed_size + sizeof(std::uint32_t), compressed_free);
                    oops::utils::pun_write(root.unwrap() + uncompressed_size + sizeof(std::uint32_t) * 2, nullptr);
                    oops::utils::pun_write(root.unwrap() + uncompressed_size + sizeof(std::uint32_t) * 2 + sizeof(char *), nullptr);
                    oops::utils::pun_write(root.unwrap() + root_size, compressed_free);
                    return {root.unwrap(), root.unwrap() + uncompressed_size + sizeof(std::uint32_t) * 2};
                }
                else
                {
                    delete_and_recover;
                }
            }
            else
            {
                if (length)
                {
                    root = stack[--length];
                    if (root_size > uncompressed_size)
                    {
                        delete_and_recover;
                    }
                    else
                    {
                        return {{}, {}};
                    }
                }
                else
                {
                    return {{}, {}};
                }
            }
        }
#undef delete_and_recover
    }
    void rb_find_and_delete_exact(char **tree_root, char *free_node)
    {
        rb_tree_node root(*tree_root), search(free_node);
        char *stack[rb_stack_size];
        std::size_t root_size = root.size(), search_size = search.size(), length = 0;
        while (root_size != search_size)
        {
            stack[length++] = root.rb_pointer();
            if (search_size < root_size)
            {
                root = root.left();
            }
            else
            {
                root = root.right();
            }
        }
        while (root.unwrap() != free_node)
        {
            stack[length++] = root.rb_pointer();
            if (free_node < root.unwrap())
            {
                root = root.left();
            }
            else
            {
                root = root.right();
            }
        }
        auto new_root = rb_tree_delete(root, stack, length);
        if (new_root)
            *tree_root = *new_root;
    }

    class linked_list_node
    {
    private:
        char *real;

    public:
        linked_list_node(char *real) : real(real) {}

        char *unwrap() const
        {
            return this->real;
        }

        linked_list_node next() const
        {
            return oops::utils::pun_read<char *>(this->real);
        }

        linked_list_node prev() const
        {
            return oops::utils::pun_read<char *>(this->real + sizeof(char *));
        }

        operator bool() const
        {
            return this->real != nullptr;
        }

        void set_next(linked_list_node next)
        {
            oops::utils::pun_write(this->real, next.unwrap());
        }

        void set_prev(linked_list_node prev) const
        {
            oops::utils::pun_write(this->real, prev.unwrap());
        }
    };
    char *ll_pop(char *head)
    {
        linked_list_node front(head);
        auto next = front.next();
        if (next)
            next.set_prev(nullptr);
        return next.unwrap();
    }
    std::optional<char *> ll_drop(char *ll_node)
    {
        linked_list_node node(ll_node);
        auto prev = node.prev();
        auto next = node.next();
        if (prev)
        {
            if (next)
            {
                next.set_prev(prev);
            }
            prev.set_next(next);
            return {};
        }
        else
        {
            if (next)
            {
                next.set_prev(prev);
            }
            return next.unwrap();
        }
    }
    void ll_push(char *head, char *node)
    {
        linked_list_node old_head(head);
        linked_list_node new_head(node);
        if (old_head)
            old_head.set_prev(new_head);
        new_head.set_next(old_head);
        new_head.set_prev(nullptr);
    }
} // namespace

std::optional<char *> old_heap::allocate_memory(std::uint64_t memory_size)
{
    for (std::uint32_t idx = size64to32(memory_size); idx < old_heap::linked_list_count; idx++)
    {
        auto head = this->linked_lists[idx];
        if (head)
        {
            this->linked_lists[idx] = ll_pop(head);
            auto ll_size = size32to64(idx);
            if (ll_size - memory_size > min_free_memory_bytes)
            {
                auto compressed_size = memory::size64to32(memory_size);
                utils::pun_write(head - sizeof(std::uint32_t), compressed_size);
                utils::pun_write(head + memory_size, compressed_size);
                auto compressed_free = memory::size64to32(ll_size - memory_size - sizeof(std::uint32_t) * 2);
                utils::pun_write(head + memory_size + sizeof(std::uint32_t), compressed_free);
                utils::pun_write(head + ll_size, compressed_free);
                auto new_free_head = head + memory_size + sizeof(std::uint32_t) * 2;
                char *free_head = this->linked_lists[compressed_free];
                if (free_head)
                    ll_push(free_head, new_free_head);
                this->linked_lists[compressed_free] = new_free_head;
            }
            std::memset(head, 0, memory_size - sizeof(std::uint32_t) * 2);
            return head;
        }
    }
    auto rb_tree_idx = __builtin_clzll(memory_size);
    for (unsigned int idx = rb_tree_idx; idx < rb_trees.size(); idx++)
    {
        if (char **rb_tree = this->rb_trees.data() + rb_tree_idx; *rb_tree)
        {
            if (auto allocated = rb_find_and_delete_by_size(rb_tree, memory_size); allocated.first)
            {
                if (allocated.second)
                {
                    auto compressed_free = utils::pun_read<std::uint32_t>(*allocated.second - sizeof(std::uint32_t));
                    if (compressed_free < linked_list_count)
                    {
                        if (char *free_head = this->linked_lists[compressed_free]; free_head)
                            ll_push(free_head, *allocated.second);
                        this->linked_lists[compressed_free] = *allocated.second;
                    }
                    else
                    {
                        auto expanded_free = size32to64(compressed_free);
                        auto free_rb_idx = __builtin_clzll(expanded_free);
                        if (char *rb_head = this->rb_trees[free_rb_idx]; rb_head)
                        {
                            rb_tree_insert(this->rb_trees.data() + free_rb_idx, *allocated.second);
                        }
                        else
                        {
                            this->rb_trees[free_rb_idx] = *allocated.second;
                        }
                    }
                }
                std::memset(*allocated.first, 0, memory_size - sizeof(std::uint32_t) * 2);
                return *allocated.first;
            }
        }
    }
    return {};
}

std::optional<oops::objects::object> old_heap::allocate_object(oops::objects::clazz cls)
{
    auto memory_size = cls.object_malloc_required_size();
    if (auto memory = this->allocate_memory(memory_size); memory)
    {
        utils::pun_write(*memory, utils::pun_reinterpret<char *>(utils::pun_reinterpret<std::uintptr_t>(cls.unwrap()) | 1));
        return objects::object(*memory);
    }
    return {};
}

std::optional<oops::objects::array> old_heap::allocate_array(oops::objects::clazz acls, std::uint64_t memory_size)
{
    if (auto memory = this->allocate_memory(memory_size); memory)
    {
        utils::pun_write(*memory, utils::pun_reinterpret<std::uintptr_t>(acls.unwrap()) | 1);
        return objects::array(*memory);
    }
    return {};
}

bool old_heap::is_old_object(objects::base_object obj)
{
    return static_cast<std::uintptr_t>(obj.unwrap() - this->base) < static_cast<std::uintptr_t>(this->cap - this->base);
}
namespace
{
    class heap_iterator
    {
    private:
        char *pointer;

    public:
        heap_iterator(char *pointer) : pointer(pointer) {}
        std::uint64_t size() const
        {
            return size32to64(oops::utils::pun_read<std::uint32_t>(this->pointer - sizeof(std::uint32_t)));
        }

        heap_iterator &operator++()
        {
            this->pointer += this->size();
            return *this;
        }

        operator bool() const
        {
            return oops::utils::pun_read<std::uintptr_t>(this->pointer) & 0b1;
        }

        bool operator*() const
        {
            return oops::utils::pun_read<std::uintptr_t>(this->pointer) & 0b10;
        }

        void clear_mark() const
        {
            oops::utils::pun_write(this->pointer, oops::utils::pun_read<std::uintptr_t>(this->pointer) & ~static_cast<std::uintptr_t>(0b10));
        }

        bool operator<(const heap_iterator &other) const
        {
            return this->pointer < other.pointer;
        }

        bool operator==(const heap_iterator &other) const
        {
            return this->pointer == other.pointer;
        }

        bool operator<=(const heap_iterator &other) const
        {
            return this->pointer <= other.pointer;
        }

        bool operator>(const heap_iterator &other) const
        {
            return this->pointer > other.pointer;
        }

        bool operator>=(const heap_iterator &other) const
        {
            return this->pointer >= other.pointer;
        }

        bool operator!=(const heap_iterator &other) const
        {
            return this->pointer != other.pointer;
        }

        char *unwrap() const
        {
            return this->pointer;
        }

        std::intptr_t operator-(const heap_iterator &other) const
        {
            return this->pointer - other.pointer;
        }
    };
} // namespace
void old_heap::sweep()
{
    heap_iterator begin(this->base + sizeof(std::uint32_t) * 2), end(this->head), prev(nullptr);
    std::uint64_t free = 0;
    bool prev_free = false;
    while (begin < end)
    {
        if (begin)
        {
            //This was an allocated chunk
            if (!*begin)
            {
                //We have a newly freed block of memory, so time to do some collapsing
                if (prev_free)
                {
                    //We had a previous block free, so we need to backtrack a teensy bit to prep for coalescing
                    auto size = prev.size();
                    auto compressed = size64to32(size);
                    if (compressed < this->linked_list_count)
                    {
                        if (auto new_head = ::ll_drop(prev.unwrap()))
                        {
                            this->linked_lists[compressed] = *new_head;
                        }
                    }
                    else
                    {
                        auto rb_index = __builtin_clzll(size);
                        ::rb_find_and_delete_exact(this->rb_trees.data() + rb_index, prev.unwrap());
                    }
                }
                else
                {
                    prev = begin;
                }
                do
                {
                    if (not begin)
                    {
                        auto size = begin.size();
                        auto compressed = size64to32(size);
                        if (compressed < this->linked_list_count)
                        {
                            if (auto new_head = ::ll_drop(begin.unwrap()))
                            {
                                this->linked_lists[compressed] = *new_head;
                            }
                        }
                        else
                        {
                            auto rb_index = __builtin_clzll(size);
                            ::rb_find_and_delete_exact(this->rb_trees.data() + rb_index, begin.unwrap());
                        }
                    }
                    free += begin.size();
                    ++begin;
                } while (begin < end and (not begin or not*begin));
                auto size = begin - prev;
                auto compressed = size64to32(size);
                utils::pun_write(prev.unwrap() - sizeof(std::uint32_t), compressed);
                utils::pun_write(begin.unwrap() - sizeof(std::uint32_t) * 2, compressed);
                if (compressed < this->linked_list_count)
                {
                    ::ll_push(this->linked_lists[compressed], prev.unwrap());
                    this->linked_lists[compressed] = prev.unwrap();
                }
                else
                {
                    auto rb_index = __builtin_clzll(size);
                    ::rb_tree_insert(this->rb_trees.data() + rb_index, prev.unwrap());
                }
                if (begin < end)
                {
                    begin.clear_mark();
                }
                else
                {
                    this->finish_old_gc(free);
                    return;
                }
            }
            else
            {
                //this object is still in use
                begin.clear_mark();
            }
        }
        else
        {
            //Free memory, we ignore it
            prev = begin;
            free += begin.size();
        }
        ++begin;
    }
    this->finish_old_gc(free);
}

bool old_heap::init(args &init_args)
{
    if (auto base = platform::reserve(init_args.max_size)) {
        this->base = *base;
        this->cap = *base + init_args.max_size;
        this->requested_free_ratio = init_args.requested_free_ratio;
        this->allocation_granularity = init_args.allocation_granularity;
        std::fill(this->linked_lists.begin(), this->linked_lists.end(), nullptr);
        std::fill(this->rb_trees.begin(), this->rb_trees.end(), nullptr);
        if (platform::commit(*base, init_args.min_size)) {
            utils::pun_write(*base + sizeof(std::uint32_t), size64to32(init_args.min_size - sizeof(char*) * 2));
            utils::pun_write(*base + init_args.min_size - sizeof(std::uint32_t) * 2, size64to32(init_args.min_size - sizeof(char*) * 2));
            this->rb_trees[__builtin_clzll(init_args.min_size)] = *base + sizeof(std::uint32_t) * 2;
            return true;
        }
        platform::dereserve(*base);
    }
    return false;
}

void old_heap::deinit() {
    platform::dereserve(this->base);
}