#ifndef PRIMER_H
#define PRIMER_H
#include "common.h"

// load primer BED file
std::vector<std::pair<int32_t, int32_t>> read_bed(const char* const primer_bed_fn);

// print primers (for debugging)
void print_primers(std::vector<std::pair<int32_t, int32_t>> const & primers);

// find all primers that cover every position of the reference genome
std::vector<std::pair<int32_t, int32_t>> find_overlapping_primers(int32_t const ref_len, const char* const primer_bed_fn, int16_t primer_offset);

#endif
