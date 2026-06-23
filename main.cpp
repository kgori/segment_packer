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
    
    std::vector<std::vector<SegmentIdx>> solve() {
        std::vector<std::vector<SegmentIdx>> rows;
        size_t remaining_count = master_segments.size();

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
                remaining_count--;
            }
            rows.push_back(std::move(best_row));
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
        
        std::optional<DpIdx> best_dp_index = get_best_dp_index(max_scores);
        std::vector<SegmentIdx> solution;
        
        while (best_dp_index) {
            SegmentIdx seg_index = active_segment_indices[*best_dp_index];
            solution.push_back(seg_index);
            best_dp_index = parent_dp_index[*best_dp_index];
        }
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
        
        std::vector<std::optional<DpIdx>> parent_dp_index(n, std::nullopt);
        std::vector<long> max_scores(n, 0);
        std::vector<DpIdx> running_best_dp_index(n, 0); 
        
        for (DpIdx i = 0; i < n; ++i) {
            const Segment& seg_i = master_segments[active_indices[i]];
            max_scores[i] = seg_i.width;

            // 2. Binary search for the first segment that ends after seg_i starts
            auto it = std::upper_bound(ends.begin(), ends.end(), seg_i.start);

            // If it's not the beginning, the element before it is the last segment that ends before seg_i starts
            if (it != ends.begin()) {
                DpIdx p_i = std::distance(ends.begin(), it) - 1;

                // Get the index holding the max score up to p_i
                DpIdx best_predecessor_dp_index = running_best_dp_index[p_i];
                
                max_scores[i] = max_scores[best_predecessor_dp_index] + seg_i.width;
                parent_dp_index[i] = best_predecessor_dp_index;
            }

            // 3. Maintain the running maximum index tracker
            if (i == 0) {
                running_best_dp_index[i] = 0;
            } else {
                if (max_scores[i] > max_scores[running_best_dp_index[i - 1]]) {
                    running_best_dp_index[i] = i;
                } else {
                    running_best_dp_index[i] = running_best_dp_index[i - 1];
                }
            }
        }
        return reconstruct_solution(parent_dp_index, max_scores, active_indices);
    }
};
    
    


namespace algorithm {


}

int main() {
    const std::vector<Segment> segments {
        {12732, 21473, "A"},
        {7665, 15000, "B"},
        {343, 7234, "C"},
        {19092, 27889, "D"},
        {343, 7234, "E"},
        {22100, 26599, "F"},
    };

    SegmentPacker packer(std::move(segments));
    auto results = packer.solve();
    const auto &master_list = packer.get_segments();

    for (size_t row_idx = 0; row_idx < results.size(); ++row_idx) {
        std::cout << "Row " << row_idx + 1 << ": ";
        for (auto seg_idx : results[row_idx]) {
            std::cout << master_list[seg_idx] << " ";
        }
        std::cout << std::endl;
    }
}
