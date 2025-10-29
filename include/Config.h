#ifndef CONFIG_H
#define CONFIG_H

#include <cstddef>
#include <cstdint>

namespace vm {

struct Config {
    size_t page_size;
    size_t offset_bits;
    size_t virtual_address_bits;
    size_t physical_memory_size;
    size_t num_frames;
    size_t page_table_levels;
    size_t bits_per_level;
    size_t tlb_size;

    static Config default_config() {
        Config config;
        config.page_size = 4096;
        config.offset_bits = 12;
        config.virtual_address_bits = 32;
        config.physical_memory_size = 64 * 1024 * 1024;
        config.num_frames = config.physical_memory_size / config.page_size;
        config.page_table_levels = 2;
        config.bits_per_level = 10;
        config.tlb_size = 64;
        return config;
    }

    static Config small_config() {
        Config config;
        config.page_size = 256;
        config.offset_bits = 8;
        config.virtual_address_bits = 16;
        config.physical_memory_size = 16 * 1024;
        config.num_frames = config.physical_memory_size / config.page_size;
        config.page_table_levels = 2;
        config.bits_per_level = 4;
        config.tlb_size = 8;
        return config;
    }
};

using VirtualAddress = uint64_t;
using PhysicalAddress = uint64_t;
using PageNumber = uint64_t;
using FrameNumber = uint64_t;

} // namespace vm

#endif // CONFIG_H
