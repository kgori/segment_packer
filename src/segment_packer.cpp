#include "segment_packer.hpp"
#include <algorithm>
#include <numeric>
#include <queue>

SegmentPacker::SegmentPacker(std::vector<Segment> segments)
    : master_segments(std::move(segments)) {}

SegmentPackerDP::SegmentPackerDP(std::vector<Segment> segments)
  : master_segments(std::move(segments)) {
    const size_t n = master_segments.size();
    m_ends.reserve(n);
    m_scores.reserve(n);
    m_parent.reserve(n);
    m_best_upto.reserve(n);
    m_cached_start.reserve(n);
    m_cached_width.reserve(n);
}

std::vector<RowIdx> SegmentPacker::solve() {
    const size_t n = master_segments.size();
    std::vector<RowIdx> rows(n);

    std::vector<SegmentIdx> by_start(n);
    std::iota(by_start.begin(), by_start.end(), 0);
    std::sort(by_start.begin(), by_start.end(),
                [this](SegmentIdx a, SegmentIdx b) {
                return master_segments[a].start < master_segments[b].start;
                });

    // Min heap on row numbers
    std::priority_queue<RowIdx, std::vector<RowIdx>, std::greater<RowIdx>> free_rows;

    // Min heap on ends
    using Entry = std::pair<long, RowIdx>; // (end, row)
    std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> occupied;

    RowIdx next_row = 0;

    for (SegmentIdx idx : by_start) {
        const Segment &s = master_segments[idx];

        while (!occupied.empty() && occupied.top().first <= s.start) {
            free_rows.push(occupied.top().second);
            occupied.pop();
        }

        RowIdx r;
        if (!free_rows.empty()) {
            r = free_rows.top();
            free_rows.pop();
        } else {
            r = next_row++;
        }

        rows[idx] = r;
        occupied.emplace(s.end, r);

    }
    return rows;
}

std::vector<RowIdx> SegmentPackerDP::solve() {
    if (!initialized) {
        initialize_active_list();
    }

    const size_t n = master_segments.size();
    std::vector<RowIdx> rows(n, 0);
    RowIdx current_row = 0;

    m_removed.assign(n, false);

    while (!m_active.empty()) {
        for (SegmentIdx idx : find_best_row(m_active)) {
            rows[idx] = current_row;
            m_removed[idx] = true;
        }

        m_active.erase(std::remove_if(m_active.begin(), m_active.end(),
            [&](SegmentIdx i) { return m_removed[i]; }),
            m_active.end());

        current_row++;
    }

    initialized = false; // Mark as uninitialized to reinitialize the list when calling this function again
    return rows;
}

void SegmentPackerDP::initialize_active_list() {
    if (!initialized) {
        const size_t n = master_segments.size();

        m_active.resize(n);
        std::iota(m_active.begin(), m_active.end(), 0);
        std::sort(m_active.begin(), m_active.end(),
                [this](SegmentIdx a, SegmentIdx b) {
                    if (master_segments[a].end == master_segments[b].end) {
                        return master_segments[a].start < master_segments[b].start;
                    }
                    return master_segments[a].end < master_segments[b].end;
                });
        
        initialized = true;
    }
}

DpIdx SegmentPackerDP::get_best_dp_index(const std::vector<long> &max_scores) const {
    auto best = std::max_element(max_scores.begin(), max_scores.end());
    return std::distance(max_scores.begin(), best);
}

std::vector<SegmentIdx> SegmentPackerDP::reconstruct_solution(
    const std::vector<DpIdx> &parent_dp_index,
    DpIdx best_dp_index,
    const std::vector<SegmentIdx> &active_segment_indices) const {

    std::vector<SegmentIdx> solution;
    DpIdx current = best_dp_index;
    
    while (current >= 0) {
        SegmentIdx seg_index = active_segment_indices[current];
        solution.push_back(seg_index);
        current = parent_dp_index[current];
    }

    std::reverse(solution.begin(), solution.end());
    return solution;
}

std::vector<SegmentIdx> SegmentPackerDP::find_best_row(const std::vector<SegmentIdx> &active_indices) {
    const size_t n = active_indices.size();
    if (n == 0) {
        return {};
    }

    // Purpose of scratch buffers:
    // m_parent[i]    = DP index of the best predecessor of i
    // m_scores[i]    = max total width of non-overlapping selection of
    //                  segments from active_indices[0..=i]
    // m_best_upto[i] = arg max_{j<=i} max scores[j] (prefix-maximum tracker)

    // Clear out the scratch memory. ends, scores and best_upto only need
    // resizing, as they are fully overwritten during the routine. parent
    // needs to be explicitly cleared.
    m_ends.resize(n);
    m_scores.resize(n);
    m_best_upto.resize(n);
    m_cached_start.resize(n);
    m_cached_width.resize(n);
    m_parent.assign(n, -1);

    // Setup
    for (size_t i = 0; i < n; ++i) {
        const Segment &s = master_segments[active_indices[i]];
        m_ends[i] = s.end;
        m_cached_start[i] = s.start;
        m_cached_width[i] = s.width;
    }

    // DP loop
    for (size_t i = 0; i < n; ++i) {
        m_scores[i] = m_cached_width[i];  // Base case: only take the current segment

        // Find the rightmost predecessor whose end <= seg_i.start using binary search
        const auto it = std::upper_bound(m_ends.begin(), m_ends.begin() + i, m_cached_start[i]);

        // If it's not the beginning, the element before it is the last segment that ends before seg_i starts
        if (it != m_ends.begin()) {
            const DpIdx p_i = static_cast<DpIdx>(it - m_ends.begin()) - 1;

            // Get the index holding the max score up to p_i
            const DpIdx best_predecessor_dp_index = m_best_upto[p_i];

            m_scores[i] = m_scores[best_predecessor_dp_index] + m_cached_width[i];
            m_parent[i] = best_predecessor_dp_index;
        }

        // 3. Maintain the running maximum index tracker
        m_best_upto[i] = (i == 0 || m_scores[i] > m_scores[m_best_upto[i - 1]])
                            ? i
                            : m_best_upto[i - 1];
    }

    return reconstruct_solution(m_parent, m_best_upto[n-1],  active_indices);
}
