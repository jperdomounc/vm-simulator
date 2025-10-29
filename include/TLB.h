#ifndef TLB_H
#define TLB_H

#include "Config.h"
#include <unordered_map>
#include <list>
#include <optional>

namespace vm {

class TLB {
public:
    explicit TLB(size_t capacity);

    std::optional<FrameNumber> lookup(PageNumber vpn);
    void insert(PageNumber vpn, FrameNumber pfn);
    void invalidate(PageNumber vpn);
    void clear();

    size_t get_hits() const { return hits_; }
    size_t get_misses() const { return misses_; }
    double get_hit_rate() const {
        size_t total = hits_ + misses_;
        return total > 0 ? static_cast<double>(hits_) / total : 0.0;
    }

    void reset_stats() {
        hits_ = 0;
        misses_ = 0;
    }

private:
    size_t capacity_;
    size_t hits_;
    size_t misses_;

    std::list<PageNumber> lru_list_;
    std::unordered_map<PageNumber, std::pair<FrameNumber, std::list<PageNumber>::iterator>> cache_;

    void touch(PageNumber vpn);
};

} // namespace vm

#endif // TLB_H
