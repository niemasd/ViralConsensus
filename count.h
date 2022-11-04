#ifndef COUNT_H
#define COUNT_H
#include "common.h"
#include <omp.h>
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

// struct to store counts
struct counts_t {
    std::vector<std::array<COUNT_T, 5>> pos_counts;                                    // position base counts
    std::unordered_map<uint32_t, std::unordered_map<std::string, COUNT_T>> ins_counts; // insertion counts
};

// compute position and insertion counts
counts_t compute_counts(const char* const in_reads_fn, const char* const in_ref_fn, int const min_qual);

// print pos_counts (for debugging)
void print_pos_counts(std::vector<std::array<COUNT_T, 5>> const & pos_counts, char delim);

// print ins_counts as JSON (for debugging)
void print_ins_counts_json(std::unordered_map<uint32_t, std::unordered_map<std::string, COUNT_T>> & ins_counts);
#endif
