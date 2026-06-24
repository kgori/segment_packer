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
using RowIdx = size_t;

struct Segment {
    const long start, end, width; // interval is half open [start, end)
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

class SegmentPacker {
public:
    explicit SegmentPacker(std::vector<Segment> segments)
        : master_segments(std::move(segments)), expired(master_segments.size(), false) {
        
        // Initialise and sort global tracking index by end
        global_sorted_indices = std::vector<SegmentIdx>(master_segments.size());
        std::iota(global_sorted_indices.begin(), global_sorted_indices.end(),
                  0);
        std::sort(global_sorted_indices.begin(), global_sorted_indices.end(),
                  [this](const SegmentIdx &a, const SegmentIdx &b) {
                      if (master_segments[a].end == master_segments[b].end) {
                          return master_segments[a].start < master_segments[b].start;
                      }
                      return master_segments[a].end < master_segments[b].end;
                  });
    }
    
    std::vector<RowIdx> solve() {
        // Reset expired flags for all segments
        std::fill(expired.begin(), expired.end(), false);
      
        std::vector<RowIdx> rows(master_segments.size(), 0);
        size_t remaining_count = master_segments.size();
        RowIdx current_row = 0;

        while (remaining_count > 0) {
            std::vector<SegmentIdx> active_segment_indices;
            active_segment_indices.reserve(remaining_count);

            for (SegmentIdx idx : global_sorted_indices) {
                if (!expired[idx]) {
                active_segment_indices.push_back(idx);
                }
            }

            std::vector<SegmentIdx> best_row =
                find_best_row(active_segment_indices);

            for (SegmentIdx idx : best_row) {
                expired[idx] = true;
                rows[idx] = current_row;
                remaining_count--;
            }
            current_row++;
        }
        return rows;
    }
    
    const std::vector<Segment>& get_segments() const {
        return master_segments;
    }

private:
    std::vector<Segment> master_segments;
    std::vector<SegmentIdx> global_sorted_indices;
    std::vector<bool> expired;

    DpIdx get_best_dp_index(const std::vector<long> &max_scores) const {
        auto best = std::max_element(max_scores.begin(), max_scores.end());
        return std::distance(max_scores.begin(), best);
    }

    std::vector<SegmentIdx> reconstruct_solution(
        const std::vector<std::optional<DpIdx>> &parent_dp_index,
        const std::vector<long> &max_scores,
        const std::vector<SegmentIdx> &active_segment_indices) const {
        
        std::vector<SegmentIdx> solution;
        DpIdx current = get_best_dp_index(max_scores);
        do {
            SegmentIdx seg_index = active_segment_indices[current];
            solution.push_back(seg_index);
            auto next = parent_dp_index[current];
            if (!next) break;
            current = *next;
        } while (true);
        
        std::reverse(solution.begin(), solution.end());
        return solution;
    }

    std::vector<SegmentIdx> find_best_row(const std::vector<SegmentIdx> &active_indices) {
        DpIdx n = active_indices.size();
        if (n == 0) {
            return {};
        }

        // 1. Extract endpoints to allow binary search
        std::vector<long> ends;
        ends.reserve(n);
        for (SegmentIdx idx : active_indices) {
            ends.push_back(master_segments[idx].end);
        }

        // parent_dp_index[i] = DP index of the best predecessor of i
        // max_scores[i] = max total width of non-overlapping selection of
        //                 segments from active_indices[0..=i] 
        // best_upto[i] = arg max_{j<=i} max scores[j] (prefix-maximum tracker)
        std::vector<std::optional<DpIdx>> parent_dp_index(n, std::nullopt);
        std::vector<long> max_scores(n, 0);
        std::vector<DpIdx> best_upto(n, 0); 
        
        for (DpIdx i = 0; i < n; ++i) {
            const Segment& seg_i = master_segments[active_indices[i]];
            max_scores[i] = seg_i.width;

            // Find the rightmost predecessor whose end <= seg_i.start using binary search
            auto it = std::upper_bound(ends.begin(), ends.end(), seg_i.start);

            // If it's not the beginning, the element before it is the last segment that ends before seg_i starts
            if (it != ends.begin()) {
                DpIdx p_i = std::distance(ends.begin(), it) - 1;

                // Get the index holding the max score up to p_i
                DpIdx best_predecessor_dp_index = best_upto[p_i];
                
                max_scores[i] = max_scores[best_predecessor_dp_index] + seg_i.width;
                parent_dp_index[i] = best_predecessor_dp_index;
            }

            // 3. Maintain the running maximum index tracker
            if (i == 0) {
                best_upto[i] = 0;
            } else {
                if (max_scores[i] > max_scores[best_upto[i - 1]]) {
                    best_upto[i] = i;
                } else {
                    best_upto[i] = best_upto[i - 1];
                }
            }
        }
        return reconstruct_solution(parent_dp_index, max_scores, active_indices);
    }
};

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
