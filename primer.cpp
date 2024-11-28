#include "primer.h"

std::pair<std::vector<std::pair<uint32_t,uint32_t>>, std::vector<std::pair<uint32_t,uint32_t>>> read_bed(const char* const primer_bed_fn) {
    // initialize forward and reverse primers
    std::vector<std::pair<uint32_t, uint32_t>> primers_forward;
    std::vector<std::pair<uint32_t, uint32_t>> primers_reverse;
    if(!primer_bed_fn) {
        return std::make_pair(primers_forward, primers_reverse);
    }
    primers_forward.reserve(FASTA_STRING_RESERVE);
    primers_reverse.reserve(FASTA_STRING_RESERVE);

    // read BED file
    std::ifstream bed_file(primer_bed_fn);
    std::string line; std::string tmp;
    uint32_t pos1; uint32_t pos2;
    while(getline(bed_file, line)) {
        // parse this line
        std::istringstream ss(line);
        getline(ss, tmp, '\t'); // chrom
        getline(ss, tmp, '\t'); // position 1 (start for forward primers, end for reverse primers)
        pos1 = stoi(tmp);
        getline(ss, tmp, '\t'); // position 2 (end for forward primers, start for reverse primers)
        pos2 = stoi(tmp);

        // add primer
        if(pos1 < pos2) {
            primers_forward.push_back(std::make_pair(pos1, pos2));
        } else {
            primers_reverse.push_back(std::make_pair(pos2, pos1));
        }
    }
    std::sort(primers_forward.begin(), primers_forward.end());
    std::sort(primers_reverse.begin(), primers_reverse.end());
    return std::make_pair(primers_forward, primers_reverse);
}

void print_primers(std::vector<std::pair<uint32_t, uint32_t>> const & primers) {
    for(std::pair<uint32_t, uint32_t> const & curr : primers) {
        std::cout << curr.first << '\t' << curr.second << std::endl;
    }
}

std::vector<std::pair<std::pair<uint32_t,uint32_t>, std::pair<uint32_t,uint32_t>>> find_overlapping_primers(uint32_t const ref_len, const char* const primer_bed_fn, uint16_t primer_offset) {
    // set up return data structure
    std::vector<std::pair<std::pair<uint32_t,uint32_t>, std::pair<uint32_t,uint32_t>>> min_max_primer_inds;
    if(!primer_bed_fn) {
        return min_max_primer_inds;
    }
    min_max_primer_inds.resize(ref_len, std::make_pair(std::make_pair((uint32_t)-1,(uint32_t)-1), std::make_pair((uint32_t)-1,(uint32_t)-1)));

    // load primers from file
    std::vector<std::pair<uint32_t,uint32_t>> primers_forward;
    std::vector<std::pair<uint32_t,uint32_t>> primers_reverse;
    std::tie(primers_forward, primers_reverse) = read_bed(primer_bed_fn);
    uint64_t const NUM_PRIMERS_FORWARD = primers_forward.size();
    uint64_t const NUM_PRIMERS_REVERSE = primers_reverse.size();

    // iterate over primers
    std::list<const std::pair<uint32_t, uint32_t>*> curr_primers_forward;
    std::list<const std::pair<uint32_t, uint32_t>*> curr_primers_reverse;
    uint64_t primers_ind_forward = 0;
    uint64_t primers_ind_reverse = 0;
    for(uint32_t pos = 0; pos < ref_len; ++pos) {
        // pop primers that are no longer covering the current position
        while(!curr_primers_forward.empty() && pos >= curr_primers_forward.front()->second + primer_offset) {
            curr_primers_forward.pop_front();
        }
        while(!curr_primers_reverse.empty() && pos >= curr_primers_reverse.front()->second + primer_offset) {
            curr_primers_reverse.pop_front();
        }

        // push primers that are now covering the current position
        while(primers_ind_forward < NUM_PRIMERS_FORWARD && pos >= primers_forward[primers_ind_forward].first - primer_offset) {
            curr_primers_forward.push_back(&primers_forward[primers_ind_forward++]);
        }
        while(primers_ind_reverse < NUM_PRIMERS_REVERSE && pos >= primers_reverse[primers_ind_reverse].first - primer_offset) {
            curr_primers_reverse.push_back(&primers_reverse[primers_ind_reverse++]);
        }

        // if there are primers covering the current position, get leftmost and rightmost positions of all primers covering this position
        if(!curr_primers_forward.empty()) {
            for(const std::pair<uint32_t, uint32_t>* const curr : curr_primers_forward) {
                if(min_max_primer_inds[pos].first.first == (uint32_t)-1 || curr->first < min_max_primer_inds[pos].first.first) {
                    min_max_primer_inds[pos].first.first = curr->first;
                }
                if(curr->second > min_max_primer_inds[pos].first.second) {
                    min_max_primer_inds[pos].first.second = curr->second;
                }
            }
        }
        if(!curr_primers_reverse.empty()) {
            for(const std::pair<uint32_t, uint32_t>* const curr : curr_primers_reverse) {
                if(min_max_primer_inds[pos].second.first == (uint32_t)-1 || curr->first < min_max_primer_inds[pos].second.first) {
                    min_max_primer_inds[pos].second.first = curr->first;
                }
                if(curr->second > min_max_primer_inds[pos].second.second) {
                    min_max_primer_inds[pos].second.second = curr->second;
                }
            }
        }
    }
    return min_max_primer_inds;
}
