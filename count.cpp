#include "count.h"
#include "fasta.h"
#include <iostream>

void print_pos_counts(std::vector<std::array<COUNT_T, 5>> const & pos_counts, char delim='\t') {
    for(auto & row : pos_counts) {
        for(auto & val : row) {
            std::cout << val << delim;
        }
        std::cout << std::endl;
    }
}

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
    uint8_t* qseq_encoded;                       // current read sequence (4-bit encoded): https://gist.github.com/PoisonAlien/350677acc03b2fbf98aa#file-readbam-c-L30
    std::string qseq;                            // current read sequence
    uint8_t* qqual;                              // current read quality string
    uint32_t pos_plus_l;                         // store current pos_plus_l value
    int q_alignment_start;                       // index of query where the alignment starts (inclusive)
    int q_alignment_end;                         // index of query where the alignment ends (exclusive)
    unsigned int DUMMY_COUNT = 0; // TODO delete

    // reserve memory for various helper variabls (avoid resizing) and initialize pos counts
    counts.pos_counts.reserve(ref.length());
    counts.pos_counts.resize(ref.length(), {0,0,0,0,0});
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
        qseq_encoded = bam_get_seq(src);
        qqual = bam_get_qual(src);
        insertion_start_inds.clear();
        insertion_end_inds.clear();
        deletion_start_inds.clear();
        deletion_end_inds.clear();
        q_alignment_start = -1;
        q_alignment_end = qlen;
        for(k = n_cigar-1; k >= 1; --k) { // https://github.com/pysam-developers/pysam/blob/master/pysam/libcalignedsegment.pyx#L519-L527
            op = cigar_p[k] & BAM_CIGAR_MASK;
            if(op == BAM_CSOFT_CLIP) {
                q_alignment_end -= (cigar_p[k] >> BAM_CIGAR_SHIFT);
            } else if(op != BAM_CHARD_CLIP) {
                break;
            }
        }

        // load read sequence
        qseq.clear();
        for(i = 0; i < qlen; ++i) { // https://gist.github.com/PoisonAlien/350677acc03b2fbf98aa#file-readbam-c-L36-L38
            qseq.push_back(seq_nt16_str[bam_seqi(qseq_encoded, i)]);
        }
        //if(bam_is_rev(src)) { // reverse strand
        //    rev_comp_inplace(qseq);
        //}

        // iterate over aligned pairs: https://github.com/pysam-developers/pysam/blob/cb3443959ca0a4d93f646c078f31d5966c0b82eb/pysam/libcalignedsegment.pyx#L2007-L2064
        for(k = 0; k < n_cigar; ++k) {
            op = cigar_p[k] & BAM_CIGAR_MASK;
            l = cigar_p[k] >> BAM_CIGAR_SHIFT;

            // if soft-clipped, skip
            if(op == BAM_CSOFT_CLIP) {
                qpos += l;
            }

            else if(op == BAM_CREF_SKIP) {
                std::cerr << "BAM_CREF_SKIP operation in CIGAR string not currently supported" << std::endl; exit(1);
            }

            // don't allow BAM_CPAD for now (I don't know what it is)
            else if(op == BAM_CPAD) {
                std::cerr << "BAM_CPAD operation in CIGAR string not currently supported" << std::endl; exit(1);
            }

            // handle match/mismatch: https://github.com/pysam-developers/pysam/blob/cb3443959ca0a4d93f646c078f31d5966c0b82eb/pysam/libcalignedsegment.pyx#L2014-L2024
            else if(op == BAM_CMATCH || op == BAM_CEQUAL || op == BAM_CDIFF) {
                pos_plus_l = pos + l;
                while(pos < pos_plus_l) {
                    if(q_alignment_start == -1) {
                        q_alignment_start = qpos;
                    }
                    if(qqual[qpos] >= min_qual) {
                        ++counts.pos_counts[pos][BASE_TO_NUM[(int)qseq[qpos]]];
                    }
                    ++qpos; ++pos;
                }
            }

            // handle insertion: https://github.com/pysam-developers/pysam/blob/cb3443959ca0a4d93f646c078f31d5966c0b82eb/pysam/libcalignedsegment.pyx#L2026-L2037
            else if(op == BAM_CINS) {
                for(i = 0; i < l; ++l) {
                    if(q_alignment_start == -1 && op == BAM_CINS) {
                        q_alignment_start = qpos;
                    }
                    // TODO RESULT IS: (qpos, None, None)
                    ++qpos;
                }
            }

            // handle deletion: https://github.com/pysam-developers/pysam/blob/cb3443959ca0a4d93f646c078f31d5966c0b82eb/pysam/libcalignedsegment.pyx#L2039-L2050
            else if(op == BAM_CDEL) {
                pos_plus_l = pos + l;
                while(pos < pos_plus_l) {
                    // TODO RESULT IS: (None, i, ref_seq[r_idx])
                    ++pos;
                }
            }
        }

        ++DUMMY_COUNT; // TODO delete
    }
    std::cout << "Number of reads: " << DUMMY_COUNT << std::endl; // TODO DELETE
    print_pos_counts(counts.pos_counts);
    return counts;
}
