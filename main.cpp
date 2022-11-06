#include "argparse.h"
#include "count.h"
#include "fasta.h"

// main function
int main(int argc, char** argv) {
    args_t user_args = parse_args(argc, argv);
    counts_t counts = compute_counts(user_args.in_reads_fn, user_args.in_ref_fn, user_args.min_qual);
    std::string consensus = compute_consensus(counts.pos_counts, counts.ins_counts, user_args);
    write_consensus_fasta(consensus, user_args);
    write_ins_counts_json(counts.ins_counts, user_args.out_ins_counts_fn);
    write_pos_counts_tsv(counts.pos_counts, user_args.out_pos_counts_fn, '\t');
    return 0;
}
