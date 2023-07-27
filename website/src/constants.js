export const OUTPUT_ID = "output-text";
export const EXAMPLE_REF_FILE = "https://raw.githubusercontent.com/niemasd/ViralConsensus/master/example/NC_045512.2.fas";
export const DEFAULT_REF_FILE_NAME = 'ref.fas';
export const EXAMPLE_ALIGNMENT_FILE = "https://raw.githubusercontent.com/niemasd/ViralConsensus/master/example/example.trimmed.unsorted.bam";
export const DEFAULT_ALIGNMENT_FILE_NAME = 'alignments.bam';
export const MINIMAP_OUTPUT_FILE_NAME = 'reads.sam';
// TODO: change back to CRAM when we can support it
export const SAMTOOLS_OUTPUT_FILE_NAME = 'mapped_reads.bam';
export const DEFAULT_VALS_FILE = "https://raw.githubusercontent.com/niemasd/ViralConsensus/main/common.h";
export const DEFAULT_VALS_MAPPING = {
    "DEFAULT_MIN_QUAL": "minBaseQuality",
    "DEFAULT_MIN_DEPTH": "minDepth",
    "DEFAULT_MIN_FREQ": "minFreq", 
    "DEFAULT_AMBIG": "ambigSymbol",
    "DEFAULT_PRIMER_OFFSET": "primerOffset",
}
export const CONSENSUS_FILE_NAME = 'consensus.fa';
export const POSITION_COUNTS_FILE_NAME = 'positionCounts.tsv';
export const INSERTION_COUNTS_FILE_NAME = 'insertionCounts.json';

export const CLEAR_LOG = () => {
    const textArea = document.getElementById(OUTPUT_ID);
    textArea.value = "";
}

export const LOG = (output) => {
    const textArea = document.getElementById(OUTPUT_ID);
    const date = new Date();
    textArea.value += `${getTimeWithMilliseconds(date)}: ` + output + "\n";
}

export const IS_FASTQ = (name) => {
	return name !== undefined && (
		name.endsWith('.fastq') ||
		name.endsWith('.fq') ||
		name.endsWith('.fastq.gz') ||
		name.endsWith('.fq.gz')
	);
}

export const getTimeWithMilliseconds = date => {
    const t = date.toLocaleTimeString();
    return `${t.substring(0, 7)}.${("00" + date.getMilliseconds()).slice(-3)}`;
}