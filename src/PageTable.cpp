#include "PageTable.h"
#include <cmath>

namespace vm {

PageTable::PageTable(const Config& config)
    : config_(config),
      num_levels_(config.page_table_levels),
      bits_per_level_(config.bits_per_level),
      entries_per_level_(1ULL << config.bits_per_level),
      num_entries_(0) {

    root_ = std::make_unique<PageTableNode>(entries_per_level_, num_levels_ == 1);
}

std::optional<FrameNumber> PageTable::translate(PageNumber vpn) {
    PageTableEntry* entry = walk_page_table(vpn, false);
    if (entry && entry->valid) {
        entry->referenced = true;
        return entry->frame_number;
    }
    return std::nullopt;
}

void PageTable::insert(PageNumber vpn, FrameNumber pfn) {
    PageTableEntry* entry = walk_page_table(vpn, true);
    if (entry) {
        if (!entry->valid) {
            num_entries_++;
        }
        entry->frame_number = pfn;
        entry->valid = true;
        entry->referenced = true;
    }
}

bool PageTable::is_present(PageNumber vpn) const {
    PageTableEntry* entry = const_cast<PageTable*>(this)->walk_page_table(vpn, false);
    return entry && entry->valid;
}

PageTableEntry* PageTable::get_entry(PageNumber vpn) {
    return walk_page_table(vpn, false);
}

void PageTable::set_dirty(PageNumber vpn, bool dirty) {
    PageTableEntry* entry = walk_page_table(vpn, false);
    if (entry && entry->valid) {
        entry->dirty = dirty;
    }
}

void PageTable::set_referenced(PageNumber vpn, bool referenced) {
    PageTableEntry* entry = walk_page_table(vpn, false);
    if (entry && entry->valid) {
        entry->referenced = referenced;
    }
}

void PageTable::invalidate(PageNumber vpn) {
    PageTableEntry* entry = walk_page_table(vpn, false);
    if (entry && entry->valid) {
        entry->valid = false;
        num_entries_--;
    }
}

void PageTable::clear() {
    root_ = std::make_unique<PageTableNode>(entries_per_level_, num_levels_ == 1);
    num_entries_ = 0;
}

size_t PageTable::extract_level_index(PageNumber vpn, size_t level) const {
    size_t shift = (num_levels_ - 1 - level) * bits_per_level_;
    size_t mask = (1ULL << bits_per_level_) - 1;
    return (vpn >> shift) & mask;
}

PageTableEntry* PageTable::walk_page_table(PageNumber vpn, bool create) {
    PageTableNode* current = root_.get();

    for (size_t level = 0; level < num_levels_; ++level) {
        size_t index = extract_level_index(vpn, level);

        if (current->is_leaf) {
            return &current->entries[index];
        }

        if (!current->children[index]) {
            if (!create) {
                return nullptr;
            }
            bool is_last_level = (level == num_levels_ - 2);
            current->children[index] = std::make_unique<PageTableNode>(
                entries_per_level_, is_last_level);
        }

        current = current->children[index].get();
    }

    return nullptr;
}

} // namespace vm
