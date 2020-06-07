#ifndef VM_PRIMITIVE_OPS_H
#define VM_PRIMITIVE_OPS_H

#include <type_traits>

namespace oops
{
    namespace vm
    {

        namespace primitives
        {
            template <typename integral>
            struct vm_integer_check
            {
                static constexpr bool valid = std::is_signed<integral>::value and std::is_integral<integral>::value;
            };
            template <typename arg1_type, typename arg2_type>
            using integer_op = typename std::enable_if_t<vm_integer_check<arg1_type>::valid and vm_integer_check<arg2_type>::valid, std::common_type_t<arg1_type, arg2_type>>;
            template <typename arg1_type, typename arg2_type>
            using float_op = typename std::enable_if_t<(std::is_floating_point<arg1_type>::value or std::is_floating_point<arg2_type>::value) and std::is_signed<arg1_type>::value and std::is_signed<arg2_type>::value, std::common_type_t<arg1_type, arg2_type>>;
            template <typename arg1_type, typename arg2_type>
            using primitive_op = typename std::enable_if_t<std::is_signed<arg1_type>::value and std::is_signed<arg2_type>::value, bool>;

            template <typename result_type, typename arg1_type, typename arg2_type, typename op_type>
            inline result_type force_unsigned_math(arg1_type arg1, arg2_type arg2, op_type op)
            {
                return static_cast<result_type>(op(static_cast<std::make_unsigned_t<arg1_type>>(arg1), static_cast<std::make_unsigned_t<arg2_type>>(arg2)));
            }
            template <typename arg1_type, typename arg2_type>
            inline typename primitives::integer_op<arg1_type, arg2_type> add(arg1_type arg1, arg2_type arg2)
            {
                return force_unsigned_math<std::common_type_t<arg1_type, arg2_type>, arg1_type, arg2_type>(arg1, arg2, [](auto a, auto b) { return a + b; });
            }
            template <typename arg1_type, typename arg2_type>
            inline typename primitives::integer_op<arg1_type, arg2_type> sub(arg1_type arg1, arg2_type arg2)
            {
                return force_unsigned_math<std::common_type_t<arg1_type, arg2_type>, arg1_type, arg2_type>(arg1, arg2, [](auto a, auto b) { return a - b; });
            }
            template <typename arg1_type, typename arg2_type>
            inline typename primitives::integer_op<arg1_type, arg2_type> mul(arg1_type arg1, arg2_type arg2)
            {
                return force_unsigned_math<std::common_type_t<arg1_type, arg2_type>, arg1_type, arg2_type>(arg1, arg2, [](auto a, auto b) { return a * b; });
            }
            template <typename arg1_type, typename arg2_type>
            inline typename primitives::integer_op<arg1_type, arg2_type> divu(arg1_type arg1, arg2_type arg2)
            {
                return force_unsigned_math<std::common_type_t<arg1_type, arg2_type>, arg1_type, arg2_type>(arg1, arg2, [](auto a, auto b) { return a / b; });
            }
            template <typename arg1_type, typename arg2_type>
            inline typename primitives::integer_op<arg1_type, arg2_type> div(arg1_type arg1, arg2_type arg2)
            {
                signed char a1n = static_cast<std::make_unsigned_t<arg1_type>>(arg1) >> (sizeof(arg1_type) * 8 - 1),
                            a2n = static_cast<std::make_unsigned_t<arg2_type>>(arg2) >> (sizeof(arg2_type) * 8 - 1),
                            rn = a1n ^ a2n;
                return (divu((arg1 ^ -a1n) + a1n, (arg2 ^ -a2n) + a2n) ^ -rn) + rn;
            }

            template <typename arg1_type, typename arg2_type>
            inline typename primitives::float_op<arg1_type, arg2_type> add(arg1_type arg1, arg2_type arg2)
            {
                return arg1 + arg2;
            }
            template <typename arg1_type, typename arg2_type>
            inline typename primitives::float_op<arg1_type, arg2_type> sub(arg1_type arg1, arg2_type arg2)
            {
                return arg1 - arg2;
            }
            template <typename arg1_type, typename arg2_type>
            inline typename primitives::float_op<arg1_type, arg2_type> mul(arg1_type arg1, arg2_type arg2)
            {
                return arg1 * arg2;
            }
            template <typename arg1_type, typename arg2_type>
            inline typename primitives::float_op<arg1_type, arg2_type> div(arg1_type arg1, arg2_type arg2)
            {
                return arg1 / arg2;
            }

