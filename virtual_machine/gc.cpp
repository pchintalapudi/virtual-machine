#include "vm.h"

#include <stack>
using namespace oops::virtual_machine;

void virtual_machine::gc(bool force_old)
{
    if (force_old)
    {
        this->old_gc();
    }
    else if (!this->young_gc())
    {
        this->old_gc();
        this->young_gc();
    }
}

template <typename container>
std::optional<oops::objects::base_object> virtual_machine::gc_obj(objects::base_object obj, container &old_gc_references)
{
    if (obj && !this->heap.is_old_object(obj))
    {
        auto moved = this->heap.gc_move_young(obj);
        //TODO handle failure to move
        if (moved.second == memory::heap::location::TENURED)
        {
            old_gc_references.push(moved.first->unwrap());
        }
        return moved.first;
    }
    return {};
}

template <typename container>
bool virtual_machine::gc_base_object(objects::base_object bobj, container &old_gc_references)
{
    bool success = false;
    auto cls = bobj.get_clazz();
    if (cls.unwrap() == this->array_classes[static_cast<unsigned int>(objects::field::type::OBJECT)].unwrap())
    {
        //We have an old array of objects
        objects::array array(bobj.unwrap());
        for (std::uint64_t pointer = array.length(); pointer-- > 0;)
        {
            if (auto moved = this->gc_obj(array.read<objects::base_object>(pointer * sizeof(char *)), old_gc_references))
            {
                array.write(pointer * sizeof(char *), *moved);
                success = true;
            }
        }
    }
    else if (!std::binary_search(this->array_classes, this->array_classes + (sizeof(this->array_classes) / sizeof(this->array_classes[0]) - 1), cls))
    {
        //We have an old object
        objects::object robj(bobj.unwrap());
        for (std::uint64_t pointer = cls.virtual_handle_count(); pointer-- > 0;)
        {
            if (auto moved = this->gc_obj(robj.read<objects::base_object>(pointer * sizeof(char *)), old_gc_references))
            {
                robj.write(pointer * sizeof(char *), *moved);
                success = true;
            }
        }
    }
    return success;
}

bool virtual_machine::young_gc()
{
    std::stack<char *, std::vector<char *>> old_gc_references;
    for (objects::clazz cls : this->static_references)
    {
        for (std::uint64_t pointer = cls.static_handle_count(); pointer-- > 0;)
        {
            if (auto moved = this->gc_obj(cls.read<objects::base_object>(pointer * sizeof(char *)), old_gc_references))
            {
                cls.write(pointer * sizeof(char *), *moved);
            }
        }
    }
    for (objects::base_object bobj : this->heap.forward_references)
    {
        this->gc_base_object(bobj, old_gc_references);
    }
    std::optional<memory::frame> f = this->frame;
    do
    {
        auto handle_map = f->get_method().get_stack_handle_map();
        for (auto i = handle_map.size(); i-- > 0;)
        {
            if (auto moved = this->gc_obj(f->read<objects::base_object>(handle_map[i]), old_gc_references))
            {
                f->write(handle_map[i], *moved);
            }
        }
        f = f->previous();
    } while (f);
    bool taint;
    auto heap_it = this->heap.ybegin();
    do {
        taint = false;
        while (!old_gc_references.empty())
        {
            objects::base_object bobj(old_gc_references.top());
            old_gc_references.pop();
            taint |= this->gc_base_object(bobj, old_gc_references);
        }
        while (heap_it != this->heap.yend()) {
            taint |= this->gc_base_object(*heap_it, old_gc_references);
            ++heap_it;
        }
    } while (taint);
    return true;
}