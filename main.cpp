#include <algorithm>
#include <iostream>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

// Define two versions of a size_t index type: one for the dynamic programming table and one for the segments themselves.
using DpIdx = size_t;
using SegmentIdx = size_t;

namespace rp {
    struct Segment {
        long start, end, width; // interval is half open [start, end)
        std::string id;
        Segment(long start_, long end_, std::string id_)
            : start(start_), end(end_), width(end_ - start_), id(id_) {
            if (start >= end) {
                throw std::invalid_argument("start must be less than end");
            }
        }
    };

    std::ostream &operator<<(std::ostream &os, const Segment &s) {
        os << s.id << " [" << s.start << ", " << s.end << "] = " << s.width;
        return os;
    }
}

namespace algorithm {
    size_t best_index(const std::vector<long>& max_scores) {
        auto best = std::max_element(max_scores.begin(), max_scores.end());
        return std::distance(max_scores.begin(), best);
    }

    std::vector<size_t>
    reconstruct_solution(const std::vector<std::optional<DpIdx>> &parent_dp_index,
                         const std::vector<long> &max_scores,
                         const std::vector<rp::Segment> &segments,
                         const std::vector<SegmentIdx> &active_segment_indices) {
        std::optional<DpIdx> best_dp_index = best_index(max_scores);
        std::vector<SegmentIdx> solution;
        while (best_dp_index) {
            size_t seg_index = active_segment_indices[*best_dp_index];
            solution.push_back(seg_index);
            best_dp_index = parent_dp_index[*best_dp_index];
        }
        std::reverse(solution.begin(), solution.end());
        return solution;
    }

    std::vector<SegmentIdx>
    find_best_row(const std::vector<rp::Segment> &segments,
                  const std::vector<SegmentIdx> &active_segment_indices) {
        size_t n = active_segment_indices.size();
        if (n == 0) {
            return {};
        }

        // 1. Extract endpoints to allow binary search
        std::vector<long> ends;
        ends.reserve(n);
        for (SegmentIdx idx : active_segment_indices) {
            ends.push_back(segments[idx].end);
        }
        
        std::vector<std::optional<DpIdx>> parent_dp_index(n, std::nullopt);
        std::vector<long> max_scores(n, 0);
        std::vector<DpIdx> running_best_index(n, 0); 
        
        for (DpIdx i = 0; i < n; ++i) {
            const rp::Segment& seg_i = segments[active_segment_indices[i]];
            max_scores[i] = seg_i.width;

            // 2. Binary search for the first segment that ends after seg_i starts
            auto it = std::upper_bound(ends.begin(), ends.end(), seg_i.start);

            // If it's not the beginning, the element before it is the last segment that ends before seg_i starts
            if (it != ends.begin()) {
                size_t p_i = std::distance(ends.begin(), it) - 1;

                // Get the index holding the max score up to p_i
                SegmentIdx best_predecessor_index = running_best_index[p_i];
                
                max_scores[i] = max_scores[best_predecessor_index] + seg_i.width;
                parent_dp_index[i] = best_predecessor_index;
            }

            // 3. Maintain the running maximum index tracker
            if (i == 0) {
                running_best_index[i] = 0;
            } else {
                if (max_scores[i] > max_scores[running_best_index[i - 1]]) {
                    running_best_index[i] = i;
                } else {
                    running_best_index[i] = running_best_index[i - 1];
                }
            }
        }
        return algorithm::reconstruct_solution(parent_dp_index, max_scores, segments, active_segment_indices);
    }
}

int main() {
    const std::vector<rp::Segment> segments {
        {12732, 21473, "A"},
        {7665, 15000, "B"},
        {343, 7234, "C"},
        {19092, 27889, "D"},
        {343, 7234, "E"},
        {22100, 26599, "F"},
    };

    std::vector<SegmentIdx> global_sorted_indices(segments.size());
    std::iota(global_sorted_indices.begin(), global_sorted_indices.end(), 0);
    std::sort(global_sorted_indices.begin(), global_sorted_indices.end(),
              [&segments](const SegmentIdx &a, const SegmentIdx &b) {
                    if (segments[a].end == segments[b].end) {
                        return segments[a].start < segments[b].start;
                    }
                    return segments[a].end < segments[b].end;
              });

    std::vector<bool> expired(segments.size(), false);
    size_t remaining_count = segments.size();
 
    for (const auto &s : segments) {
        std::cout << s << std::endl;
    }
    std::cout << std::endl;

    while (remaining_count > 0) {
        std::vector<SegmentIdx> active_segment_indices;
        active_segment_indices.reserve(remaining_count);
        for (auto idx : global_sorted_indices) {
            if (!expired[idx]) {
                active_segment_indices.push_back(idx);
            }
        }
        
        auto best_row = algorithm::find_best_row(segments, active_segment_indices);

        for (auto &i : best_row) {
          std::cout << segments[i].id << ", ";
          expired[i] = true;
          remaining_count--;
        }
        std::cout << std::endl;
    }
}
