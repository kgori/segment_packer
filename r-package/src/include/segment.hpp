#pragma once
#include <ostream>
#include <string>

struct Segment {
    const long start, end, width; // interval is half open [start, end)
    std::string id;
    Segment(long start_, long end_, std::string id_);
};

std::ostream &operator<<(std::ostream &os, const Segment &s);
