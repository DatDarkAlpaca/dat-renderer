#pragma once
#include <numeric>
#include <limits>

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

constexpr inline u32 invalid_handle = std::numeric_limits<u32>::max();