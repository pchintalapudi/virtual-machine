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
std::optional<oops::objects::base_object> virtual_machine::young_gc_obj(objects::base_object obj, container &old_gc_references)
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
bool virtual_machine::young_gc_base_object(objects::base_object bobj, container &old_gc_references)
{
    bool success = false;
    auto cls = bobj.get_clazz();
    if (cls.unwrap() == this->array_classes[static_cast<unsigned int>(objects::field::type::OBJECT)].unwrap())
    {
        //We have an old array of objects
        objects::array array(bobj.unwrap());
        for (std::uint64_t pointer = array.length(); pointer-- > 0;)
        {
            if (auto moved = this->young_gc_obj(array.read<objects::base_object>(pointer * sizeof(char *)), old_gc_references))
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
            if (auto moved = this->young_gc_obj(robj.read<objects::base_object>(pointer * sizeof(char *)), old_gc_references))
            {
                robj.write(pointer * sizeof(char *), *moved);
                success = true;
            }
        }
    }
    return success;
}

//TODO gc prologue and epilogue (prep young heap, reset young heap boundaries, and commit/decommit memory)
bool virtual_machine::young_gc()
{
    std::stack<char *, std::vector<char *>> old_gc_references;
    for (objects::clazz cls : this->static_references)
    {
        for (std::uint64_t pointer = cls.static_handle_count(); pointer-- > 0;)
        {
            if (auto moved = this->young_gc_obj(cls.read<objects::base_object>(pointer * sizeof(char *)), old_gc_references))
            {
                cls.write(pointer * sizeof(char *), *moved);
            }
        }
    }
    for (auto it = this->heap.forward_references.begin(); it != this->heap.forward_references.end();)
    {
        if (this->young_gc_base_object(*it, old_gc_references))
        {
            ++it;
        }
        else
        {
            it = this->heap.forward_references.erase(it);
        }
    }
    std::optional<memory::frame> f = this->frame;
    do
    {
        auto handle_map = f->get_method().get_stack_handle_map();
        for (auto i = handle_map.size(); i-- > 0;)
        {
            if (auto moved = this->young_gc_obj(f->read<objects::base_object>(handle_map[i]), old_gc_references))
            {
                f->write(handle_map[i], *moved);
            }
        }
        f = f->previous();
    } while (f);
    bool taint;
    auto heap_it = this->heap.ybegin();
    do
    {
        taint = false;
        while (!old_gc_references.empty())
        {
            this->heap.back_references.erase(old_gc_references.top());
            objects::base_object bobj(old_gc_references.top());
            old_gc_references.pop();
            if (this->young_gc_base_object(bobj, old_gc_references))
            {
                taint = true;
                this->heap.forward_references.insert(bobj.unwrap());
            }
        }
        while (heap_it != this->heap.yend())
        {
            taint |= this->young_gc_base_object(*heap_it, old_gc_references);
            ++heap_it;
        }
    } while (taint);
    decltype(this->heap.back_references) new_refs;
    for (objects::base_object obj : this->heap.back_references)
    {
        if (auto forwarded = this->heap.moved(obj))
        {
            new_refs.insert(forwarded->unwrap());
        }
    }
    std::swap(new_refs, this->heap.back_references);
    std::vector<char *> unfinalized;
    for (objects::base_object obj : this->heap.finalizable)
    {
        if (auto forwarded = this->heap.moved(obj))
        {
            unfinalized.push_back(forwarded->unwrap());
        }
        else
        {
            if (obj.get_clazz() == this->array_classes[static_cast<unsigned>(objects::field::type::OBJECT)])
            {
                objects::array array(obj.unwrap());
                for (std::uint64_t i = array.length(); i-- > 0;)
                {
                    auto object = array.read<objects::base_object>(i * sizeof(char *));
                    if (auto moved = this->heap.moved(object))
                    {
                        array.write(i * sizeof(char *), *moved);
                    }
                }
                //TODO call native finalizer
            }
            else if (!std::binary_search(this->array_classes, this->array_classes + static_cast<unsigned>(objects::field::type::OBJECT), obj.get_clazz()))
            {
                objects::object object(obj.unwrap());
                for (std::uint64_t i = object.get_clazz().virtual_handle_count(); i-- > 0;)
                {
                    auto bobj = object.read<objects::base_object>(i * sizeof(char *));
                    if (auto moved = this->heap.moved(bobj))
                    {
                        object.write(i * sizeof(char *), *moved);
                    }
                }
                //TODO call native finalizer
            }
        }
    }
    std::swap(unfinalized, this->heap.finalizable);
    return true;
}

void virtual_machine::mark(objects::base_object obj)
{
    if (this->heap.is_old_object(obj) && !obj.marked())
    {
        obj.mark();
        if (obj.get_clazz() == this->array_classes[static_cast<unsigned>(objects::field::type::OBJECT)])
        {
            //Object array
            objects::array array(obj.unwrap());
            for (std::uint64_t i = 0; i < array.length(); i++)
            {
                if (auto bobj = array.read<objects::base_object>(i * sizeof(char *)))
                {
                    this->mark(bobj);
                }
            }
        }
        else if (!std::binary_search(this->array_classes, this->array_classes + static_cast<unsigned>(objects::field::type::OBJECT), obj.get_clazz()))
        {
            //Basic object
            objects::object bobj(obj.unwrap());
            std::uint64_t handle_count = bobj.get_clazz().virtual_handle_count();
            for (std::uint64_t i = 0; i < handle_count; i++)
            {
                if (auto pointer = bobj.read<objects::base_object>(i * sizeof(char *)))
                {
                    this->mark(pointer);
                }
            }
        }
    }
}

void virtual_machine::old_gc()
{
    for (objects::clazz cls : this->static_references)
    {
        for (std::uint64_t pointer = cls.static_handle_count(); pointer-- > 0;)
        {
            if (auto obj = cls.read<objects::base_object>(pointer * sizeof(char *)))
                this->mark(obj);
        }
    }
    std::optional<memory::frame> f = this->frame;
    do
    {
        auto handle_map = f->get_method().get_stack_handle_map();
        for (auto i = handle_map.size(); i-- > 0;)
        {
            if (auto obj = f->read<objects::base_object>(handle_map[i]))
                this->mark(obj);
        }
        f = f->previous();
    } while (f);
    //Finalization
    for (objects::base_object obj : this->heap.finalizable)
    {
        if (this->heap.is_old_object(obj) && (obj.marked()))
        {
            if (obj.get_clazz() == this->array_classes[static_cast<unsigned>(objects::field::type::OBJECT)])
            {
                objects::array array(obj.unwrap());
                for (std::uint64_t i = array.length(); i-- > 0;)
                {
                    auto object = array.read<objects::base_object>(i * sizeof(char *));
                    if (auto moved = this->heap.moved(object))
                    {
                        array.write(i * sizeof(char *), *moved);
                    }
                }
                //TODO call native finalizer
            }
            else if (!std::binary_search(this->array_classes, this->array_classes + static_cast<unsigned>(objects::field::type::OBJECT), obj.get_clazz()))
            {
                objects::object object(obj.unwrap());
                for (std::uint64_t i = object.get_clazz().virtual_handle_count(); i-- > 0;)
                {
                    auto bobj = object.read<objects::base_object>(i * sizeof(char *));
                    if (auto moved = this->heap.moved(bobj))
                    {
                        object.write(i * sizeof(char *), *moved);
                    }
                }
                //TODO call native finalizer
            }
        }
    }
    this->heap.sweep();
}