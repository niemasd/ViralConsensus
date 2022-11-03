#include "count.h"
#include "fasta.h"
#include <iostream>

counts_t compute_counts(const char* const in_reads_fn, const char* const in_ref_fn) {
    // open reference FASTA file and CRAM/BAM/SAM file
    std::string ref = read_fasta(in_ref_fn);
    std::cout << ref << std::endl;
    htsFile* reads = hts_open(in_reads_fn, "r");
    if(!reads) {
        exit(1); // failed to open file
    } else if(reads->format.format != sam && reads->format.format != bam && reads->format.format != cram) {
        std::cerr << "Not a CRAM/BAM/SAM file: " << in_reads_fn << std::endl; exit(1);
    }
    // TODO ADD CRAM REFERENCE SUPPORT (hts_set_opt?)
    // https://github.com/pysam-developers/pysam/commit/3e150c284baba10fdd8ce5feb217d2ee25d24183
    // https://github.com/samtools/htslib/blob/36312fb0a06bd59188fd39a860055fbb4dd0dc63/htslib/hts.h

    // set up htsFile for parsing
    bam_hdr_t* header = sam_hdr_read(reads);
    if(!header) {
        std::cerr << "Unable to open CRAM/BAM/SAM header: " << in_reads_fn << std::endl; exit(1);
    } else if(header->n_targets != 1) {
        std::cerr << "CRAM/BAM/SAM has " << header->n_targets << " references, but it should have exactly 1: " << in_reads_fn << std::endl; exit(1);
    }

    // compute counts
    bam1_t* aln = bam_init1(); int ret;
    while(true) {
        // read next alignment record (aln)
        ret = sam_read1(reads, header, aln);
        if(ret == -1) {
            break;
        } else if(ret < -1) {
            std::cerr << "Error reading file: " << in_reads_fn << std::endl; exit(1);
        }

        // iterate over aligned pairs
        // TODO: https://github.com/pysam-developers/pysam/blob/cb3443959ca0a4d93f646c078f31d5966c0b82eb/pysam/libcalignedsegment.pyx#L1958-L2066
    }
    counts_t counts;
    return counts;
}
