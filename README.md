# ViralConsensus
Fast viral consensus genome reconstruction

# Installation
ViralConsensus is written in C++ and depends on htslib. You can simply download the latest release tarball (or clone the repo) and compile with `make`:

```bash
git clone https://github.com/niemasd/ViralConsensus.git
cd ViralConsensus
make
sudo mv viral_consensus /usr/local/bin/ # optional; install NGG executables globally
```

You can also find a Docker container of NGG on DockerHub: TODO

# Usage
```
viral_consensus -i IN_READS -r REF_GENOME -o OUT_CONSENSUS [-op OUT_POS_COUNTS] [-oi OUT_INS_COUNTS] [-q MIN_QUAL] [-d MIN_DEPTH] [-f MIN_FREQ] [-a AMBIG] [-p PRIMER_BED] [-po PRIMER_OFFSET]
  -i/--in_reads IN_READS                Input reads file (CRAM/BAM/SAM), or '-' for standard input
  -r/--ref_genome REF_GENOME            Input reference genome (FASTA)
  -o/--out_consensus OUT_CONSENSUS      Output consensus genome (FASTA), or '-' for standard output
  -op/--out_pos_counts OUT_POS_COUNTS   Output position counts (TSV), or '-' for standard output (default: don't output)
  -oi/--out_ins_counts OUT_INS_COUNTS   Output insertion counts (JSON), or '-' for standard output (default: don't output)
  -q/--min_qual MIN_QUAL                Minimum base quality to count base in counts (default: 20)
  -d/--min_depth MIN_DEPTH              Minimum depth to call base/insertion in consensus (default: 10)
  -f/--min_freq MIN_FREQ                Minimum frequency to call base/insertion in consensus (default: 0.5)
  -a/--ambig AMBIG                      Ambiguous symbol (default: N)
  -p/--primer_bed PRIMER_BED            Primer file (BED)
  -po/--primer_offset PRIMER_OFFSET     Number of bases after primer to also trim (default: 0)
  -v/--version                          Print version number
  -h/--help                             Print this usage message
```

# Citing CoaTran
If you use ViralConsensus in your work, please cite:

> https://github.com/niemasd/ViralConsensus
