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
    args = parser.parse_args()
    for a in ['fasta', 'tsv', 'json', 'ref', 'output']:
        v = getattr(args, a)
        if v in {'stdout', 'stderr', 'stdin'}:
            continue
        p = Path(v)
        if a == 'output':
            if p.exists():
                raise ValueError(f"Output exists: {p}")
        else:
            if not p.is_file():
                raise ValueError(f"File not found: {p}")
        setattr(args, a, p)
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
        ext = p.suffix.strip().lower()
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
        ref_seq = ''.join(l.strip() for l in f.readlines())
    return ref_ID, ref_seq

# build VCF from ViralConsensus outputs
def build_vcf(vc_params, pos_counts, ins_counts, ref_ID, ref_seq, sample_name='sample'):
    # set up VCF header
    vcf_lines = list()
    vcf_lines.append('##fileformat=VCFv4.2')
    vcf_lines.append(f'##source=ViralConsensus v{vc_params["version"]}')
    vcf_lines.append(f'##FILTER=<ID=q{vc_params["min_qual"]},Description="ViralConsensus --min_qual={vc_params["min_qual"]}">')
    vcf_lines.append(f'##FILTER=<ID=d{vc_params["min_depth"]},Description="ViralConsensus --min_depth={vc_params["min_depth"]}">')
    vcf_lines.append(f'##FILTER=<ID=f{vc_params["min_freq"]},Description="ViralConsensus --min_freq={vc_params["min_freq"]}">')
    vcf_lines.append('##FORMAT=<ID=DP,Number=1,Type=Integer,Description="Total Depth">')
    vcf_lines.append('##FORMAT=<ID=DR,Number=1,Type=Integer,Description="Depth of Reference Allele">')
    vcf_lines.append('##FORMAT=<ID=DA,Number=1,Type=Integer,Description="Depth of Alternate Allele">')
    vcf_lines.append(f'#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT\t{sample_name}')

    # call variants and return VCF
    for pos in range(len(ref_seq)): # pos is 0-based; VCF is 1-based
        # handle match/mismatch/deletion at pos
        curr_pos_counts = pos_counts[pos]
        ref_nuc = ref_seq[pos]
        ref_depth = curr_pos_counts[ref_nuc]
        pos_depth = sum(curr_pos_counts.values())
        pos_best_count, pos_best = max((curr_pos_counts[s],s) for s in 'ACGT-')
        if (pos_depth < vc_params['min_depth']) or (pos_best_count < (pos_depth * vc_params['min_freq'])):
            pos_best_count = pos_depth
            pos_best = 'N'
        if pos_best == '-':
            pos_best = '.' # VCF uses '.' to denote a deletion

        # handle insertions after pos
        if pos_best != '.' and (pos+1) in ins_counts: # JSON has insertions that appear *before* position key
            curr_ins_counts = ins_counts[pos+1]
            curr_ins_depth = sum(curr_ins_counts.values())
            best_ins_count, best_ins_str = max((c,s) for s, c in curr_ins_counts.items())
            if (best_ins_str != '') and (curr_ins_depth >= vc_params['min_depth']) and (best_ins_count >= (curr_ins_depth * vc_params['min_freq'])):
                pos_depth = min(pos_depth, curr_ins_depth)
                pos_best_count = min(pos_best_count, best_ins_count)
                pos_best += best_ins_str

        # add to VCF output
        if pos_best not in {'N', ref_nuc}: # only output unambiguous non-reference variants
            vcf_lines.append(f'{ref_ID}\t{pos+1}\t.\t{ref_nuc}\t{pos_best}\t.\tPASS\t.\tGT:DP:DR:DA\t1:{pos_depth}:{ref_depth}:{pos_best_count}')
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
    if max(ins_counts.keys()) > len(ref_seq) + 1:
        raise ValueError(f"Maximum JSON position ({max(ins_counts.keys())}) exceeds reference ({len(ref_seq)+1})")
    vcf_data = build_vcf(vc_params, pos_counts, ins_counts, ref_ID, ref_seq, sample_name=args.fasta.stem)
    with open_file(args.output, mode='wt') as f:
        f.write(vcf_data)

# run tool
if __name__ == "__main__":
    main()
