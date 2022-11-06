#ifndef COUNT_H
#define COUNT_H
#include "common.h"

// struct to store counts
struct counts_t {
    std::vector<std::array<COUNT_T, 5>> pos_counts;                                    // position base counts
    std::unordered_map<uint32_t, std::unordered_map<std::string, COUNT_T>> ins_counts; // insertion counts
};

// compute position and insertion counts
counts_t compute_counts(const char* const in_reads_fn, const char* const in_ref_fn, uint8_t const min_qual);

// write pos_counts as TSV file
void write_pos_counts_tsv(std::vector<std::array<COUNT_T, 5>> const & pos_counts, std::ostream & out_file, char delim='\t');
void write_pos_counts_tsv(std::vector<std::array<COUNT_T, 5>> const & pos_counts, const char* const out_fn, char delim='\t');

// write ins_counts as JSON file
void write_ins_counts_json(std::unordered_map<uint32_t, std::unordered_map<std::string, COUNT_T>> & ins_counts, std::ostream & out_file);
void write_ins_counts_json(std::unordered_map<uint32_t, std::unordered_map<std::string, COUNT_T>> & ins_counts, const char* const out_fn);
#endif
