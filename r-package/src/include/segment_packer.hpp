#pragma once
#include <vector>

#include "segment.hpp"


// Define two versions of a size_t index type: one for the dynamic programming table and one for the segments themselves.
using DpIdx = int;
using SegmentIdx = size_t;
using RowIdx = size_t;

class SegmentPacker {
public:
    explicit SegmentPacker(std::vector<Segment> segments);

    std::vector<RowIdx> solve();

    std::vector<RowIdx> solve_by_dp();

    const std::vector<Segment>& get_segments() const {
        return master_segments;
    }

private:
    std::vector<Segment> master_segments;
};

class SegmentPackerDP {
private:
    bool initialized = false;
    std::vector<SegmentIdx> m_active;
    std::vector<bool> m_removed;
    std::vector<long> m_ends;
    std::vector<long> m_scores;
    std::vector<DpIdx> m_parent;
    std::vector<DpIdx> m_best_upto;
    std::vector<long> m_cached_start;
    std::vector<long> m_cached_width;

public:
    explicit SegmentPackerDP(std::vector<Segment> segments);

    std::vector<RowIdx> solve();

    const std::vector<Segment>& get_segments() const {
        return master_segments;
    }
private:
    std::vector<Segment> master_segments;

    void initialize_active_list();

    DpIdx get_best_dp_index(const std::vector<long> &max_scores) const;

    std::vector<SegmentIdx> reconstruct_solution(
        const std::vector<DpIdx> &parent_dp_index,
        DpIdx best_dp_index,
        const std::vector<SegmentIdx> &active_segment_indices) const;

    std::vector<SegmentIdx> find_best_row(const std::vector<SegmentIdx> &active_indices);
};
