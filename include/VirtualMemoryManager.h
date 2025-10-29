#ifndef VIRTUAL_MEMORY_MANAGER_H
#define VIRTUAL_MEMORY_MANAGER_H

#include "Config.h"
#include "TLB.h"
#include "PageTable.h"
#include "PhysicalMemory.h"
#include <memory>
#include <iostream>

namespace vm {

class VirtualMemoryManager {
public:
    explicit VirtualMemoryManager(const Config& config);

    std::optional<PhysicalAddress> translate(VirtualAddress vaddr, bool write = false);
    uint8_t read_byte(VirtualAddress vaddr);
    void write_byte(VirtualAddress vaddr, uint8_t value);
    bool allocate_page(VirtualAddress vaddr);
    void free_page(VirtualAddress vaddr);
    void print_statistics(std::ostream& os = std::cout) const;
    void reset_statistics();

    TLB& get_tlb() { return *tlb_; }
    PageTable& get_page_table() { return *page_table_; }
    PhysicalMemory& get_physical_memory() { return *physical_memory_; }

    const Config& get_config() const { return config_; }

private:
    Config config_;
    std::unique_ptr<TLB> tlb_;
    std::unique_ptr<PageTable> page_table_;
    std::unique_ptr<PhysicalMemory> physical_memory_;

    size_t total_accesses_;
    size_t tlb_hits_;
    size_t page_table_hits_;
    size_t page_faults_;

    PageNumber extract_page_number(VirtualAddress vaddr) const;
    size_t extract_offset(VirtualAddress vaddr) const;
    bool handle_page_fault(PageNumber vpn);
};

} // namespace vm

#endif // VIRTUAL_MEMORY_MANAGER_H
