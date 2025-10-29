#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include "Config.h"
#include <vector>
#include <memory>
#include <optional>

namespace vm {

struct PageTableEntry {
    FrameNumber frame_number;
    bool valid;
    bool dirty;
    bool referenced;

    PageTableEntry()
        : frame_number(0), valid(false), dirty(false), referenced(false) {}
};

class PageTable {
public:
    PageTable(const Config& config);

    std::optional<FrameNumber> translate(PageNumber vpn);
    void insert(PageNumber vpn, FrameNumber pfn);
    bool is_present(PageNumber vpn) const;
    PageTableEntry* get_entry(PageNumber vpn);
    void set_dirty(PageNumber vpn, bool dirty = true);
    void set_referenced(PageNumber vpn, bool referenced = true);
    void invalidate(PageNumber vpn);
    void clear();

    size_t get_num_entries() const { return num_entries_; }

private:
    Config config_;
    size_t num_levels_;
    size_t bits_per_level_;
    size_t entries_per_level_;
    size_t num_entries_;

    struct PageTableNode {
        std::vector<std::unique_ptr<PageTableNode>> children;
        std::vector<PageTableEntry> entries;
        bool is_leaf;

        PageTableNode(size_t size, bool leaf) : is_leaf(leaf) {
            if (leaf) {
                entries.resize(size);
            } else {
                children.resize(size);
            }
        }
    };

    std::unique_ptr<PageTableNode> root_;

    size_t extract_level_index(PageNumber vpn, size_t level) const;
    PageTableEntry* walk_page_table(PageNumber vpn, bool create);
};

} // namespace vm

#endif // PAGE_TABLE_H
