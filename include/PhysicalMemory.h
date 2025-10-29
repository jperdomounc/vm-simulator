#ifndef PHYSICAL_MEMORY_H
#define PHYSICAL_MEMORY_H

#include "Config.h"
#include <vector>
#include <queue>
#include <optional>

namespace vm {

struct Frame {
    bool allocated;
    PageNumber owner_vpn;
    bool pinned;

    Frame() : allocated(false), owner_vpn(0), pinned(false) {}
};

class PhysicalMemory {
public:
    explicit PhysicalMemory(const Config& config);

    std::optional<FrameNumber> allocate_frame(PageNumber vpn);
    void free_frame(FrameNumber pfn);
    bool is_allocated(FrameNumber pfn) const;
    const Frame& get_frame(FrameNumber pfn) const;
    void pin_frame(FrameNumber pfn);
    void unpin_frame(FrameNumber pfn);
    uint8_t read_byte(PhysicalAddress addr);
    void write_byte(PhysicalAddress addr, uint8_t value);

    size_t get_num_frames() const { return num_frames_; }
    size_t get_free_frames() const { return free_frames_.size(); }
    size_t get_allocated_frames() const { return allocated_frames_; }
    size_t get_page_faults() const { return page_faults_; }

    void reset_stats() { page_faults_ = 0; }

private:
    Config config_;
    size_t num_frames_;
    size_t allocated_frames_;
    size_t page_faults_;

    std::vector<Frame> frames_;
    std::vector<uint8_t> memory_;
    std::queue<FrameNumber> free_frames_;

    std::optional<FrameNumber> find_victim_frame();
};

} // namespace vm

#endif // PHYSICAL_MEMORY_H
