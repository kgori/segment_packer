#include <algorithm>
#include <iostream>
#include <vector>

#include "segment.hpp"
#include "segment_packer.hpp"

int main() {
    const std::vector<Segment> segments {
      {12732, 21473, "A"}, {7665, 15000, "B"}, {343, 7234, "C"},
          {19092, 27889, "D"}, {343, 7234, "E"}, {22100, 26599, "F"},
          {5185, 21580, "G"}, {11438, 27411, "H"}, {8076, 24021, "I"},
          {1996, 18605, "J"}, {18206, 35414, "K"}, {26765, 43516, "L"},
          {9455, 25673, "M"}, {19555, 37194, "N"}, {6461, 24440, "O"},
          {2716, 18718, "P"}, {6243, 24315, "Q"}, {1515, 18454, "R"},
          {30000, 40000, "S"}
    };

    SegmentPacker packer(std::move(segments));
    auto results = packer.solve();
    for (size_t i = 0; i < results.size(); ++i) {
        std::cout << packer.get_segments()[i] << " assigned to row " << results[i] << "\n";
    }

    return 0;
}