            template <typename arg1_type, typename arg2_type>
            inline typename primitives::integer_op<arg1_type, arg2_type> bit_and(arg1_type arg1, arg2_type arg2)
            {
                return force_unsigned_math<std::common_type_t<arg1_type, arg2_type>, arg1_type, arg2_type>(arg1, arg2, [](auto a, auto b) { return a & b; });
            }
            template <typename arg1_type, typename arg2_type>
            inline typename primitives::integer_op<arg1_type, arg2_type> bit_or(arg1_type arg1, arg2_type arg2)
            {
                return force_unsigned_math<std::common_type_t<arg1_type, arg2_type>, arg1_type, arg2_type>(arg1, arg2, [](auto a, auto b) { return a | b; });
            }
            template <typename arg1_type, typename arg2_type>
            inline typename primitives::integer_op<arg1_type, arg2_type> bit_xor(arg1_type arg1, arg2_type arg2)
            {
                return force_unsigned_math<std::common_type_t<arg1_type, arg2_type>, arg1_type, arg2_type>(arg1, arg2, [](auto a, auto b) { return a ^ b; });
            }
            template <typename arg1_type, typename arg2_type>
            inline typename primitives::integer_op<arg1_type, arg2_type> bit_sll(arg1_type arg1, arg2_type arg2)
            {
                return force_unsigned_math<std::common_type_t<arg1_type, arg2_type>, arg1_type, arg2_type>(arg1, arg2, [](auto a, auto b) { return a << (b & (sizeof(a) * 8 - 1)); });
            }
            template <typename arg1_type, typename arg2_type>
            inline typename primitives::integer_op<arg1_type, arg2_type> bit_srl(arg1_type arg1, arg2_type arg2)
            {
                return force_unsigned_math<std::common_type_t<arg1_type, arg2_type>, arg1_type, arg2_type>(arg1, arg2, [](auto a, auto b) { return a >> (b & (sizeof(a) * 8 - 1)); });
            }
            template <typename arg1_type, typename arg2_type>
            inline typename std::enable_if_t<vm_integer_check<arg1_type>::valid and vm_integer_check<arg2_type>::valid, arg1_type> bit_sra(arg1_type arg1, arg2_type arg2)
            {
                return arg1 >> (static_cast<std::make_unsigned_t<arg2_type>>(arg2) & (sizeof(arg1_type) * 8 - 1));
            }

            template <typename arg1_type, typename arg2_type>
            inline typename primitives::primitive_op<arg1_type, arg2_type> eq(arg1_type arg1, arg2_type arg2)
            {
                return arg1 == arg2;
            }
            template <typename arg1_type, typename arg2_type>
            inline typename primitives::primitive_op<arg1_type, arg2_type> ne(arg1_type arg1, arg2_type arg2)
            {
                return arg1 != arg2;
            }
            template <typename arg1_type, typename arg2_type>
            inline typename primitives::primitive_op<arg1_type, arg2_type> le(arg1_type arg1, arg2_type arg2)
            {
                return arg1 <= arg2;
            }
            template <typename arg1_type, typename arg2_type>
            inline typename primitives::primitive_op<arg1_type, arg2_type> ge(arg1_type arg1, arg2_type arg2)
            {
                return arg1 >= arg2;
            }
            template <typename arg1_type, typename arg2_type>
            inline typename primitives::primitive_op<arg1_type, arg2_type> lt(arg1_type arg1, arg2_type arg2)
            {
                return arg1 < arg2;
            }
            template <typename arg1_type, typename arg2_type>
            inline typename primitives::primitive_op<arg1_type, arg2_type> gt(arg1_type arg1, arg2_type arg2)
            {
                return arg1 > arg2;
            }

            template<typename arg_type>
            inline typename std::enable_if_t<std::is_signed<arg_type>::value, arg_type> neg(arg_type arg) {
                return -arg;
            }

            template<typename from_type, typename to_type>
            inline typename std::enable_if_t<std::is_signed<from_type>::value and std::is_signed<to_type>::value, to_type> cast(from_type from) {
                return static_cast<to_type>(from);
            }
        };
    } // namespace vm
} // namespace ops
#endif