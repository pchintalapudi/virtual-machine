#include "datatypes.h"

using namespace oops::classes;

static constexpr unsigned sizes[] = {sizeof(std::int8_t),  sizeof(std::int16_t),
                                     sizeof(std::int32_t), sizeof(float),
                                     sizeof(std::int64_t), sizeof(double),
                                     sizeof(void *)};

std::size_t oops::classes::datatype_size(datatype dt) {
  return sizes[static_cast<int>(dt)];
}
