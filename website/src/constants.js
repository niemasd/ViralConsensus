export const BIOWASM_WORKING_DIR = "/shared/data/";
export const OUTPUT_ID = "output-text";
export const EXAMPLE_REF_FILE = "https://raw.githubusercontent.com/niemasd/ViralConsensus/master/example/NC_045512.2.fas";
export const DEFAULT_REF_FILE_NAME = BIOWASM_WORKING_DIR + 'ref.fas';
export const EXAMPLE_ALIGNMENT_FILE = "https://raw.githubusercontent.com/niemasd/ViralConsensus/master/example/example.trimmed.unsorted.bam";
export const DEFAULT_ALIGNMENT_SAM_FILE_NAME = BIOWASM_WORKING_DIR + 'alignments.sam';
export const DEFAULT_ALIGNMENT_BAM_FILE_NAME = BIOWASM_WORKING_DIR + 'alignments.bam';
export const DEFAULT_PRIMER_FILE_NAME = BIOWASM_WORKING_DIR + 'primers.txt';
export const COMBINED_SEQUENCES_FILE_NAME = BIOWASM_WORKING_DIR + 'sequences.fastq.gz';
export const MINIMAP_OUTPUT_FILE_NAME = BIOWASM_WORKING_DIR + 'reads.sam';
export const DEFAULT_VALS_FILE = "https://raw.githubusercontent.com/niemasd/ViralConsensus/main/common.h";
export const DEFAULT_VALS_MAPPING = {
	"DEFAULT_MIN_QUAL": "minBaseQuality",
	"DEFAULT_MIN_DEPTH": "minDepth",
	"DEFAULT_MIN_FREQ": "minFreq",
	"DEFAULT_AMBIG": "ambigSymbol",
	"DEFAULT_PRIMER_OFFSET": "primerOffset",
}
export const CONSENSUS_FILE_NAME = BIOWASM_WORKING_DIR + 'consensus.fa';
export const POSITION_COUNTS_FILE_NAME = BIOWASM_WORKING_DIR + 'positionCounts.tsv';
export const INSERTION_COUNTS_FILE_NAME = BIOWASM_WORKING_DIR + 'insertionCounts.json';

export const CLEAR_LOG = () => {
	const textArea = document.getElementById(OUTPUT_ID);
	textArea.value = "";
}

export const LOG = (output) => {
	const textArea = document.getElementById(OUTPUT_ID);
	const date = new Date();
	textArea.value += `${getTimeWithMilliseconds(date)}: ` + output + "\n";
}

export const ARE_FASTQ = (files) => {
	if (files === undefined || files.length === 0) {
		return false;
	}

	for (const file of files) {
		const name = file.name;
		if (!(name !== undefined && (
			name.endsWith('.fastq') ||
			name.endsWith('.fq') ||
			name.endsWith('.fastq.gz') ||
			name.endsWith('.fq.gz')
		))) {
			return false;
		}
	}

	return true;
}

export const IS_GZIP = (arrayBuffer) => {
	if (arrayBuffer.byteLength < 2) {
		return false;
	}

	const uint8Array = new Uint8Array(arrayBuffer.slice(0, 2));
	return uint8Array[0] === 0x1f && uint8Array[1] === 0x8b;
}

export const INPUT_IS_NONNEG_INTEGER = (input, lowBound = 0, upperBound = Number.MAX_SAFE_INTEGER) => {
	return (input === '' || (input >= lowBound && input <= upperBound && input == parseInt(input)))
}

export const getTimeWithMilliseconds = date => {
	const t = date.toLocaleTimeString();
	return `${t.substring(0, 7)}.${("00" + date.getMilliseconds()).slice(-3)}`;
}