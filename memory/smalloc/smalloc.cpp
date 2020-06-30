#include "smalloc.h"
void oops::memory::old_heap::_rb_insert_node(oops::memory::old_heap::_rb_tree_free_node head, oops::memory::old_heap::_rb_tree_free_node to_insert)
{
    std::vector<_rb_tree_free_node> ancestors;
    std::uint32_t to_insert_sz = to_insert.compressed_size();
    do
    {
        bool left = to_insert_sz < head.compressed_size();
        ancestors.push_back(head);
        head = left ? head.left() : head.right();
    } while (head);
    auto location = ancestors.back();
    if (to_insert_sz < location.compressed_size())
    {
        location.set_left(to_insert);
    }
    else
    {
        location.set_right(to_insert);
    }
    ancestors.push_back(to_insert);
    switch (ancestors.size())
    {
        while (ancestors.size() > 3)
        {
            auto k = ancestors[ancestors.size() - 1];
            auto p = ancestors[ancestors.size() - 2];
            //Tree is balanced, escape
            if (!p.is_red())
                return;
            auto gp = ancestors[ancestors.size() - 3];
            auto ggp = ancestors[ancestors.size() - 4];
            auto parent_left = gp.is_left(p); //bool = parent is left child
            auto u = parent_left ? gp.right() : gp.left();
            if (u.is_red())
            {
                //uncle is red, recolor and propagate
                p.recolor(false);
                u.recolor(false);
                gp.recolor(true);
                ggp.reset(gp);
                if (!parent_left)
                    std::swap(p, u);
                gp.set_left(p);
                gp.set_right(u);
                ancestors.pop_back();
                ancestors.pop_back();
                //grandparent is now kid
                //proceed with while loop
            }
            else
            {
                //uncle is black, rotate and resolve
                if (parent_left)
                {
                    //Parent is left child
                    if (p.is_left(k))
                    {
                        //Left-left case, single rotate
                        p.recolor(false);
                        gp.recolor(true);
                        auto swap = p.right();
                        gp.set_left(swap);
                        p.set_right(gp);
                        ggp.replace(gp, p);
                    }
                    else
                    {
                        //Left-right case, double rotate
                        p.set_right(k.left());
                        k.set_left(p);
                        gp.recolor(true);
                        k.recolor(false);
                        gp.set_left(k.right());
                        k.set_right(gp);
                        ggp.replace(gp, k);
                    }
                }
                else
                {
                    //Parent is right child
                    if (p.is_left(k))
                    {
                        //right-left case, double rotate
                        p.set_left(k.right());
                        k.set_right(p);
                        gp.recolor(true);
                        k.recolor(false);
                        gp.set_right(k.left());
                        k.set_left(gp);
                        ggp.replace(gp, k);
                    }
                    else
                    {
                        //right-right case, single rotate
                        p.recolor(false);
                        gp.recolor(true);
                        auto swap = p.left();
                        gp.set_right(swap);
                        p.set_left(gp);
                        ggp.replace(gp, p);
                    }
                }
                return; //Tree was just balanced by rotations
            }
        }
        //Top node is black, red child is known safe
        if (ancestors.size() == 2)
            return;
        {
            auto k = ancestors[2];
            auto p = ancestors[1];
            auto gp = ancestors[0];
            auto parent_left = gp.is_left(p);
            auto u = parent_left ? gp.right() : gp.left();
            if (u.is_red())
            {
                p.recolor(false);
                u.recolor(false);
                if (!parent_left)
                    std::swap(p, u);
                gp.set_left(p);
                gp.set_right(u);
                return;
            }
            else
            {
                //uncle is black, rotate to resolve
                if (parent_left)
                {
                    //Parent is left child
                    if (p.is_left(k))
                    {
                        //Left-left case, single rotate
                        p.recolor(false);
                        gp.recolor(true);
                        auto swap = p.right();
                        gp.set_left(swap);
                        p.set_right(gp);
                    }
                    else
                    {
                        //Left-right case, double rotate
                        p.set_right(k.left());
                        k.set_left(p);
                        gp.recolor(true);
                        k.recolor(false);
                        gp.set_left(k.right());
                        k.set_right(gp);
                    }
                }
                else
                {
                    //Parent is right child
                    if (p.is_left(k))
                    {
                        //right-left case, double rotate
                        p.set_left(k.right());
                        k.set_right(p);
                        gp.recolor(true);
                        k.recolor(false);
                        gp.set_right(k.left());
                        k.set_left(gp);
                    }
                    else
                    {
                        //right-right case, single rotate
                        p.recolor(false);
                        gp.recolor(true);
                        auto swap = p.left();
                        gp.set_right(swap);
                        p.set_left(gp);
                    }
                }
                return;
            }
        }
    }
}

