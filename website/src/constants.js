export const OUTPUT_ID = "output-text";
export const EXAMPLE_REF_FILE = "https://raw.githubusercontent.com/niemasd/ViralConsensus/master/example/NC_045512.2.fas";
export const EXAMPLE_BAM_FILE = "https://raw.githubusercontent.com/niemasd/ViralConsensus/master/example/example.trimmed.unsorted.bam";
export const DEFAULT_VALS_FILE = "https://raw.githubusercontent.com/niemasd/ViralConsensus/main/common.h";
export const DEFAULT_VALS_MAPPING = {
    "DEFAULT_MIN_QUAL": "minBaseQuality",
    "DEFAULT_MIN_DEPTH": "minDepth",
    "DEFAULT_MIN_FREQ": "minFreq", 
    "DEFAULT_AMBIG": "ambigSymbol",
    "DEFAULT_PRIMER_OFFSET": "primerOffset",
}

export const CLEAR_LOG = () => {
    const textArea = document.getElementById(OUTPUT_ID);
    textArea.value = "";
}

export const LOG = (output) => {
    const textArea = document.getElementById(OUTPUT_ID);
    const date = new Date();
    textArea.value += `${getTimeWithMilliseconds(date)}: ` + output + "\n";
}

export const getTimeWithMilliseconds = date => {
    const t = date.toLocaleTimeString();
    return `${t.substring(0, 7)}.${("0" + date.getMilliseconds()).slice(-3)}`;
}