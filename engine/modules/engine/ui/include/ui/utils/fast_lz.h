#pragma once
#include <cstdint>
namespace api {
    ////////////////////////////////////////////////////////////////////////////////////////////////////
/// FastLZ - lightning-fast lossless compression library
/// https://ariya.github.io/FastLZ/
////////////////////////////////////////////////////////////////////////////////////////////////////
    struct FastLZ
    {
        /// Returns the size needed to decompress the given block of data
        static uint32_t DecompressBufferSize(const void* buffer);

        /// Decompress a block of compressed data
        static void Decompress(const void* buffer, uint32_t length, void* output);
    };
}