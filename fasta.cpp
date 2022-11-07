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
    std::cout << ">viral_consensus v" << VERSION << " (--in_reads " << user_args.in_reads_fn << " --ref_genome " << user_args.in_ref_fn << " --min_qual " << (int)user_args.min_qual << " --min_depth " << user_args.min_depth << " --min_freq " << user_args.min_freq << " --ambig " << user_args.ambig;
    if(user_args.primer_bed_fn) {
        std::cout << " --primer_bed " << user_args.primer_bed_fn << " --primer_offset " << user_args.primer_offset;
    }
    std::cout << ')' << std::endl << consensus << std::endl;
}
