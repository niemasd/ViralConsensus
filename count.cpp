#include "count.h"

void write_pos_counts_tsv(std::vector<std::array<COUNT_T, 5>> const & pos_counts, std::ostream & out_file, char delim) {
    out_file << "Pos" << delim << 'A' << delim << 'C' << delim << 'G' << delim << 'T' << delim << '-' << delim << "Total" << std::endl;
    long unsigned int const pos_counts_size = pos_counts.size(); COUNT_T row_sum;
    for(long unsigned int pos = 0; pos < pos_counts_size; ++pos) {
        out_file << pos; row_sum = 0;
        for(auto & val : pos_counts[pos]) {
            out_file << delim << val; row_sum += val;
        }
        out_file << delim << row_sum << std::endl;
    }
}

void write_pos_counts_tsv(std::vector<std::array<COUNT_T, 5>> const & pos_counts, const char* const out_fn, char delim) {
    if(strcmp(out_fn, "-")) {
        std::ofstream out_file(out_fn);
        write_pos_counts_tsv(pos_counts, out_file, delim);
        out_file.close();
    } else {
        write_pos_counts_tsv(pos_counts, std::cout, delim);
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

void write_ins_counts_json(std::unordered_map<uint32_t, std::unordered_map<std::string, COUNT_T>> & ins_counts, const char* const out_fn) {
    if(strcmp(out_fn, "-")) {
        std::ofstream out_file(out_fn);
        write_ins_counts_json(ins_counts, out_file);
        out_file.close();
    } else {
        write_ins_counts_json(ins_counts, std::cout);
    }
}

counts_t compute_counts(const char* const in_reads_fn, std::string const & ref, uint8_t const min_qual, std::vector<std::pair<uint32_t, uint32_t>> const & min_max_primer_inds) {
    // open reference FASTA file and CRAM/BAM/SAM file
    htsFile* reads = hts_open(in_reads_fn, "r");
    if(!reads) {
        std::cerr << "Failed to open file: " << in_reads_fn << std::endl; exit(1);
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
    bam1_t* src = bam_init1();                   // holds the current alignment record, which is read by sam_read1()
    int32_t ret;                                 // holds the return value of sam_read1()
    uint32_t k, i, pos, qpos, l, n_cigar;        // https://github.com/pysam-developers/pysam/blob/cb3443959ca0a4d93f646c078f31d5966c0b82eb/pysam/libcalignedsegment.pyx#L1985
    int32_t op;                                  // https://github.com/pysam-developers/pysam/blob/cb3443959ca0a4d93f646c078f31d5966c0b82eb/pysam/libcalignedsegment.pyx#L1986
    uint32_t* cigar_p;                           // https://github.com/pysam-developers/pysam/blob/cb3443959ca0a4d93f646c078f31d5966c0b82eb/pysam/libcalignedsegment.pyx#L1987
    bool is_reverse;                             // current read is reverse read
    uint32_t qlen;                               // current read length: https://gist.github.com/PoisonAlien/350677acc03b2fbf98aa#file-readbam-c-L28
    uint8_t* qseq_encoded;                       // current read sequence (4-bit encoded): https://gist.github.com/PoisonAlien/350677acc03b2fbf98aa#file-readbam-c-L30
    std::string qseq;                            // current read sequence
    uint8_t* qqual;                              // current read quality string
    uint8_t curr_base_qual;                      // current base quality
    uint8_t min_ins_qual;                        // minimum base quality in an insertion
    uint32_t tmp_uint32;                         // store current tmp_uint32 value
    std::string curr_ins;                        // current insertion
    bool primer_trim = !min_max_primer_inds.empty();

    // prepare helper iterator objects
    std::unordered_map<uint32_t, std::unordered_map<std::string, COUNT_T>>::iterator ins_counts_it;
    std::unordered_map<std::string, COUNT_T>::iterator ins_counts_pos_it;

    // reserve memory for various helper variabls (avoid resizing) and initialize pos counts
    counts.pos_counts.reserve(ref.length());
    counts.pos_counts.resize(ref.length(), {0,0,0,0,0});
    counts.ins_counts.reserve(ref.length());
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
        is_reverse = src->core.flag & BAM_FREVERSE;

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
                    if(curr_base_qual >= min_qual && (!primer_trim || (!is_reverse && pos >= min_max_primer_inds[src->core.pos].second) || (is_reverse && pos < min_max_primer_inds[src->core.pos].first))) {
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
                    ++counts.pos_counts[pos++][BASE_TO_NUM[(int)'-']];
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

std::string compute_consensus(std::vector<std::array<COUNT_T, 5>> const & pos_counts, std::unordered_map<uint32_t, std::unordered_map<std::string, COUNT_T>> & ins_counts, args_t const & user_args) {
    std::string out; out.reserve(FASTA_STRING_RESERVE);
    long unsigned int const pos_counts_size = pos_counts.size();
    std::unordered_map<uint32_t, std::unordered_map<std::string, COUNT_T>>::iterator ins_counts_it;
    char best_pos_base; const std::string* best_ins_seq; double best_count; double tot_depth;
    for(long unsigned int pos = 0; pos <= pos_counts_size; ++pos) {
        // handle insertions before pos
        ins_counts_it = ins_counts.find(pos);
        if(ins_counts_it != ins_counts.end()) {
            best_count = 0; tot_depth = 0;
            for(auto curr_ins : ins_counts_it->second) {
                tot_depth += curr_ins.second;
                if(curr_ins.second > best_count) {
                    best_count = curr_ins.second; best_ins_seq = &(curr_ins.first);
                }
            }
            if(tot_depth >= user_args.min_depth && (best_count/tot_depth) > user_args.min_freq) {
                out += (*best_ins_seq);
            }
        }

        // handle pos
        if(pos != pos_counts_size) {
            best_count = 0; tot_depth = pos_counts[pos][0] + pos_counts[pos][1] + pos_counts[pos][2] + pos_counts[pos][3] + pos_counts[pos][4];
            if(pos_counts[pos][0] > best_count) {
                best_count = pos_counts[pos][0]; best_pos_base = 'A';
            }
            if(pos_counts[pos][1] > best_count) {
                best_count = pos_counts[pos][1]; best_pos_base = 'C';
            }
            if(pos_counts[pos][2] > best_count) {
                best_count = pos_counts[pos][2]; best_pos_base = 'G';
            }
            if(pos_counts[pos][3] > best_count) {
                best_count = pos_counts[pos][3]; best_pos_base = 'T';
            }
            if(pos_counts[pos][4] > best_count) {
                best_count = pos_counts[pos][4]; best_pos_base = '-';
            }
            if(tot_depth >= user_args.min_depth && (best_count/tot_depth) > user_args.min_freq) {
                if(best_pos_base != '-') {
                    out += best_pos_base;
                }
            } else {
                out += user_args.ambig;
            }
        }
    }
    return out;
}
