#include "htslib/htslib/sam.h"
#include <omp.h>
#include "argparse.h"

// main function
int main(int argc, char** argv) {
    args user_args = parse_args(argc, argv);
    print_args(user_args);
    return 0;
}
