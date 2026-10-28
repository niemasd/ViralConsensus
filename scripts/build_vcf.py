#! /usr/bin/env python3
'''
Build a VCF file from ViralConsensus output files
'''

# imports
from gzip import open as gopen
from json import load as jload
from pathlib import Path
import argparse

# parse user args
def parse_args():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-f', '--fasta', required=True, type=str, help="ViralConsensus Output: Consensus Sequence FASTA")
    parser.add_argument('-t', '--tsv', required=True, type=str, help="ViralConsensus Output: Position Counts TSV")
    parser.add_argument('-j', '--json', required=True, type=str, help="ViralConsensus Output: Insertion Counts JSON")
    parser.add_argument('-r', '--ref', required=True, type=str, help="Reference FASTA")
    parser.add_argument('-o', '--output', required=False, type=str, default='stdout', help="Output VCF")
    parser.add_argument('-s', '--sample', required=False, type=str, default=None, help="Sample Name (defaults to the FASTA filename stem)")
    args = parser.parse_args()
    for a in ['fasta', 'tsv', 'json', 'ref', 'output']:
        v = getattr(args, a)
        if v in {'stdout', 'stderr', 'stdin'}:
            continue
        p = Path(v)
        if a == 'output':
            if p.exists():
                raise ValueError(f"Output exists: {p}")
        elif not p.is_file():
            raise ValueError(f"File not found: {p}")
        setattr(args, a, p)
    if args.sample is None:
        args.sample = args.fasta.stem
    return args

# open a file
def open_file(p, mode='rt'):
    if p == 'stdout':
        from sys import stdout as f
    elif p == 'stderr':
        from sys import stderr as f
    elif p == 'stdin':
        from sys import stdin as f
    else:
        ext = Path(p).suffix.strip().lower()
        if ext == '.gz':
            f = gopen(p, mode=mode)
        else:
            f = open(p, mode=mode)
    return f

# load ViralConsensus arguments from FASTA
def load_vc_params(p):
    with open_file(p, mode='rt') as f:
        header = f.readline()
    return {
        'version': header.split('viral_consensus v')[1].split()[0],
        'min_qual': int(header.split('--min_qual ')[1].split()[0]),
        'min_depth': int(header.split('--min_depth ')[1].split()[0]),
        'min_freq': float(header.split('--min_freq ')[1].split()[0]),
    }

# load position counts from TSV
def load_pos_counts(p):
    out = dict()
    with open_file(p, mode='rt') as f:
        for line_num, line in enumerate(f):
            parts = [s.strip().upper() for s in line.split('\t')]
            if line_num == 0:
                col2ind = {col:ind for ind, col in enumerate(parts)}
            else:
                out[int(parts[col2ind['POS']])] = {col:int(parts[col2ind[col]]) for col in 'ACGT-'}
    return out

# load insertion counts from JSON
def load_ins_counts(p):
    with open_file(p, mode='rt') as f:
        return {int(k):v for k, v in jload(f).items()}

# load reference from FASTA
def load_ref(p):
    with open_file(p, mode='rt') as f:
        ref_ID = f.readline().split()[0][1:]
        ref_seq = ''.join(l.strip() for l in f).upper()
    return ref_ID, ref_seq