void oops::memory::old_heap::_rb_delete_node_double_black(_rb_tree_free_node *nodes, unsigned int length)
{
    while (length > 2)
    {
        auto node = nodes[length - 1];
        auto parent = nodes[length - 2];
        auto grandparent = nodes[length - 3];
        auto node_left = parent.is_left(node);
        auto sibling = node_left ? parent.right() : parent.left();
        if (sibling.is_red())
        {
            sibling.recolor(false);
            if (grandparent.is_left(parent))
            {
                grandparent.set_left(sibling);
            }
            else
            {
                grandparent.set_right(sibling);
            }
            if (node_left)
            {
                auto niece = sibling.left();
                auto niece_d = niece.left();
                auto niece_s = niece.right();
                if (niece_s.is_red())
                {
                    parent.set_right(niece_d);
                    niece_s.recolor(false);
                    niece.set_right(niece_s);
                    niece.recolor(true);
                    sibling.set_left(niece);
                }
                else if (niece_d.is_red())
                {
                    parent.set_right(niece_d.left());
                    niece.set_left(niece_d.right());
                    niece_d.set_left(parent);
                    niece_d.set_right(niece);
                    sibling.set_left(niece_d);
                }
                else
                {
                    sibling.set_left(parent);
                    niece.recolor(true);
                    parent.set_right(niece);
                }
            }
            else
            {
                auto nephew = sibling.right();
                auto nephew_d = nephew.left();
                auto nephew_s = nephew.right();
                if (nephew_d.is_red())
                {
                    parent.set_left(nephew_s);
                    nephew_d.recolor(false);
                    nephew.set_left(nephew_d);
                    nephew.recolor(true);
                    sibling.set_right(nephew);
                }
                else if (nephew_s.is_red())
                {
                    sibling.set_right(nephew_s);
                    nephew.set_right(nephew_s.left());
                    parent.set_left(nephew_s.right());
                    nephew_s.set_left(nephew);
                    nephew_s.set_right(parent);
                }
                else
                {
                    sibling.set_right(parent);
                    nephew.recolor(true);
                    parent.set_left(nephew);
                }
            }
            return; //All rotations complete (somehow...)
        }
        else if (!parent.is_red() && !sibling.left().is_red() && !sibling.right().is_red())
        {
            sibling.recolor(true);
            if (node_left)
            {
                parent.set_right(sibling);
            }
            else
            {
                parent.set_left(sibling);
            }
            length--;
            //Intentional continue (explicit for readability)
            continue;
        }
        else
        {
            auto niece = sibling.left();
            auto nephew = sibling.right();
            if (node_left)
            {
                if (nephew.is_red())
                {
                    sibling.recolor(parent.is_red());
                    parent.recolor(false);
                    parent.set_right(niece);
                    sibling.set_left(parent);
                    grandparent.replace(parent, sibling);
                }
                else if (niece.is_red())
                {
                    niece.recolor(parent.is_red());
                    parent.recolor(false);
                    parent.set_right(niece.left());
                    sibling.set_left(niece.right());
                    niece.set_left(parent);
                    niece.set_right(sibling);
                    grandparent.replace(parent, niece);
                }
                else
                {
                    parent.recolor(false);
                    sibling.recolor(true);
                    parent.set_right(sibling);
                    grandparent.reset(parent);
                }
            }
            else
            {
                if (niece.is_red())
                {
                    sibling.recolor(parent.is_red());
                    parent.recolor(false);
                    parent.set_left(nephew);
                    sibling.set_right(parent);
                    grandparent.replace(parent, sibling);
                }
                else if (nephew.is_red())
                {
                    nephew.recolor(parent.is_red());
                    parent.recolor(false);
                    parent.set_left(nephew.right());
                    sibling.set_right(nephew.left());
                    nephew.set_left(sibling);
                    nephew.set_right(parent);
                    grandparent.replace(parent, nephew);
                }
                else
                {
                    parent.recolor(false);
                    sibling.recolor(true);
                    parent.set_left(sibling);
                    grandparent.reset(parent);
                }
            }
            return;
        }
    }
    auto node = nodes[1];
    auto parent = nodes[0];
    auto node_left = parent.is_left(node);
    auto sibling = node_left ? parent.right() : parent.left();
    if (sibling.is_red())
    {
        sibling.recolor(false);
        if (node_left)
        {
            auto niece = sibling.left();
            auto niece_d = niece.left();
            auto niece_s = niece.right();
            if (niece_s.is_red())
            {
                parent.set_right(niece_d);
                niece_s.recolor(false);
                niece.set_right(niece_s);
                niece.recolor(true);
                sibling.set_left(niece);
            }
            else if (niece_d.is_red())
            {
                parent.set_right(niece_d.left());
                niece.set_left(niece_d.right());
                niece_d.set_left(parent);
                niece_d.set_right(niece);
                sibling.set_left(niece_d);
            }
            else
            {
                sibling.set_left(parent);
                niece.recolor(true);
                parent.set_right(niece);
            }
        }
        else
        {
            auto nephew = sibling.right();
            auto nephew_d = nephew.left();
            auto nephew_s = nephew.right();
            if (nephew_d.is_red())
            {
                parent.set_left(nephew_s);
                nephew_d.recolor(false);
                nephew.set_left(nephew_d);
                nephew.recolor(true);
                sibling.set_right(nephew);
            }
            else if (nephew_s.is_red())
            {
                sibling.set_right(nephew_s);
                nephew.set_right(nephew_s.left());
                parent.set_left(nephew_s.right());
                nephew_s.set_left(nephew);
                nephew_s.set_right(parent);
            }
            else
            {
                sibling.set_right(parent);
                nephew.recolor(true);
                parent.set_left(nephew);
            }
        }
        return; //All rotations complete (somehow...)
    }
    else if (!parent.is_red() && !sibling.left().is_red() && !sibling.right().is_red())
    {
        sibling.recolor(true);
        parent.reset(sibling);
        return;
    }
    else
    {
        auto niece = sibling.left();
        auto nephew = sibling.right();
        if (node_left)
        {
            if (nephew.is_red())
            {
                sibling.recolor(parent.is_red());
                parent.recolor(false);
                parent.set_right(niece);
                sibling.set_left(parent);
            }
            else if (niece.is_red())
            {
                niece.recolor(parent.is_red());
                parent.recolor(false);
                parent.set_right(niece.left());
                sibling.set_left(niece.right());
                niece.set_left(parent);
                niece.set_right(sibling);
            }
            else
            {
                parent.recolor(false);
                sibling.recolor(true);
                parent.set_right(sibling);
            }
        }
        else
        {
            if (niece.is_red())
            {
                sibling.recolor(parent.is_red());
                parent.recolor(false);
                parent.set_left(nephew);
                sibling.set_right(parent);
            }
            else if (nephew.is_red())
            {
                nephew.recolor(parent.is_red());
                parent.recolor(false);
                parent.set_left(nephew.right());
                sibling.set_right(nephew.left());
                nephew.set_left(sibling);
                nephew.set_right(parent);
            }
            else
            {
                parent.recolor(false);
                sibling.recolor(true);
                parent.set_left(sibling);
            }
        }
        return;
    }
}

