#include "TLB.h"

namespace vm {

TLB::TLB(size_t capacity)
    : capacity_(capacity), hits_(0), misses_(0) {}

std::optional<FrameNumber> TLB::lookup(PageNumber vpn) {
    auto it = cache_.find(vpn);
    if (it != cache_.end()) {
        hits_++;
        touch(vpn);
        return it->second.first;
    }
    misses_++;
    return std::nullopt;
}

void TLB::insert(PageNumber vpn, FrameNumber pfn) {
    auto it = cache_.find(vpn);

    if (it != cache_.end()) {
        it->second.first = pfn;
        touch(vpn);
    } else {
        if (cache_.size() >= capacity_) {
            PageNumber lru_vpn = lru_list_.back();
            lru_list_.pop_back();
            cache_.erase(lru_vpn);
        }

        lru_list_.push_front(vpn);
        cache_[vpn] = {pfn, lru_list_.begin()};
    }
}

void TLB::invalidate(PageNumber vpn) {
    auto it = cache_.find(vpn);
    if (it != cache_.end()) {
        lru_list_.erase(it->second.second);
        cache_.erase(it);
    }
}

void TLB::clear() {
    cache_.clear();
    lru_list_.clear();
}

void TLB::touch(PageNumber vpn) {
    auto it = cache_.find(vpn);
    if (it != cache_.end()) {
        lru_list_.erase(it->second.second);
        lru_list_.push_front(vpn);
        it->second.second = lru_list_.begin();
    }
}

} // namespace vm
