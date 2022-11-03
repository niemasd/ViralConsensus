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

    // set up htsFile for parsing
    bam_hdr_t* header = sam_hdr_read(reads);
    if(!header) {
        std::cerr << "Unable to open CRAM/BAM/SAM header: " << in_reads_fn << std::endl; exit(1);
    } else if(header->n_targets != 1) {
        std::cerr << "CRAM/BAM/SAM has " << header->n_targets << " references, but it should have exactly 1: " << in_reads_fn << std::endl; exit(1);
    }

    // prepare helper variables for computing counts
    counts_t counts;                             // counts_t object to store the counts themselves
    std::vector<int> insertion_start_inds;       // start indices of insertions
    std::vector<int> insertion_end_inds;         // end indices of insertions
    std::vector<int> deletion_start_inds;        // start indices of deletions
    std::vector<int> deletion_end_inds;          // end indices of deletions
    bam1_t* src = bam_init1();                   // holds the current alignment record, which is read by sam_read1()
    int ret;                                     // holds the return value of sam_read1()
    uint32_t k, i, pos, qpos, l, n_cigar;        // https://github.com/pysam-developers/pysam/blob/cb3443959ca0a4d93f646c078f31d5966c0b82eb/pysam/libcalignedsegment.pyx#L1985
    int op;                                      // https://github.com/pysam-developers/pysam/blob/cb3443959ca0a4d93f646c078f31d5966c0b82eb/pysam/libcalignedsegment.pyx#L1986
    uint32_t* cigar_p;                           // https://github.com/pysam-developers/pysam/blob/cb3443959ca0a4d93f646c078f31d5966c0b82eb/pysam/libcalignedsegment.pyx#L1987
    uint32_t qlen;                               // current read length: https://gist.github.com/PoisonAlien/350677acc03b2fbf98aa#file-readbam-c-L28
    uint8_t* qual_s;                             // current read quality string: https://gist.github.com/PoisonAlien/350677acc03b2fbf98aa#file-readbam-c-L30
    std::string qseq;                            // current read sequence
    unsigned int DUMMY_COUNT = 0; // TODO delete

    // reserve memory for various helper variabls (avoid resizing)
    counts.pos_counts.reserve(ref.length());
    counts.ins_counts.reserve(ref.length());
    insertion_start_inds.reserve(ref.length());
    insertion_end_inds.reserve(ref.length());
    deletion_start_inds.reserve(ref.length());
    deletion_end_inds.reserve(ref.length());
    qseq.reserve(READ_SEQUENCE_RESERVE);

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

        // prepare helper variables
        pos = src->core.pos;
        qpos = 0;
        cigar_p = bam_get_cigar(src);
        qlen = src->core.l_qseq;
        qual_s = bam_get_seq(src);

        // load read sequence
        qseq.clear();
        for(i = 0; i < qlen; ++i) {
            qseq.push_back(seq_nt16_str[bam_seqi(qual_s,i)]);
        }

        // iterate over aligned pairs: https://github.com/pysam-developers/pysam/blob/cb3443959ca0a4d93f646c078f31d5966c0b82eb/pysam/libcalignedsegment.pyx#L2007-L2064
        for(k = 0; k < n_cigar; ++k) {
            op = cigar_p[k] & BAM_CIGAR_MASK;
            l = cigar_p[k] >> BAM_CIGAR_SHIFT;

            // handle match/mismatch: https://github.com/pysam-developers/pysam/blob/cb3443959ca0a4d93f646c078f31d5966c0b82eb/pysam/libcalignedsegment.pyx#L2014-L2024
            if(op == BAM_CMATCH || op == BAM_CEQUAL || op == BAM_CDIFF) {
                for(i = pos; i < pos + l; ++i) {
                    // TODO RESULT IS: (qpos, i, ref_seq[r_idx])
                    ++qpos;
                }
                pos += l;
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
                }
                pos += l;
            }
        }

        ++DUMMY_COUNT; // TODO delete
    }
    std::cout << "Number of reads: " << DUMMY_COUNT << std::endl; // TODO DELETE
    return counts;
}
