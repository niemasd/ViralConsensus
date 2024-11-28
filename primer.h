#ifndef PRIMER_H
#define PRIMER_H
#include "common.h"

// load primer BED file (first is forward primers; second is reverse primers)
std::pair<std::vector<std::pair<uint32_t,uint32_t>>, std::vector<std::pair<uint32_t,uint32_t>>> read_bed(const char* const primer_bed_fn);

// print primers (for debugging)
void print_primers(std::vector<std::pair<uint32_t, uint32_t>> const & primers);

// find all primers that cover every position of the reference genome (first is forward primers; second is reverse primers)
std::vector<std::pair<std::pair<uint32_t,uint32_t>, std::pair<uint32_t,uint32_t>>> find_overlapping_primers(uint32_t const ref_len, const char* const primer_bed_fn, uint16_t primer_offset);

#endif
