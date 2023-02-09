# ViralConsensus
ViralConsensus is a fast and memory-efficient tool for calling viral consensus genome sequences directly from read alignment data. ViralConsensus is orders of magnitude faster and more memory-efficient than existing methods. Further, unlike existing methods, ViralConsensus can pipe data directly from a read mapper via standard input and performs viral consensus calling on-the-fly, making it an ideal tool for viral sequencing pipelines.

# Installation
ViralConsensus is written in C++ and depends on htslib. You can simply download the latest release tarball (or clone the repo) and compile with `make`:

```bash
git clone https://github.com/niemasd/ViralConsensus.git
cd ViralConsensus
make
sudo mv viral_consensus /usr/local/bin/ # optional; install NGG executables globally
```

You can also find a Docker container of ViralConsensus on DockerHub: [niemasd/viral_consensus](https://hub.docker.com/r/niemasd/viral_consensus)

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

## Piping from Read Mapper
Because ViralConsensus does not require reads to be sorted, data can be piped directly from the read mapper to ViralConsensus to avoid unnecessary disk I/O (and thus improve speed):

```bash
# ViralConsensus is single-threaded, so if running many samples, best to run mapper single-threaded as well and parallelize across samples
minimap2 -t 1 -a -x sr reference.fas reads.fastq.gz | viral_consensus -i - -r reference.fas -o consensus.fas
```

Users will likely want to keep the output of the read mapper (e.g. as a compressed BAM file) for other downstream analyses, but rather than writing the output SAM/BAM/CRAM to disk and *then* having ViralConsensus read from that file, `tee` should be used instead to send the stream down two (or more) paths:

```bash
minimap2 -t 1 -a -x sr reference.fas reads.fastq.gz | tee >(viral_consensus -i - -r reference.fas -o consensus.fas) | samtools view -b -@ 1 > reads.bam
```

# Citing ViralConsensus
If you use ViralConsensus in your work, please cite:

> Moshiri N (2022). "ViralConsensus: A fast and memory-efficient tool for calling viral consensus genome sequences directly from read alignment data." *bioRxiv*. [doi:10.1101/2023.01.05.522928](https://doi.org/10.1101/2020.11.10.377499)
