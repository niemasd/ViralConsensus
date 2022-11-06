#include "fasta.h"
#include <algorithm>
#include <fstream>
#include <iostream>

std::string read_fasta(const char* const in_ref_fn) {
    std::string ref; ref.reserve(FASTA_STRING_RESERVE);
    std::ifstream ref_file(in_ref_fn); std::string line; line.reserve(FASTA_STRING_RESERVE);
    bool seen_header = false;
    while(getline(ref_file, line)) {
        if(line.length() == 0) {    // skip empty lines
            continue;
        } else if(line[0] == '>') { // skip header (but verify just 1 header)
            if(seen_header) {
                std::cerr << "Reference genome FASTA must have exactly 1 sequence: " << in_ref_fn << std::endl; exit(1);
            } else {
                seen_header = true;
            }
        } else if(seen_header) {    // append to sequence
            line.erase(remove_if(line.begin(), line.end(), isspace), line.end()); // strip whitespace
            ref += line;
        } else {
            std::cerr << "Malformed FASTA file: " << in_ref_fn << std::endl; exit(1);
        }
    }
    if(ref.length() == 0) {
        if(seen_header) {
            std::cerr << "Malformed FASTA file: " << in_ref_fn << std::endl; exit(1);
        } else {
            std::cerr << "File not found: " << in_ref_fn << std::endl; exit(1);
        }
    }
    return ref;
}

void write_consensus_fasta(std::string const & consensus, args_t const & user_args) {
    std::cout << '>' << "SEQUENCE" << std::endl // TODO UPDATE FASTA ID
              << consensus << std::endl;
}
