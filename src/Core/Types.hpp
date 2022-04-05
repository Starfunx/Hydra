#pragma once

// std
#include <cstdint>
#include <cstdlib>

namespace hyd
{
    using i8 = int_least8_t;
    using i16 = int_least16_t;
    using i32 = int_least32_t;
    using i64 = int_least64_t;
    using ix = int_fast32_t;

    using u8 = uint_least8_t;
    using u16 = uint_least16_t;
    using u32 = uint_least32_t;
    using u64 = uint_least64_t;
    using ux = uint_fast32_t;
    using usize = std::size_t;

    using f32 = float;
    using f64 = double;
} // namespace hyd
