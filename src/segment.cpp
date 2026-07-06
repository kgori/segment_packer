#include "segment.hpp"

Segment::Segment(long start_, long end_, std::string id_)
    : start(start_), end(end_), width(end_ - start_), id(std::move(id_)) {
    if (start >= end) {
        throw std::invalid_argument("start must be less than end");
    }
}

std::ostream &operator<<(std::ostream &os, const Segment &s) {
    os << s.id << " [" << s.start << ", " << s.end << "] = " << s.width;
    return os;
}
