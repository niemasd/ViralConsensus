#include <cstring>
#include <unistd.h>
#include "argparse.h"

args parse_args(int const argc, char** const argv) {
    // check for -h or --help
    for(int i = 1; i < argc; ++i) {
        if(strcmp(argv[i],"-h") == 0 || strcmp(argv[i],"--help") == 0) {
            print_usage(argv[0], std::cout); exit(0);
        }
    }

    // parse user args
    if(argc != 6) {
        print_usage(argv[0], std::cerr); exit(1);
    }
    args user_args;
    user_args.in_reads_fn = argv[1];
    user_args.in_ref_fn = argv[2];
    user_args.out_pos_counts_fn = argv[3];
    user_args.out_ins_counts_fn = argv[4];
    user_args.num_threads = atoi(argv[5]);
    if(user_args.num_threads == 0) {
        std::cerr << "Invalid number of threads: " << argv[5] << std::endl; exit(1);
    }
    return user_args;
}

void print_args(args const & user_args) {
    std::cout << "in_reads_fn: " << user_args.in_reads_fn << std::endl
              << "in_ref_fn: " << user_args.in_ref_fn << std::endl
              << "out_pos_counts_fn: " << user_args.out_pos_counts_fn << std::endl
              << "out_ins_counts_fn: " << user_args.out_ins_counts_fn << std::endl
              << "num_threads: " << user_args.num_threads << std::endl;
}

void print_usage(const char* const exe_name="viral_consensus_mp", std::ostream & out=std::cout) {
    out << "USAGE: " << exe_name << " <IN_READS> <IN_REF_GENOME> <OUT_POS_COUNTS> <OUT_INS_COUNTS> <THREADS>" << std::endl
        << "  - IN_READS         Input reads (CRAM/BAM/SAM)" << std::endl
        << "  - IN_REF_GENOME    Input reference genome (FASTA)" << std::endl
        << "  - OUT_POS_COUNTS   Output position counts" << std::endl
        << "  - OUT_INS_COUNTS   Output insertion counts (JSON)" << std::endl
        << "  - THREADS          Number of threads" << std::endl;
}
