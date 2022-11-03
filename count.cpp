#include "count.h"
#include "fasta.h"
#include <iostream>

counts_t compute_counts(const char* const in_reads_fn, const char* const in_ref_fn, int const min_qual=DEFAULT_MIN_QUAL) {
    // open reference FASTA file and CRAM/BAM/SAM file
    std::string ref = read_fasta(in_ref_fn);
    htsFile* reads = hts_open(in_reads_fn, "r");
    if(!reads) {
        exit(1); // failed to open file
    } else if(reads->format.format != sam && reads->format.format != bam && reads->format.format != cram) {
        std::cerr << "Not a CRAM/BAM/SAM file: " << in_reads_fn << std::endl; exit(1);
    }
    // TODO ADD CRAM REFERENCE SUPPORT (hts_set_opt?)
    // might not actually be necessary since I have the reference genome in memory
    // https://github.com/pysam-developers/pysam/commit/3e150c284baba10fdd8ce5feb217d2ee25d24183
    // https://github.com/samtools/htslib/blob/36312fb0a06bd59188fd39a860055fbb4dd0dc63/htslib/hts.h

    // set up htsFile for parsing
    bam_hdr_t* header = sam_hdr_read(reads);
    if(!header) {
        std::cerr << "Unable to open CRAM/BAM/SAM header: " << in_reads_fn << std::endl; exit(1);
    } else if(header->n_targets != 1) {
        std::cerr << "CRAM/BAM/SAM has " << header->n_targets << " references, but it should have exactly 1: " << in_reads_fn << std::endl; exit(1);
    }

    // prepare helper variables for computing counts
    counts_t counts; counts.pos_counts.reserve(ref.length()); // counts_t object to store the counts themselves
    bam1_t* src = bam_init1();                                // holds the current alignment record, which is read by sam_read1()
    int ret;                                                  // holds the return value of sam_read1()
    uint32_t k, i, pos, qpos, r_idx, l, n_cigar;              // https://github.com/pysam-developers/pysam/blob/cb3443959ca0a4d93f646c078f31d5966c0b82eb/pysam/libcalignedsegment.pyx#L1985
    int op;                                                   // https://github.com/pysam-developers/pysam/blob/cb3443959ca0a4d93f646c078f31d5966c0b82eb/pysam/libcalignedsegment.pyx#L1986
    uint32_t* cigar_p;                                        // https://github.com/pysam-developers/pysam/blob/cb3443959ca0a4d93f646c078f31d5966c0b82eb/pysam/libcalignedsegment.pyx#L1987
    unsigned int DUMMY_COUNT = 0; // TODO delete

    // compute counts
    while(true) {
        // read next alignment record
        ret = sam_read1(reads, header, src);
        if(ret == -1) {
            break;
        } else if(ret < -1) {
            std::cerr << "Error reading file: " << in_reads_fn << std::endl; exit(1);
        }

        // if this CIGAR is empty, skip
        n_cigar = src->core.n_cigar;
        if(n_cigar == 0) { continue; }

        // iterate over aligned pairs: https://github.com/pysam-developers/pysam/blob/cb3443959ca0a4d93f646c078f31d5966c0b82eb/pysam/libcalignedsegment.pyx#L2007-L2064
        pos = src->core.pos;
        qpos = 0;
        cigar_p = bam_get_cigar(src);
        for(k = 0; k < n_cigar; ++k) {
            op = cigar_p[k] & BAM_CIGAR_MASK;
            l = cigar_p[k] >> BAM_CIGAR_SHIFT;

            // handle match/mismatch: https://github.com/pysam-developers/pysam/blob/cb3443959ca0a4d93f646c078f31d5966c0b82eb/pysam/libcalignedsegment.pyx#L2014-L2024
            if(op == BAM_CMATCH || op == BAM_CEQUAL || op == BAM_CDIFF) {
                for(i = pos; i < pos + l; ++i) {
                    // TODO RESULT IS: (qpos, i, ref_seq[r_idx])
                    ++r_idx; ++qpos;
                }
                ++pos;
            }

            // handle insertion: https://github.com/pysam-developers/pysam/blob/cb3443959ca0a4d93f646c078f31d5966c0b82eb/pysam/libcalignedsegment.pyx#L2026-L2037
            else if(op == BAM_CINS || op == BAM_CSOFT_CLIP || op == BAM_CPAD) {
                for(i = pos; i < pos + l; ++i) {
                    // TODO RESULT IS: (qpos, None, None)
                    ++qpos;
                }
            }

            // handle deletion: https://github.com/pysam-developers/pysam/blob/cb3443959ca0a4d93f646c078f31d5966c0b82eb/pysam/libcalignedsegment.pyx#L2039-L2050
            else if(op == BAM_CDEL) {
                for(i = pos; i < pos + l; ++i) {
                    // TODO RESULT IS: (None, i, ref_seq[r_idx])
                    ++r_idx;
                }
                ++pos;
            }
        }

        ++DUMMY_COUNT; // TODO delete
    }
    std::cout << "Number of reads: " << DUMMY_COUNT << std::endl; // TODO DELETE
    return counts;
}
