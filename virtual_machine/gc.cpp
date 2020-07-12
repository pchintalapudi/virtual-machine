#include "vm.h"
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

bool virtual_machine::young_gc()
{
    std::vector<char *> old_gc_references;
    for (objects::clazz cls : this->static_references)
    {
        for (std::uint64_t pointer = cls.static_handle_count(); pointer-- > 0;)
        {
            if (auto obj = cls.read<objects::base_object>(pointer * sizeof(char *)); obj && !this->heap.is_old_object(obj))
            {
                auto moved = this->heap.gc_move_young(obj);
                //TODO handle failure to move
                if (moved.second == memory::heap::location::TENURED)
                {
                    old_gc_references.push_back(moved.first->unwrap());
                }
            }
        }
    }
    for (objects::base_object bobj : this->heap.gc_forward_references())
    {
        if (bobj.get_clazz().unwrap() == this->array_classes[static_cast<unsigned int>(objects::field::type::OBJECT)].unwrap())
        {
            //We have an old array of objects
            objects::array array(bobj.unwrap());
            for (std::uint64_t pointer = array.length(); pointer-- > 0;)
            {
                if (auto obj = array.read<objects::base_object>(pointer * sizeof(char *)); obj && !this->heap.is_old_object(obj))
                {
                    auto moved = this->heap.gc_move_young(obj);
                    //TODO handle failure to move
                    if (moved.second == memory::heap::location::TENURED)
                    {
                        old_gc_references.push_back(moved.first->unwrap());
                    }
                }
            }
        }
        else
        {
            //We have an old object
            objects::object robj(bobj.unwrap());
            for (std::uint64_t pointer = bobj.get_clazz().virtual_handle_count(); pointer-- > 0;)
            {
                if (auto obj = robj.read<objects::base_object>(pointer * sizeof(char *)); obj && !this->heap.is_old_object(obj))
                {
                    auto moved = this->heap.gc_move_young(obj);
                    //TODO handle failure to move
                    if (moved.second == memory::heap::location::TENURED)
                    {
                        old_gc_references.push_back(moved.first->unwrap());
                    }
                }
            }
        }
    }
    //TODO scan stack
    //TODO scan copied objects
}