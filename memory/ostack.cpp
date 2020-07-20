#include "ostack.h"

#include "../platform_specific/memory.h"

using namespace oops::memory;

oops::objects::method frame::get_method() const
{
    return objects::method(utils::pun_read<char *>(this->real));
}

std::uint16_t frame::return_address() const
{
    return utils::pun_read<std::uint16_t>(this->real + sizeof(char *));
}
std::uint16_t frame::return_offset() const
{
    return utils::pun_read<std::uint16_t>(this->real + sizeof(char *) + sizeof(std::uint16_t));
}

std::uint16_t frame::prev_size() const
{
    return utils::pun_read<std::uint16_t>(this->real + sizeof(char *) + sizeof(std::uint16_t) * 2);
}

char *frame::data_start() const
{
    return this->real + sizeof(char *) + sizeof(std::uint16_t *) * 4;
}

bool frame::prev_native() const
{
    return utils::pun_read<std::uint16_t>(this->real + sizeof(char *) + sizeof(std::uint16_t) * 3) & 1;
}

std::optional<frame> frame::previous() const {
    std::uint32_t prev_size = this->prev_size();
    if (prev_size) {
        frame prev;
        prev.real = this->real - prev_size * sizeof(std::uint32_t);
        return prev;
    } else {
        return {};
    }
}

template <typename result_type>
char *stack::load_and_pop(frame &frame, result_type result)
{
    if (frame.prev_native())
    {
        return nullptr;
    }
    char *next_frame_ptr = frame.real - (static_cast<std::uint32_t>(frame.prev_size()) * sizeof(std::uint32_t) + sizeof(char*) + sizeof(std::uint16_t) * 4);
    this->current_size = frame.prev_size();
    auto offset = frame.return_offset();
    std::uint32_t ret_addr = utils::pun_read<std::uint16_t>(next_frame_ptr + sizeof(char *));
    frame.real = next_frame_ptr;
    frame.write(offset, result);
    return objects::method(utils::pun_read<char *>(next_frame_ptr)).bytecode_begin() + ret_addr * sizeof(std::uint64_t);
}

#define load_fwd(type) template char *oops::memory::stack::load_and_pop<type>(frame &, type)
load_fwd(std::int32_t);
load_fwd(std::int64_t);
load_fwd(float);
load_fwd(double);
load_fwd(oops::objects::base_object);
#undef load_fwd

bool stack::init_frame(frame &frame, objects::method method, std::uint16_t return_offset, bool native_root, char *ip)
{
    typedef objects::field::type ftype;
    constexpr std::uint32_t types_per_instr = sizeof(std::uint64_t) * CHAR_BIT / objects::field::type_bits, offsets_per_instr = sizeof(char*) / sizeof(std::uint16_t);
    std::uint32_t size = method.stack_frame_size();
    size *= sizeof(std::uint32_t);
    size += sizeof(char *) + sizeof(std::uint16_t) * 4;
    if (this->cap - this->head < size)
        return false;
    std::memset(this->head, 0, size);
    auto dest = this->head + sizeof(char *) + sizeof(std::uint16_t *) * 4;
    auto arg_count = method.arg_count();
    std::uint64_t arg_pack;
    std::uint64_t arg_offsets;
    for (auto i = 0u; i < arg_count; i++)
    {
        if (i % types_per_instr == 0) {
            arg_pack = method.arg_offset_pack(i / types_per_instr);
        }
        if (i % offsets_per_instr == 0) {
            arg_offsets = utils::pun_read<std::uint64_t>(ip += sizeof(std::uint64_t));
        }
        auto type = static_cast<objects::field::type>(arg_pack & ((1 << (objects::field::type_bits - 1)) - 1));
        arg_pack >>= objects::field::type_bits;
        auto offset = static_cast<std::uint16_t>(arg_offsets & 0xffffull);
        arg_offsets >>= sizeof(std::uint16_t) * CHAR_BIT;
        switch(type) {
            default:
            return false;
            case ftype::INT:
            utils::pun_write(dest, frame.read<std::int32_t>(offset));
            dest += sizeof(std::int32_t);
            break;
            case ftype::FLOAT:
            utils::pun_write(dest, frame.read<float>(offset));
            dest += sizeof(float);
            break;
            case ftype::LONG:
            utils::pun_write(dest, frame.read<std::int64_t>(offset));
            dest += sizeof(std::int64_t);
            break;
            case ftype::DOUBLE:
            utils::pun_write(dest, frame.read<double>(offset));
            dest += sizeof(double);
            break;
            case ftype::OBJECT:
            utils::pun_write(dest, frame.read<objects::base_object>(offset).unwrap());
            dest += sizeof(char*);
            break;
        }
    }
    std::uint32_t instr_diff = ip + sizeof(std::uint64_t) - frame.get_method().bytecode_begin();
    utils::pun_write(this->head, method.unwrap());
    utils::pun_write(this->head + sizeof(char*), static_cast<std::uint16_t>(instr_diff / sizeof(std::uint64_t)));
    utils::pun_write(this->head + sizeof(char *) + sizeof(std::uint16_t), return_offset);
    utils::pun_write(this->head + sizeof(char *) + sizeof(std::uint16_t) * 2, this->current_size);
    utils::pun_write(this->head + sizeof(char *) + sizeof(std::uint16_t) * 3, static_cast<std::uint16_t>(native_root));
    this->current_size = method.stack_frame_size();
    frame.real = this->head;
    this->head += size;
    return true;
}