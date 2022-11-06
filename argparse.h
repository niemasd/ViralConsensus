#ifndef ARGPARSE_H
#define ARGPARSE_H
#include "common.h"

// struct to store program args
struct args_t {
    char* in_reads_fn = nullptr;               // input reads filename (CRAM/BAM/SAM)
    char* in_ref_fn = nullptr;                 // input reference genome filename (FASTA)
    char* out_pos_counts_fn = nullptr;         // output position counts filename
    char* out_ins_counts_fn = nullptr;         // output insertion counts filename (JSON)
    int16_t num_threads = DEFAULT_NUM_THREADS; // number of threads
    uint8_t min_qual = DEFAULT_MIN_QUAL;       // minimum base quality to count base in counts
    COUNT_T min_depth = DEFAULT_MIN_DEPTH;     // minimum depth to call base in consensus
    double min_freq = DEFAULT_MIN_FREQ;        // minimum frequency to call base in consensus
    char ambig = DEFAULT_AMBIG;                // ambiguous symbol
};

// parse user args
args_t parse_args(int const argc, char** const argv);

// check user args
void check_args(args_t const & user_args);

// print user args (for debugging)
void print_args(args_t const & user_args);

// print usage
void print_usage(const char* const exe_name, std::ostream & out);
#endif
