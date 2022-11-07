#include "primer.h"

std::vector<std::pair<int32_t, int32_t>> read_bed(const char* const primer_bed_fn) {
    std::vector<std::pair<int32_t, int32_t>> primers;
    if(!primer_bed_fn) {
        return primers;
    }
    primers.reserve(FASTA_STRING_RESERVE);
    std::ifstream bed_file(primer_bed_fn);
    std::string line; std::string tmp;
    int32_t start;
    while(getline(bed_file, line)) {
        std::istringstream ss(line);
        getline(ss, tmp, '\t'); // chrom
        getline(ss, tmp, '\t'); // start
        start = stoi(tmp);
        getline(ss, tmp, '\t'); // end
        primers.push_back(std::make_pair(start, stoi(tmp)));
    }
    std::sort(primers.begin(), primers.end()); // shouldn't be necessary (BED should be sorted), but just in case
    return primers;
}

void print_primers(std::vector<std::pair<int32_t, int32_t>> const & primers) {
    for(std::pair<int32_t, int32_t> const & curr : primers) {
        std::cout << curr.first << '\t' << curr.second << std::endl;
    }
}

std::vector<std::pair<int32_t, int32_t>> find_overlapping_primers(int32_t const ref_len, const char* const primer_bed_fn, int16_t primer_offset) {
    std::vector<std::pair<int32_t, int32_t>> min_max_primer_inds;
    if(!primer_bed_fn) {
        return min_max_primer_inds;
    }
    min_max_primer_inds.resize(ref_len, std::make_pair(-1,-1));
    std::vector<std::pair<int32_t, int32_t>> primers = read_bed(primer_bed_fn);
    unsigned long const NUM_PRIMERS = primers.size();
    std::list<const std::pair<int32_t, int32_t>*> curr_primers;
    unsigned long primers_ind = 0;
    for(int32_t pos = 0; pos < ref_len; ++pos) {
        while(!curr_primers.empty() && pos >= curr_primers.front()->second + primer_offset) {
            curr_primers.pop_front();
        }
        while(primers_ind < NUM_PRIMERS && pos >= primers[primers_ind].first - primer_offset) {
            curr_primers.push_back(&primers[primers_ind++]);
        }
        if(!curr_primers.empty()) {
            for(const std::pair<int32_t, int32_t>* const curr : curr_primers) {
                if(min_max_primer_inds[pos].first == -1 || curr->first < min_max_primer_inds[pos].first) {
                    min_max_primer_inds[pos].first = curr->first;
                }
                if(curr->second > min_max_primer_inds[pos].second) {
                    min_max_primer_inds[pos].second = curr->second;
                }
            }
        }
    }
    return min_max_primer_inds;
}
