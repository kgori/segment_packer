#include <Rcpp.h>
#include "segment_packer.hpp"
using namespace Rcpp;

//' pack
//'
//' Assign segments to layers such that no segments in the same layer overlap.
//' Returns a vector of layer indices corresponding to the input segments.
//' @param starts Integer vector of segment start positions.
//' @param ends Integer vector of segment end positions.
//' @param use_dp Flag to choose the slower dynamic programming algorithm
//' that guarantees each layer is maximally packed (default: false). The
//' alternative sweep algorithm guarantees the minimum number of layers and is
//' much faster.
//' @export
// [[Rcpp::export]]
IntegerVector pack(IntegerVector starts, IntegerVector ends, bool use_dp = false) {
  if (starts.length() != ends.length()) {
    stop("Lengths don't match");
  }
  std::vector<Segment> segments;
  segments.reserve(starts.length());
  for (int i = 0; i < starts.length(); ++i) {
    segments.emplace_back(starts[i], ends[i], "x");
  }

  if (use_dp) {
    SegmentPackerDP packer(segments);
    auto solution = packer.solve();
    return wrap(solution);
  }
  
  SegmentPacker packer(segments);
  auto solution = packer.solve();
  return wrap(solution);
}
