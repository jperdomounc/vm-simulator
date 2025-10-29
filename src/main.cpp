#include "VirtualMemoryManager.h"
#include <iostream>
#include <random>
#include <iomanip>

using namespace vm;

void demo_basic_operations(VirtualMemoryManager& vmm) {
    std::cout << "\n=== Demo 1: Basic Memory Operations ===\n";

    std::cout << "Writing values to virtual addresses...\n";
    for (VirtualAddress addr = 0; addr < 10000; addr += 1000) {
        vmm.write_byte(addr, static_cast<uint8_t>(addr % 256));
        std::cout << "  Wrote " << static_cast<int>(addr % 256)
                  << " to virtual address " << addr << "\n";
    }

    std::cout << "\nReading values back from virtual addresses...\n";
    for (VirtualAddress addr = 0; addr < 10000; addr += 1000) {
        uint8_t value = vmm.read_byte(addr);
        std::cout << "  Read " << static_cast<int>(value)
                  << " from virtual address " << addr << "\n";
    }
}

void demo_tlb_behavior(VirtualMemoryManager& vmm) {
    std::cout << "\n=== Demo 2: TLB Behavior ===\n";

    const size_t page_size = vmm.get_config().page_size;
    const size_t tlb_size = vmm.get_config().tlb_size;

    vmm.reset_statistics();

    std::cout << "Accessing pages sequentially (should have high TLB miss rate initially)...\n";
    for (size_t i = 0; i < tlb_size + 10; ++i) {
        VirtualAddress addr = i * page_size;
        vmm.write_byte(addr, static_cast<uint8_t>(i));
    }

    std::cout << "TLB stats after first pass:\n";
    std::cout << "  TLB hit rate: " << std::fixed << std::setprecision(2)
              << vmm.get_tlb().get_hit_rate() * 100.0 << "%\n";

    std::cout << "\nAccessing the same pages again (should have high TLB hit rate)...\n";
    for (size_t i = 0; i < tlb_size - 5; ++i) {
        VirtualAddress addr = i * page_size;
        uint8_t value = vmm.read_byte(addr);
        (void)value;
    }

    std::cout << "TLB stats after second pass:\n";
    std::cout << "  TLB hit rate: " << vmm.get_tlb().get_hit_rate() * 100.0 << "%\n";
}

void demo_demand_paging(VirtualMemoryManager& vmm) {
    std::cout << "\n=== Demo 3: Demand Paging ===\n";

    const size_t page_size = vmm.get_config().page_size;
    size_t initial_page_faults = vmm.get_physical_memory().get_page_faults();

    vmm.reset_statistics();

    std::cout << "Accessing new pages (will cause page faults)...\n";
    for (size_t i = 0; i < 20; ++i) {
        VirtualAddress addr = (100000 + i) * page_size;
        vmm.write_byte(addr, static_cast<uint8_t>(i * 7));
    }

    size_t page_faults = vmm.get_physical_memory().get_page_faults();
    std::cout << "Page faults during allocation: " << page_faults << "\n";

    std::cout << "\nAccessing the same pages again (no new page faults)...\n";
    vmm.reset_statistics();
    for (size_t i = 0; i < 20; ++i) {
        VirtualAddress addr = (100000 + i) * page_size;
        uint8_t value = vmm.read_byte(addr);
        if (value != static_cast<uint8_t>(i * 7)) {
            std::cout << "  ERROR: Incorrect value read!\n";
        }
    }

    size_t new_page_faults = vmm.get_physical_memory().get_page_faults();
    std::cout << "Page faults during re-access: " << new_page_faults << "\n";
}

void demo_page_table_hierarchy(VirtualMemoryManager& vmm) {
    std::cout << "\n=== Demo 4: Multi-level Page Table ===\n";

    const Config& config = vmm.get_config();
    std::cout << "Page table configuration:\n";
    std::cout << "  Levels: " << config.page_table_levels << "\n";
    std::cout << "  Bits per level: " << config.bits_per_level << "\n";
    std::cout << "  Entries per level: " << (1ULL << config.bits_per_level) << "\n";

    std::cout << "\nAllocating sparse pages across virtual address space...\n";
    size_t num_sparse_pages = 10;
    for (size_t i = 0; i < num_sparse_pages; ++i) {
        VirtualAddress addr = i * 1000000;
        vmm.write_byte(addr, static_cast<uint8_t>(i));
        std::cout << "  Allocated page at virtual address " << addr << "\n";
    }

    std::cout << "Page table entries created: " << vmm.get_page_table().get_num_entries() << "\n";
}

