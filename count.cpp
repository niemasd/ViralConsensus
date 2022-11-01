#include "count.h"
#include <iostream>

counts_t compute_counts(const char* const in_reads_fn, const char* const in_ref_fn) {
    // open CRAM/BAM/SAM file
    htsFile* reads = hts_open(in_reads_fn, "r");
    if(!reads) {
        exit(1); // failed to open file
    } else if(reads->format.format != sam && reads->format.format != bam && reads->format.format != cram) {
        std::cerr << "Not a CRAM/BAM/SAM file: " << in_reads_fn << std::endl; exit(1);
    }

    // set up htsFile for parsing
    bam_hdr_t* header = sam_hdr_read(reads);
    if(!header) {
        std::cerr << "Unable to open CRAM/BAM/SAM header: " << in_reads_fn << std::endl; exit(1);
    } else if(header->n_targets != 1) {
        std::cerr << "CRAM/BAM/SAM has " << header->n_targets << " references, but it should have exactly 1: " << in_reads_fn << std::endl; exit(1);
    }

    // compute counts
    bam1_t* aln = bam_init1(); int ret;
    unsigned int count = 0; // TODO DELETE; just for testing
    while(true) {
        // read next alignment record (aln)
        ret = sam_read1(reads, header, aln);
        if(ret == -1) {
            break;
        } else if(ret < -1) {
            std::cerr << "Error reading file: " << in_reads_fn << std::endl; exit(1);
        }
        ++count; // TODO DELETE; just for testing
    }
    std::cout << "Number of reads: " << count << std::endl; // TODO DELETE; just for testing
    counts_t counts;
    return counts;
}
