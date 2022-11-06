#include "argparse.h"
#include <array>
#include <cstring>
#include <unistd.h>

args_t parse_args(int const argc, char** const argv) {
    // check for -h or --help first
    int i;
    for(i = 1; i < argc; ++i) {
        for(const char* const & s : HELP_ARG_STRINGS) {
            if(strcmp(argv[i], s) == 0) {
                print_usage(argv[0], std::cout); exit(0);
            }
        }
    }

    // parse user args
    args_t user_args;
    for(i = 1; i < argc; ++i) {
        if(strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--in_reads") == 0) {
            if(++i == argc) {
                std::cerr << "Argument -i/--in_reads expected 1 argument" << std::endl; exit(1);
            }
            user_args.in_reads_fn = argv[i];
        } else if(strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--ref_genome") == 0) {
            if(++i == argc) {
                std::cerr << "Argument -r/--ref_genome expected 1 argument" << std::endl; exit(1);
            }
            user_args.in_ref_fn = argv[i];
        } else if(strcmp(argv[i], "-op") == 0 || strcmp(argv[i], "--out_pos_counts") == 0) {
            if(++i == argc) {
                std::cerr << "Argument -op/--out_pos_counts expected 1 argument" << std::endl; exit(1);
            }
            user_args.out_pos_counts_fn = argv[i];
        } else if(strcmp(argv[i], "-oi") == 0 || strcmp(argv[i], "--out_ins_counts") == 0) {
            if(++i == argc) {
                std::cerr << "Argument -oi/--out_ins_counts expected 1 argument" << std::endl; exit(1);
            }
            user_args.out_ins_counts_fn = argv[i];
        } else if(strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--threads") == 0) {
            if(++i == argc) {
                std::cerr << "Argument -t/--threads expected 1 argument" << std::endl; exit(1);
            }
            user_args.num_threads = atoi(argv[i]);
            if(user_args.num_threads < 1) {
                std::cerr << "Invalid number of threads: " << argv[i] << std::endl; exit(1);
            }
        } else if(strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--min_qual") == 0) {
            if(++i == argc) {
                std::cerr << "Argument -q/--min_qual expected 1 argument" << std::endl; exit(1);
            }
            user_args.min_qual = atoi(argv[i]);
            if(user_args.min_qual < 1) {
                std::cerr << "Invalid minimum base quality: " << argv[1] << std::endl; exit(1);
            }
        } else {
            std::cerr << "Invalid argument: " << argv[i] << std::endl; print_usage(argv[0], std::cerr); exit(1);
        }
    }

    // check user args and return if valid
    check_args(user_args);
    return user_args;
}

void check_args(args_t const & user_args) {
    if(!user_args.in_reads_fn) {
        std::cerr << MESSAGE_MISSING_REQUIRED_ARG << "-i/--in_reads" << std::endl; exit(1);
    } else if(!user_args.in_ref_fn) {
        std::cerr << MESSAGE_MISSING_REQUIRED_ARG << "-r/--ref_genome" << std::endl; exit(1);
    } else if(!user_args.out_pos_counts_fn) {
        std::cerr << MESSAGE_MISSING_REQUIRED_ARG << "-op/--out_pos_counts" << std::endl; exit(1);
    } else if(!user_args.out_ins_counts_fn) {
        std::cerr << MESSAGE_MISSING_REQUIRED_ARG << "-oi/--out_ins_counts" << std::endl; exit(1);
    }
}

void print_args(args_t const & user_args) {
    std::cout << "in_reads_fn: " << user_args.in_reads_fn << std::endl
              << "in_ref_fn: " << user_args.in_ref_fn << std::endl
              << "out_pos_counts_fn: " << user_args.out_pos_counts_fn << std::endl
              << "out_ins_counts_fn: " << user_args.out_ins_counts_fn << std::endl
              << "num_threads: " << user_args.num_threads << std::endl
              << "min_qual: " << user_args.min_qual << std::endl;
}

void print_usage(const char* const exe_name="viral_consensus_mp", std::ostream & out=std::cout) {
    out << "USAGE: " << exe_name << " -i IN_READS -r REF_GENOME -op OUT_POS_COUNTS -oi OUT_INS_COUNTS [-t THREADS] [-q MIN_QUAL]" << std::endl
        << "  -i/--in_reads IN_READS                Input reads file (CRAM/BAM/SAM), or '-' for standard input" << std::endl
        << "  -r/--ref_genome REF_GENOME            Input reference genome (FASTA)" << std::endl
        << "  -op/--out_pos_counts OUT_POS_COUNTS   Output position counts (TSV), or '-' for standard output" << std::endl
        << "  -oi/--out_ins_counts OUT_INS_COUNTS   Output insertion counts (JSON), or '-' for standard output" << std::endl
        << "  -t/--threads THREADS                  Number of threads (default: " << DEFAULT_NUM_THREADS << ")" << std::endl
        << "  -q/--min_qual MIN_QUAL                Minimum base quality (default: " << DEFAULT_MIN_QUAL << ")" << std::endl
        << "  -h/--help                             Print this usage message" << std::endl;
}
