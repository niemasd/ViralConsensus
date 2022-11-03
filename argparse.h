#ifndef ARGPARSE_H
#define ARGPARSE_H
#include "common.h"
#include <cstdint>
#include <iostream>

// struct to store program args
struct args_t {
    char* in_reads_fn = nullptr;           // input reads filename (CRAM/BAM/SAM)
    char* in_ref_fn = nullptr;             // input reference genome filename (FASTA)
    char* out_pos_counts_fn = nullptr;     // output position counts filename
    char* out_ins_counts_fn = nullptr;     // output insertion counts filename (JSON)
    int num_threads = DEFAULT_NUM_THREADS; // number of threads
    int min_qual = DEFAULT_MIN_QUAL;       // minimum base quality
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
