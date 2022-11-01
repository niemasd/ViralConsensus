#include "htslib/htslib/sam.h"
#include <omp.h>
#include "argparse.h"

// main function
int main(int argc, char** argv) {
    // parse user args
    args_t user_args = parse_args(argc, argv);

    // parse input CRAM/BAM/SAM file
    htsFile* reads = hts_open(user_args.in_reads_fn, "r");
    return 0;
}
