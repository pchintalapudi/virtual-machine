#ifndef VIRTUAL_MACHINE_PRIMITIVES
#define VIRTUAL_MACHINE_PRIMITIVES

#include <type_traits>
#include "../objects/objects.h"

namespace oops
{
    namespace virtual_machine
    {
        namespace primitives
        {

#define primitive_op(name, op)              \
    template <typename p1_t, typename p2_t> \
    std::enable_if_t<(std::is_same_v<std::int32_t, p1_t> or std::is_same_v<std::int64_t, p1_t>) and (std::is_same_v<std::int32_t, p2_t> or std::is_same_v<std::int64_t, p2_t>), std::common_type_t<p1_t, p2_t>> name(p1_t p1, p2_t p2) { using ct = std::common_type_t<p1_t, p2_t>; return static_cast<std::make_unsigned_t<ct>>(static_cast<ct>(p1)) op static_cast<std::make_unsigned_t<ct>>(static_cast<ct>(p2)); } \
    template <typename p1_t, typename p2_t> \
    std::enable_if_t<std::is_floating_point_v<p1_t> or std::is_floating_point_v<p2_t>, std::common_type_t<p1_t, p2_t>> name(p1_t p1, p2_t p2) {return p1 op p2;}

            primitive_op(add, +);
            primitive_op(sub, -);
            primitive_op(mul, *);
            primitive_op(divu, /);

            primitive_op(eq, ==);
            primitive_op(gt, >);
            primitive_op(lt, <);

#undef primitive_op

            template<typename p1_t, typename p2_t>
            std::enable_if_t<std::is_signed_v<p1_t>, p1_t> neg(p1_t p1, p2_t) {
                return -p1;
            }

            template <typename p1_t, typename p2_t>
            std::enable_if_t<std::is_signed_v<p1_t> and std::is_signed_v<p2_t>, std::common_type_t<p1_t, p2_t>> div(p1_t p1, p2_t p2)
            {
                return p1 / p2;
            }

            template<typename from_t, typename to_t>
            std::enable_if_t<std::is_signed_v<from_t> and std::is_signed_v<to_t>, to_t> cast(from_t p1) {
                return static_cast<to_t>(p1);
            }

#define integral_op(name, op)               \
    template <typename p1_t, typename p2_t> \
    std::enable_if_t<std::is_integral_v<p1_t> and std::is_integral_v<p2_t> and std::is_signed_v<p1_t> and std::is_signed_v<p2_t>, std::common_type_t<p1_t, p2_t>> name(p1_t p1, p2_t p2) { return static_cast<std::make_unsigned_t<p1_t>>(p1) op static_cast<std::make_unsigned_t<p2_t>>(p2); }

            integral_op(band, &);
            integral_op(bor, |);
            integral_op(bxor, ^);
            integral_op(bsll, <<);
            integral_op(bsrl, >>);

#undef integral_op

            constexpr bool sra_supported = (-1 >> 1) == -1;

            template <typename p1_t, typename p2_t>
            std::enable_if_t<std::is_integral_v<p1_t> and std::is_integral_v<p2_t>, std::common_type_t<p1_t, p2_t>> bsra(p1_t p1, p2_t p2)
            {
                if (sra_supported)
                    return p1 >> p2;
                else
                {
                    return p1 < 0 ? ~(~p1 >> p2) : p1 >> p2;
                }
            }

            bool eq(objects::base_object p1, objects::base_object p2)
            {
                return p1.unwrap() == p2.unwrap();
            }
        } // namespace primitives
    }     // namespace virtual_machine
} // namespace oops

#endif /* VIRTUAL_MACHINE_PRIMITIVES */