void oops::memory::old_heap::_rb_delete_node_single_child(oops::memory::old_heap::_rb_tree_free_node *nodes, unsigned int length)
{
    auto node = nodes[length - 1];
    auto node_parent = nodes[length - 2];
    auto node_left = node.left();
    auto node_right = node.right();
    if (node_left)
    {
        if (node_left.is_red())
        {
            node_left.recolor(false);
            node.set_left(node_left);
            //Right guaranteed nil, so node is black, so we recolor red leaf black to preserve nil black heights
            return;
        }
        else
        {
            //Double black fallthrough
        }
    }
    else if (node_right)
    {
        if (node_right.is_red())
        {
            node_right.recolor(false);
            node.set_right(node_right);
            //Left guaranteed nil, so node is black, so we recolor red leaf black to preserve nil black heights
            return;
        }
        else
        {
            //Double black fallthrough
        }
    }
    else
    {
        if (node.is_red())
        {
            node_parent.replace(node, node_left);
            //This is a red leaf, so we're done here
            return;
        }
        else
        {
            //Double black fallthrough
        }
    }
    _rb_delete_node_double_black(nodes, length);
}

void oops::memory::old_heap::_rb_delete_node(oops::memory::old_heap::_rb_tree_free_node *nodes, unsigned int length)
{
    auto node = nodes[length - 1];
    if (!node.left())
    {
        _rb_delete_node_single_child(nodes, length);
    }
    else
    {
        auto new_length = _rb_find_single_child_node(nodes, length);
        auto swap = nodes[new_length - 1];
        auto node_red = node.is_red();
        //Swap colors
        node.recolor(swap.is_red());
        swap.recolor(node_red);
        nodes[length - 2].replace(node, swap);
        nodes[new_length - 2].replace(node, swap);
        //Swap children
        auto swap_child = node.left();
        node.set_left(swap.left());
        swap.set_left(swap_child);
        swap_child = node.right();
        node.set_right(swap.right());
        swap.set_right(swap_child);
        //Swap parent pointers
        std::swap(nodes[length - 1], nodes[new_length - 1]);
        //Node to be deleted is now guaranteed to not have >1 child. Single child deletion may commence.
        _rb_delete_node_single_child(nodes, new_length);
    }
}

unsigned int oops::memory::old_heap::_rb_find_single_child_node(oops::memory::old_heap::_rb_tree_free_node *nodes, unsigned int length)
{
    auto parent = nodes[length - 1].right();
    while (parent)
    {
        nodes[length++] = parent;
        parent = parent.left();
    }
    return length;
}