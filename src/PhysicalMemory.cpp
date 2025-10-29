#include "PhysicalMemory.h"
#include <stdexcept>

namespace vm {

PhysicalMemory::PhysicalMemory(const Config& config)
    : config_(config),
      num_frames_(config.num_frames),
      allocated_frames_(0),
      page_faults_(0) {

    frames_.resize(num_frames_);
    memory_.resize(config.physical_memory_size, 0);

    for (size_t i = 0; i < num_frames_; ++i) {
        free_frames_.push(i);
    }
}

std::optional<FrameNumber> PhysicalMemory::allocate_frame(PageNumber vpn) {
    page_faults_++;

    if (!free_frames_.empty()) {
        FrameNumber pfn = free_frames_.front();
        free_frames_.pop();

        frames_[pfn].allocated = true;
        frames_[pfn].owner_vpn = vpn;
        allocated_frames_++;

        return pfn;
    }

    return find_victim_frame();
}

void PhysicalMemory::free_frame(FrameNumber pfn) {
    if (pfn >= num_frames_) {
        throw std::out_of_range("Invalid frame number");
    }

    if (frames_[pfn].allocated) {
        frames_[pfn].allocated = false;
        frames_[pfn].owner_vpn = 0;
        frames_[pfn].pinned = false;
        allocated_frames_--;
        free_frames_.push(pfn);
    }
}

bool PhysicalMemory::is_allocated(FrameNumber pfn) const {
    if (pfn >= num_frames_) {
        return false;
    }
    return frames_[pfn].allocated;
}

const Frame& PhysicalMemory::get_frame(FrameNumber pfn) const {
    if (pfn >= num_frames_) {
        throw std::out_of_range("Invalid frame number");
    }
    return frames_[pfn];
}

void PhysicalMemory::pin_frame(FrameNumber pfn) {
    if (pfn >= num_frames_) {
        throw std::out_of_range("Invalid frame number");
    }
    frames_[pfn].pinned = true;
}

void PhysicalMemory::unpin_frame(FrameNumber pfn) {
    if (pfn >= num_frames_) {
        throw std::out_of_range("Invalid frame number");
    }
    frames_[pfn].pinned = false;
}

uint8_t PhysicalMemory::read_byte(PhysicalAddress addr) {
    if (addr >= memory_.size()) {
        throw std::out_of_range("Physical address out of range");
    }
    return memory_[addr];
}

void PhysicalMemory::write_byte(PhysicalAddress addr, uint8_t value) {
    if (addr >= memory_.size()) {
        throw std::out_of_range("Physical address out of range");
    }
    memory_[addr] = value;
}

std::optional<FrameNumber> PhysicalMemory::find_victim_frame() {
    for (size_t i = 0; i < num_frames_; ++i) {
        if (frames_[i].allocated && !frames_[i].pinned) {
            return i;
        }
    }

    return std::nullopt;
}

} // namespace vm
