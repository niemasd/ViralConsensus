#ifndef COUNT_H
#define COUNT_H
#include "common.h"
#include "argparse.h"

// struct to store counts
struct counts_t {
    std::vector<std::array<COUNT_T, 5>> pos_counts;                                    // position base counts
    std::unordered_map<uint32_t, std::unordered_map<std::string, COUNT_T>> ins_counts; // insertion counts
};

// compute position and insertion counts
counts_t compute_counts(const char* const in_reads_fn, std::string const & ref, uint8_t const min_qual, std::vector<std::pair<uint32_t, uint32_t>> const & min_max_primer_inds);

// compute consensus genome sequence from counts
std::string compute_consensus(std::vector<std::array<COUNT_T, 5>> const & pos_counts, std::unordered_map<uint32_t, std::unordered_map<std::string, COUNT_T>> & ins_counts, args_t const & user_args);

// write pos_counts as TSV file
void write_pos_counts_tsv(std::vector<std::array<COUNT_T, 5>> const & pos_counts, std::ostream & out_file, char delim='\t');
void write_pos_counts_tsv(std::vector<std::array<COUNT_T, 5>> const & pos_counts, const char* const out_fn, char delim='\t');

// write ins_counts as JSON file
void write_ins_counts_json(std::unordered_map<uint32_t, std::unordered_map<std::string, COUNT_T>> & ins_counts, std::ostream & out_file);
void write_ins_counts_json(std::unordered_map<uint32_t, std::unordered_map<std::string, COUNT_T>> & ins_counts, const char* const out_fn);
#endif
