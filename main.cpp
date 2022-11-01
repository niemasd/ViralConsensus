#include "htslib/htslib/sam.h"
#include <omp.h>
#include "argparse.h"

// main function
int main(int argc, char** argv) {
    args user_args = parse_args(argc, argv);
    htsFile* reads = hts_open(user_args.in_reads_fn, "r");
    return 0;
}
