#include "VirtualMemoryManager.h"
#include <iomanip>

namespace vm {

VirtualMemoryManager::VirtualMemoryManager(const Config& config)
    : config_(config),
      tlb_(std::make_unique<TLB>(config.tlb_size)),
      page_table_(std::make_unique<PageTable>(config)),
      physical_memory_(std::make_unique<PhysicalMemory>(config)),
      total_accesses_(0),
      tlb_hits_(0),
      page_table_hits_(0),
      page_faults_(0) {}

std::optional<PhysicalAddress> VirtualMemoryManager::translate(VirtualAddress vaddr, bool write) {
    total_accesses_++;

    PageNumber vpn = extract_page_number(vaddr);
    size_t offset = extract_offset(vaddr);

    auto tlb_result = tlb_->lookup(vpn);
    if (tlb_result.has_value()) {
        tlb_hits_++;
        FrameNumber pfn = tlb_result.value();

        if (write) {
            page_table_->set_dirty(vpn, true);
        }
        page_table_->set_referenced(vpn, true);

        PhysicalAddress paddr = (pfn * config_.page_size) + offset;
        return paddr;
    }

    auto pt_result = page_table_->translate(vpn);
    if (pt_result.has_value()) {
        page_table_hits_++;
        FrameNumber pfn = pt_result.value();

        tlb_->insert(vpn, pfn);

        if (write) {
            page_table_->set_dirty(vpn, true);
        }

        PhysicalAddress paddr = (pfn * config_.page_size) + offset;
        return paddr;
    }

    page_faults_++;
    if (!handle_page_fault(vpn)) {
        return std::nullopt;
    }

    pt_result = page_table_->translate(vpn);
    if (pt_result.has_value()) {
        FrameNumber pfn = pt_result.value();
        tlb_->insert(vpn, pfn);

        if (write) {
            page_table_->set_dirty(vpn, true);
        }

        PhysicalAddress paddr = (pfn * config_.page_size) + offset;
        return paddr;
    }

    return std::nullopt;
}

uint8_t VirtualMemoryManager::read_byte(VirtualAddress vaddr) {
    auto paddr = translate(vaddr, false);
    if (!paddr.has_value()) {
        throw std::runtime_error("Failed to translate virtual address for read");
    }
    return physical_memory_->read_byte(paddr.value());
}

void VirtualMemoryManager::write_byte(VirtualAddress vaddr, uint8_t value) {
    auto paddr = translate(vaddr, true);
    if (!paddr.has_value()) {
        throw std::runtime_error("Failed to translate virtual address for write");
    }
    physical_memory_->write_byte(paddr.value(), value);
}

bool VirtualMemoryManager::allocate_page(VirtualAddress vaddr) {
    PageNumber vpn = extract_page_number(vaddr);

    if (page_table_->is_present(vpn)) {
        return true;
    }

    return handle_page_fault(vpn);
}

void VirtualMemoryManager::free_page(VirtualAddress vaddr) {
    PageNumber vpn = extract_page_number(vaddr);

    auto entry = page_table_->get_entry(vpn);
    if (entry && entry->valid) {
        FrameNumber pfn = entry->frame_number;

        page_table_->invalidate(vpn);
        tlb_->invalidate(vpn);
        physical_memory_->free_frame(pfn);
    }
}

void VirtualMemoryManager::print_statistics(std::ostream& os) const {
    os << "\n========== Virtual Memory Manager Statistics ==========\n";
    os << std::fixed << std::setprecision(2);

    os << "\nMemory Configuration:\n";
    os << "  Page size: " << config_.page_size << " bytes\n";
    os << "  Virtual address space: " << config_.virtual_address_bits << " bits\n";
    os << "  Physical memory: " << config_.physical_memory_size << " bytes ("
       << (config_.physical_memory_size / 1024) << " KB)\n";
    os << "  Number of frames: " << config_.num_frames << "\n";
    os << "  Page table levels: " << config_.page_table_levels << "\n";
    os << "  TLB size: " << config_.tlb_size << " entries\n";

    os << "\nMemory Access Statistics:\n";
    os << "  Total memory accesses: " << total_accesses_ << "\n";
    os << "  TLB hits: " << tlb_hits_ << "\n";
    os << "  Page table hits: " << page_table_hits_ << "\n";
    os << "  Page faults: " << page_faults_ << "\n";

    if (total_accesses_ > 0) {
        double tlb_hit_rate = static_cast<double>(tlb_hits_) / total_accesses_ * 100.0;
        double pt_hit_rate = static_cast<double>(page_table_hits_) / total_accesses_ * 100.0;
        double fault_rate = static_cast<double>(page_faults_) / total_accesses_ * 100.0;

        os << "\nHit Rates:\n";
        os << "  TLB hit rate: " << tlb_hit_rate << "%\n";
        os << "  Page table hit rate: " << pt_hit_rate << "%\n";
        os << "  Page fault rate: " << fault_rate << "%\n";
    }

    os << "\nMemory Usage:\n";
    os << "  Allocated frames: " << physical_memory_->get_allocated_frames()
       << " / " << physical_memory_->get_num_frames() << "\n";
    os << "  Free frames: " << physical_memory_->get_free_frames() << "\n";
    os << "  Page table entries: " << page_table_->get_num_entries() << "\n";

    os << "======================================================\n\n";
}

void VirtualMemoryManager::reset_statistics() {
    total_accesses_ = 0;
    tlb_hits_ = 0;
    page_table_hits_ = 0;
    page_faults_ = 0;
    tlb_->reset_stats();
    physical_memory_->reset_stats();
}

PageNumber VirtualMemoryManager::extract_page_number(VirtualAddress vaddr) const {
    return vaddr >> config_.offset_bits;
}

size_t VirtualMemoryManager::extract_offset(VirtualAddress vaddr) const {
    size_t mask = (1ULL << config_.offset_bits) - 1;
    return vaddr & mask;
}

bool VirtualMemoryManager::handle_page_fault(PageNumber vpn) {
    auto pfn = physical_memory_->allocate_frame(vpn);
    if (!pfn.has_value()) {
        return false;
    }

    page_table_->insert(vpn, pfn.value());

    return true;
}

} // namespace vm
