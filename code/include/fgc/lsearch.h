#pragma once
#ifndef FGC_LOCAL_SEARCH_H__
#define FGC_LOCAL_SEARCH_H__
#include "fgc/graph.h"
#include "fgc/diskmat.h"
#include "fgc/kcenter.h"

/*
 * In this file, we use the local search heuristic for k-median.
 * Originally described in "Local Search Heuristics for k-median and Facility Location Problems",
 * Vijay Arya, Naveen Garg, Rohit Khandekar, Adam Meyerson, Kamesh Munagala, Vinayaka Pandit
 * (http://theory.stanford.edu/~kamesh/lsearch.pdf)
 */

namespace fgc {

template<typename Graph>
DiskMat<typename Graph::edge_property_type::value_type> graph2diskmat(const Graph &x, std::string path) {
    static_assert(std::is_arithmetic<typename Graph::edge_property_type::value_type>::value, "This should be floating point, or at least arithmetic");
    using FT = typename Graph::edge_property_type::value_type;
    auto nv = boost::num_vertices(x);
    DiskMat<FT> ret(nv, nv, path);
    //std::vector<typename boost::graph_traits<Graph>::vertex_descriptor> vertices(boost::vertexs(x).first, boost::vertexs(x).second);
    typename boost::graph_traits<Graph>::vertex_iterator vertices = boost::vertices(x).first;
    const size_t e = boost::num_vertices(x);
    OMP_PFOR
    for(size_t i = 0; i < e; ++i) {
        auto mr = row(~ret, i);
        boost::dijkstra_shortest_paths(x, vertices[i], distance_map(&mr[0]));
    }
    assert((~ret).rows() == nv && (~ret).columns() == nv);
    return ret;
}

#if 0
template<typename Mat, typename Norm>
struct MatrixIndexNorm {
    const Mat mat_;
    const Norm norm_;
    template<typename Mat>
    MatrixMetric(const Mat &mat): mat_(mat) {}
    template<typename AT>
    auto operator()(size_t i, size_t j) const {
        return mat_(i, j);
    }
};
#endif

template<typename MatType, typename WFT=float, typename IType=std::uint32_t>
struct LocalKMedSearcher {
    using DType = typename MatType::ElementType;
    using SolType = blaze::SmallArray<IType, 16>;


    const MatType &mat_;
    SolType sol_;
    std::vector<IType> assignments_;
    blz::DV<DType> costs_;
    SolType counts_;
    double current_cost_;

    // Constructors

    LocalKMedSearcher(const LocalKMedSearcher &o) = default;
    LocalKMedSearcher(LocalKMedSearcher &&o) {
        auto ptr = reinterpret_cast<const uint8_t *>(this);
        std::memset(ptr, 0, sizeof(*this));
        std::swap_ranges(ptr, ptr + sizeof(*this), reinterpret_cast<const uint8_t *>(std::addressof(o)));
    }

    void assign() {
        const size_t nc = mat_.columns(), nr = mat_.rows(), ncand = sol_.size();
        assert(assignments_.size() == nc);
        OMP_PFOR
        for(size_t ri = 0; ri < ncand; ++ri) {
            assert(sol_[ri] < mat_.rows());
            auto r = row(mat_, sol_[ri], blaze::unchecked);
            for(size_t ci = 0; ci < nc; ++ci) {
                assert(ci < r.size());
                if(const auto newcost = r[ci];
                   newcost < costs_[ci])
                {
                    OMP_CRITICAL
                    {
                        if(newcost < costs_[ci]) {
                            costs_[ci] = newcost;
                            assignments_[ci] = ri;
                        }
                    }
                }
            }
        }
#ifndef NDEBUG
        for(size_t i = 0; i < nc; ++i) {
            std::fprintf(stderr, "index %zu. cost: %f. assignment: %zu\n", i, costs_[i], size_t(assignments_[i]));
        }
        for(const auto asn: assignments_)
            assert(asn < nr);
#endif
        assert(assignments_.size() == nc);
        current_cost_ = blaze::sum(costs_);
        std::fill(counts_.begin(), counts_.end(), IType(0));
        OMP_PFOR
        for(size_t ci = 0; ci < nc; ++ci) {
            OMP_ATOMIC
            ++counts_[assignments_[ci]];
        }
        assert(std::accumulate(counts_.begin(), counts_.end(), 0u) == nc || !std::fprintf(stderr, "Doesn't add up: %u\n", std::accumulate(counts_.begin(), counts_.end(), 0u)));
        std::fprintf(stderr, "current cost: %f\n", current_cost_);
        assert(counts_.size() == sol_.size());
        for(size_t i = 0; i < counts_.size(); ++i) {
            std::fprintf(stderr, "count %u is %zu\n", sol_[i], counts_[i]);
        }
        std::fputc('\n', stderr);
    }

    // Steps:
    // 1. Use k-center approx for seeds
    // 2. Loop over finding candidate replacements and performing swaps.
    LocalKMedSearcher(const MatType &mat, unsigned k, double eps=0.01):
        mat_(mat), assignments_(mat.columns(), IType(-1)),
        costs_(mat.columns(), std::numeric_limits<DType>::max()),
        counts_(k),
        current_cost_(std::numeric_limits<DType>::max())
    {
        wy::WyRand<IType, 2> rng(k / eps * mat.rows() + mat.columns());
        auto rowits = rowiterator(mat);
        auto approx = coresets::kcenter_greedy_2approx(rowits.begin(), rowits.end(), rng, k, MatrixLookup());
        std::fprintf(stderr, "Approx is complete with size %zu (expected k: %u)\n", approx.size(), k);
        sol_.resize(k);
        std::copy(approx.begin(), approx.end(), sol_.begin());
        pdqsort(sol_.data(), sol_.data() + sol_.size()); // Just for access pattern
        assign();
    }
};

template<typename Mat, typename FT=float, typename IType=std::uint32_t>
auto make_kmed_lsearcher(const Mat &mat, unsigned k, double eps=0.01) {
    return LocalKMedSearcher<Mat, FT, IType>(mat, k, eps);
}

} // fgc

#endif /* FGC_LOCAL_SEARCH_H__ */
