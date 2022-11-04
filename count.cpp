#include "count.h"
#include "fasta.h"

void print_pos_counts(std::vector<std::array<COUNT_T, 5>> const & pos_counts, char delim='\t') {
    for(auto & row : pos_counts) {
        for(auto & val : row) {
            std::cout << val << delim;
        }
        std::cout << std::endl;
    }
}

void write_ins_counts_json(std::unordered_map<uint32_t, std::unordered_map<std::string, COUNT_T>> & ins_counts, std::ostream & out_file) {
    bool first1 = true, first2; out_file << "{";
    for(auto pair1 : ins_counts) {
        if(first1) {
            first1 = false;
        } else {
            out_file << ", ";
        }
        out_file << '"' << pair1.first << "\": {"; first2 = true;
        for(auto pair2 : pair1.second) {
            if(first2) {
                first2 = false;
            } else {
                out_file << ", ";
            }
            out_file << '"' << pair2.first << "\": " << pair2.second;
        }
        out_file << "}";
    }
    out_file << "}" << std::endl;
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
    //std::vector<int> deletion_start_inds;        // start indices of deletions
    //std::vector<int> deletion_end_inds;          // end indices of deletions
    bam1_t* src = bam_init1();                   // holds the current alignment record, which is read by sam_read1()
    int ret;                                     // holds the return value of sam_read1()
    uint32_t k, i, pos, qpos, l, n_cigar;        // https://github.com/pysam-developers/pysam/blob/cb3443959ca0a4d93f646c078f31d5966c0b82eb/pysam/libcalignedsegment.pyx#L1985
    int op;                                      // https://github.com/pysam-developers/pysam/blob/cb3443959ca0a4d93f646c078f31d5966c0b82eb/pysam/libcalignedsegment.pyx#L1986
    uint32_t* cigar_p;                           // https://github.com/pysam-developers/pysam/blob/cb3443959ca0a4d93f646c078f31d5966c0b82eb/pysam/libcalignedsegment.pyx#L1987
    uint32_t qlen;                               // current read length: https://gist.github.com/PoisonAlien/350677acc03b2fbf98aa#file-readbam-c-L28
    uint8_t* qseq_encoded;                       // current read sequence (4-bit encoded): https://gist.github.com/PoisonAlien/350677acc03b2fbf98aa#file-readbam-c-L30
    std::string qseq;                            // current read sequence
    uint8_t* qqual;                              // current read quality string
    int curr_base_qual;                          // current base quality
    int min_ins_qual;                            // minimum base quality in an insertion
    uint32_t tmp_uint32;                         // store current tmp_uint32 value
    std::string curr_ins;                        // current insertion
    //int q_alignment_start;                       // index of query where the alignment starts (inclusive)
    //int q_alignment_end;                         // index of query where the alignment ends (exclusive)

    // prepare helper iterator objects
    std::unordered_map<uint32_t, std::unordered_map<std::string, COUNT_T>>::iterator ins_counts_it;
    std::unordered_map<std::string, COUNT_T>::iterator ins_counts_pos_it;

    // reserve memory for various helper variabls (avoid resizing) and initialize pos counts
    counts.pos_counts.reserve(ref.length());
    counts.pos_counts.resize(ref.length(), {0,0,0,0,0});
    counts.ins_counts.reserve(ref.length());
    //deletion_start_inds.reserve(ref.length());
    //deletion_end_inds.reserve(ref.length());
    qseq.reserve(READ_SEQUENCE_RESERVE);
    curr_ins.reserve(INSERTION_RESERVE);

    // compute counts
    while(true) {
        // read next alignment record
        ret = sam_read1(reads, header, src);
        if(ret == -1) {
            break;
        } else if(ret < -1) {
            std::cerr << "Error reading file: " << in_reads_fn << std::endl; exit(1);
        }

        // if this read fails the required flags or this CIGAR is empty, skip
        n_cigar = src->core.n_cigar;
        if(src->core.flag & FAIL_FLAGS || n_cigar == 0) { continue; }

        // prepare helper variables
        pos = src->core.pos;
        qpos = 0;
        cigar_p = bam_get_cigar(src);
        qlen = src->core.l_qseq;
        qseq_encoded = bam_get_seq(src);
        qqual = bam_get_qual(src);
        curr_base_qual = -1;
        //deletion_start_inds.clear();
        //deletion_end_inds.clear();
        //q_alignment_start = -1;
        //q_alignment_end = qlen;
        for(k = n_cigar-1; k >= 1; --k) { // https://github.com/pysam-developers/pysam/blob/master/pysam/libcalignedsegment.pyx#L519-L527
            op = cigar_p[k] & BAM_CIGAR_MASK;
            if(op == BAM_CSOFT_CLIP) {
                //q_alignment_end -= (cigar_p[k] >> BAM_CIGAR_SHIFT);
            } else if(op != BAM_CHARD_CLIP) {
                break;
            }
        }

        // load read sequence
        qseq.clear();
        for(i = 0; i < qlen; ++i) { // https://gist.github.com/PoisonAlien/350677acc03b2fbf98aa#file-readbam-c-L36-L38
            qseq.push_back(seq_nt16_str[bam_seqi(qseq_encoded, i)]);
        }

        // iterate over aligned pairs: https://github.com/pysam-developers/pysam/blob/cb3443959ca0a4d93f646c078f31d5966c0b82eb/pysam/libcalignedsegment.pyx#L2007-L2064
        for(k = 0; k < n_cigar; ++k) {
            op = cigar_p[k] & BAM_CIGAR_MASK;
            l = cigar_p[k] >> BAM_CIGAR_SHIFT;

            // if soft-clipped, skip
            if(op == BAM_CSOFT_CLIP) {
                qpos += l;
            }

            // handle match/mismatch: https://github.com/pysam-developers/pysam/blob/cb3443959ca0a4d93f646c078f31d5966c0b82eb/pysam/libcalignedsegment.pyx#L2014-L2024
            else if(op == BAM_CMATCH || op == BAM_CEQUAL || op == BAM_CDIFF) {
                tmp_uint32 = pos + l;
                while(pos < tmp_uint32) {
                    curr_base_qual = qqual[qpos];
                    if(curr_base_qual >= min_qual) {
                        ++counts.pos_counts[pos][BASE_TO_NUM[(int)qseq[qpos]]];
                    }
                    ++qpos; ++pos;
                }
            }

            // handle insertion: https://github.com/pysam-developers/pysam/blob/cb3443959ca0a4d93f646c078f31d5966c0b82eb/pysam/libcalignedsegment.pyx#L2026-L2037
            else if(op == BAM_CINS) {
                tmp_uint32 = qpos + l;
                curr_ins.clear();
                min_ins_qual = qqual[qpos];
                while(qpos < tmp_uint32) {
                    curr_base_qual = qqual[qpos];
                    if(curr_base_qual < min_ins_qual) {
                        min_ins_qual = curr_base_qual;
                    }
                    curr_ins.push_back(qseq[qpos++]);
                }
                if(min_ins_qual >= min_qual) {
                    ins_counts_it = counts.ins_counts.find(pos);
                    if(ins_counts_it == counts.ins_counts.end()) {
                        ins_counts_it = counts.ins_counts.emplace(pos, std::unordered_map<std::string, COUNT_T>()).first;
                    }
                    ins_counts_pos_it = ins_counts_it->second.find(curr_ins);
                    if(ins_counts_pos_it == ins_counts_it->second.end()) {
                        ins_counts_pos_it = ins_counts_it->second.emplace(curr_ins, 0).first;
                    }
                    ++(ins_counts_pos_it->second);
                }
            }

            // handle deletion: https://github.com/pysam-developers/pysam/blob/cb3443959ca0a4d93f646c078f31d5966c0b82eb/pysam/libcalignedsegment.pyx#L2039-L2050
            else if(op == BAM_CDEL) {
                tmp_uint32 = pos + l;
                while(pos < tmp_uint32) {
                    // TODO RESULT IS: (None, i, ref_seq[r_idx])
                    ++pos;
                }
            }

            // don't allow BAM_CREF_SKIP for now (I don't know what it is)
            else if(op == BAM_CREF_SKIP) {
                std::cerr << "BAM_CREF_SKIP operation in CIGAR string not currently supported" << std::endl; exit(1);
            }

            // don't allow BAM_CPAD for now (I don't know what it is)
            else if(op == BAM_CPAD) {
                std::cerr << "BAM_CPAD operation in CIGAR string not currently supported" << std::endl; exit(1);
            }
        }

    }
    return counts;
}
