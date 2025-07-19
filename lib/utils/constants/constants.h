#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstddef>
#include <cstdint>

namespace tektask::utils::constants
{
#if defined(__cpp_lib_hardware_interference_size)
#include <new>
    constexpr std::size_t CACHE_SIZE{std::hardware_destructive_interference_size};

#elif defined(__APPLE__) && defined(__aarch64__)
    // MacOs with ARM processors
    constexpr std::size_t CACHE_SIZE{128};

#else
    // default cache size
    constexpr std::size_t CACHE_SIZE{64};

#endif

    static constexpr int64_t INVALID_TRIPLET_ID{-1};
}

#endif //CONSTANTS_H