# build VCF from ViralConsensus outputs
def build_vcf(vc_params, pos_counts, ins_counts, ref_ID, ref_seq, sample_name='sample'):
    # preprocess reference
    ref_seq = ref_seq.strip().upper()
    ref_len = len(ref_seq)

    # set up VCF header
    vcf_lines = list()
    vcf_lines.append('##fileformat=VCFv4.2')
    vcf_lines.append(f'##source=ViralConsensus v{vc_params["version"]}')
    vcf_lines.append(f'##contig=<ID={ref_ID},length={ref_len}>')
    vcf_lines.append(f'##ViralConsensus_min_qual={vc_params["min_qual"]}')
    vcf_lines.append(f'##ViralConsensus_min_depth={vc_params["min_depth"]}')
    vcf_lines.append(f'##ViralConsensus_min_freq={vc_params["min_freq"]}')
    vcf_lines.append('##FORMAT=<ID=GT,Number=1,Type=String,Description="Genotype">')
    vcf_lines.append('##FORMAT=<ID=DP,Number=1,Type=Integer,Description="Total Depth">')
    vcf_lines.append('##FORMAT=<ID=DR,Number=1,Type=Integer,Description="Depth of Reference Allele">')
    vcf_lines.append('##FORMAT=<ID=DA,Number=1,Type=Integer,Description="Depth of Alternate Allele">')
    vcf_lines.append(f'#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT\t{sample_name}')

    # make per-position consensus call
    calls = list()
    alleles = list()
    depths = list()
    ref_depths = list()
    alt_depths = list()
    for pos in range(ref_len): # pos is 0-based; VCF coordinates are 1-based
        curr_counts = pos_counts[pos]
        ref_base = ref_seq[pos]
        ref_depth = curr_counts.get(ref_base, 0) if ref_base in 'ACGT' else 0
        depth = sum(curr_counts.values())
        best_count, best_call = 0, 'N'
        for base in 'ACGT-':
            if curr_counts[base] > best_count:
                best_count, best_call = curr_counts[base], base
        if depth < vc_params['min_depth'] or not (best_count > depth * vc_params['min_freq']):
            best_call = 'N'
            best_count = depth
        allele = best_call if best_call in 'ACGT' else None
        alt_depth = best_count
        if best_call in 'ACGT' and (pos + 1) in ins_counts:
            curr_ins_counts = ins_counts[pos + 1]
            if curr_ins_counts:
                valid_insertions = {seq.upper(): count for seq, count in curr_ins_counts.items() if seq}
                if valid_insertions:
                    best_ins_str, best_ins_count = max(((seq, count) for seq, count in valid_insertions.items()), key=lambda x: (x[1], x[0]))
                    ins_depth = sum(curr_ins_counts.values())
                    if ins_depth >= vc_params['min_depth'] and best_ins_count > ins_depth * vc_params['min_freq']:
                        allele = best_call + best_ins_str
                        depth = min(depth, ins_depth)
                        alt_depth = min(best_count, best_ins_count)
        calls.append(best_call)
        alleles.append(allele)
        depths.append(depth)
        ref_depths.append(ref_depth)
        alt_depths.append(alt_depth)

    # append one record (start is 0-based, and end is exclusive)
    def append_record(start, end, alt, support_positions):
        if not alt:
            return
        ref_allele = ref_seq[start:end]
        if not ref_allele or alt == ref_allele:
            return
        dp = min(depths[index] for index in support_positions)
        dr = min(ref_depths[index] for index in support_positions)
        da = min(alt_depths[index] for index in support_positions)
        vcf_lines.append(f'{ref_ID}\t{start + 1}\t.\t{ref_allele}\t{alt}\t.\tPASS\t.\tGT:DP:DR:DA\t1:{dp}:{dr}:{da}')

    # emit SNVs/insertions and merge each deletion run with an adjacent anchor base
    pos = 0
    while pos < ref_len:
        if calls[pos] == 'N':
            pos += 1; continue
        if calls[pos] == '-': # no usable left anchor (e.g. deletion at first base or after an N)
            deletion_end = pos # use next base as right anchor when available
            while deletion_end < ref_len and calls[deletion_end] == '-':
                deletion_end += 1
            if deletion_end < ref_len and calls[deletion_end] in 'ACGT':
                append_record(pos, deletion_end + 1, alleles[deletion_end], range(pos, deletion_end + 1))
                pos = deletion_end + 1
            else: # deletion with no unambiguous adjacent anchor can't be represented as VCF allele (so omit)
                pos = deletion_end
            continue
        if pos + 1 < ref_len and calls[pos + 1] == '-':
            deletion_end = pos + 1
            while deletion_end < ref_len and calls[deletion_end] == '-':
                deletion_end += 1
            append_record(pos, deletion_end, alleles[pos], range(pos, deletion_end))
            pos = deletion_end
            continue

        # normal SNV or insertion (N positions were skipped above)
        if alleles[pos] is not None:
            append_record(pos, pos + 1, alleles[pos], (pos,))
        pos += 1
    return '\n'.join(vcf_lines) + '\n'

# main logic
def main():
    args = parse_args()
    vc_params = load_vc_params(args.fasta)
    pos_counts = load_pos_counts(args.tsv)
    ins_counts = load_ins_counts(args.json)
    ref_ID, ref_seq = load_ref(args.ref)
    if len(ref_seq) != len(pos_counts):
        raise ValueError(f"Different number of positions in reference ({len(ref_seq)}) and TSV ({len(pos_counts)})")
    if ins_counts and max(ins_counts) > len(ref_seq) + 1:
        raise ValueError(f"Maximum JSON position ({max(ins_counts.keys())}) exceeds reference ({len(ref_seq)+1})")
    vcf_data = build_vcf(vc_params, pos_counts, ins_counts, ref_ID, ref_seq, sample_name=args.sample)
    with open_file(args.output, mode='wt') as f:
        f.write(vcf_data)

# run tool
if __name__ == "__main__":
    main()
