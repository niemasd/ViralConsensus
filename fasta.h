#ifndef FASTA_H
#define FASTA_H
#include "common.h"
#include "argparse.h"

// load reference genome from FASTA
std::string read_fasta(const char* const in_ref_fn);

// write consensus genome as FASTA
void write_consensus_fasta(std::string const & consensus, args_t const & user_args);
#endif
