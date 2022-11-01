#include "argparse.h"
#include "count.h"

// main function
int main(int argc, char** argv) {
    args_t user_args = parse_args(argc, argv);
    counts_t counts = compute_counts(user_args.in_reads_fn, user_args.in_ref_fn);
    return 0;
}
