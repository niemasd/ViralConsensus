#include "argparse.h"
#include "count.h"
#include "fasta.h"
#include "primer.h"

// main function
int main(int argc, char** argv) {
    args_t user_args = parse_args(argc, argv);
    std::string ref = read_fasta(user_args.in_ref_fn);
    std::vector<std::pair<uint32_t, uint32_t>> min_max_primer_inds = find_overlapping_primers(ref.length(), user_args.primer_bed_fn, user_args.primer_offset);
    counts_t counts = compute_counts(user_args.in_reads_fn, ref, user_args.min_qual, min_max_primer_inds);
    std::string consensus = compute_consensus(counts.pos_counts, counts.ins_counts, user_args);
    write_consensus_fasta(consensus, user_args);
    write_ins_counts_json(counts.ins_counts, user_args.out_ins_counts_fn);
    write_pos_counts_tsv(counts.pos_counts, user_args.out_pos_counts_fn, '\t');
    return 0;
}