void demo_random_access(VirtualMemoryManager& vmm) {
    std::cout << "\n=== Demo 5: Random Access Pattern ===\n";

    const size_t page_size = vmm.get_config().page_size;
    const size_t num_accesses = 1000;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<VirtualAddress> addr_dist(0, page_size * 100);
    std::uniform_int_distribution<uint8_t> value_dist(0, 255);

    vmm.reset_statistics();

    std::cout << "Performing " << num_accesses << " random memory accesses...\n";
    for (size_t i = 0; i < num_accesses; ++i) {
        VirtualAddress addr = addr_dist(gen);
        uint8_t value = value_dist(gen);

        if (i % 2 == 0) {
            vmm.write_byte(addr, value);
        } else {
            vmm.read_byte(addr);
        }
    }

    std::cout << "Random access completed.\n";
}

void demo_access_patterns(VirtualMemoryManager& vmm) {
    std::cout << "\n=== Demo 6: Access Pattern Comparison ===\n";

    const size_t page_size = vmm.get_config().page_size;
    const size_t num_pages = 50;

    std::cout << "\nSequential access pattern:\n";
    vmm.reset_statistics();
    for (size_t i = 0; i < num_pages; ++i) {
        VirtualAddress addr = i * page_size;
        vmm.write_byte(addr, static_cast<uint8_t>(i));
    }
    // Read back
    for (size_t i = 0; i < num_pages; ++i) {
        VirtualAddress addr = i * page_size;
        vmm.read_byte(addr);
    }
    std::cout << "  TLB hit rate: " << std::fixed << std::setprecision(2)
              << vmm.get_tlb().get_hit_rate() * 100.0 << "%\n";

    std::cout << "\nRandom access pattern:\n";
    vmm.get_tlb().clear();
    vmm.reset_statistics();

    std::random_device rd;
    std::mt19937 gen(42);
    std::vector<size_t> indices;
    for (size_t i = 0; i < num_pages; ++i) {
        indices.push_back(i);
    }
    std::shuffle(indices.begin(), indices.end(), gen);

    for (size_t idx : indices) {
        VirtualAddress addr = idx * page_size;
        vmm.write_byte(addr, static_cast<uint8_t>(idx));
    }
    std::shuffle(indices.begin(), indices.end(), gen);
    for (size_t idx : indices) {
        VirtualAddress addr = idx * page_size;
        vmm.read_byte(addr);
    }
    std::cout << "  TLB hit rate: " << vmm.get_tlb().get_hit_rate() * 100.0 << "%\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "   Virtual Memory Manager Simulator\n";
    std::cout << "========================================\n";

    Config config = Config::default_config();
    VirtualMemoryManager vmm(config);

    std::cout << "\nInitialized Virtual Memory Manager\n";
    std::cout << "Configuration:\n";
    std::cout << "  Page size: " << config.page_size << " bytes\n";
    std::cout << "  Physical memory: " << config.physical_memory_size / 1024 << " KB\n";
    std::cout << "  Number of frames: " << config.num_frames << "\n";
    std::cout << "  TLB size: " << config.tlb_size << " entries\n";
    std::cout << "  Page table levels: " << config.page_table_levels << "\n";

    try {
        demo_basic_operations(vmm);
        demo_tlb_behavior(vmm);
        demo_demand_paging(vmm);
        demo_page_table_hierarchy(vmm);
        demo_random_access(vmm);
        demo_access_patterns(vmm);

        vmm.print_statistics();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "\n=== Simulation Complete ===\n";
    std::cout << "\nKey Features Demonstrated:\n";
    std::cout << "  - Multi-level page tables\n";
    std::cout << "  - TLB with LRU replacement\n";
    std::cout << "  - Demand paging\n";
    std::cout << "  - Configurable page sizes and memory hierarchies\n";
    std::cout << "  - Various memory access patterns\n";

    return 0;
}
