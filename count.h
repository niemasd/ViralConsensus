#ifndef COUNT_H
#define COUNT_H
#include "htslib/htslib/sam.h"
#include <omp.h>
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

// definitions
#define COUNT_T unsigned int

// struct to store counts
struct counts_t {
    std::vector<std::array<COUNT_T, 4>> pos_counts;      // position base counts
    std::unordered_map<std::string, COUNT_T> ins_counts; // insertion counts
};

// compute position and insertion counts
counts_t compute_counts(const char* const in_reads_fn, const char* const in_ref_fn);
#endif
