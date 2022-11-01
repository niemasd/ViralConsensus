#ifndef ARGPARSE_H
#define ARGPARSE_H
#include <cstdint>
#include <iostream>

// struct to store program args
struct args_t {
    char* in_reads_fn;       // input reads filename (CRAM/BAM/SAM)
    char* in_ref_fn;         // input reference genome filename (FASTA)
    char* out_pos_counts_fn; // output position counts filename
    char* out_ins_counts_fn; // output insertion counts filename (JSON)
    int num_threads;         // number of threads
};

// parse user args
args_t parse_args(int const argc, char** const argv);

// print user args (for debugging)
void print_args(args_t const & user_args);

// print usage
void print_usage(const char* const exe_name, std::ostream & out);
#endif
