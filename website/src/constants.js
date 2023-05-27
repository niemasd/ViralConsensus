export const OUTPUT_ID = "outputText";
export const EXAMPLE_REF_FILE = "https://raw.githubusercontent.com/niemasd/ViralConsensus/master/example/NC_045512.2.fas";
export const EXAMPLE_BAM_FILE = "https://raw.githubusercontent.com/niemasd/ViralConsensus/master/example/example.trimmed.unsorted.bam";

export const LOG = (output, clear = false) => {
    const textArea = document.getElementById(OUTPUT_ID);
    if (clear) {
        textArea.value = "";
    }
    const date = new Date();
    textArea.value += `${getTimeWithMilliseconds(date)}: ` + output + "\n";
}

export const getTimeWithMilliseconds = date => {
    const t = date.toLocaleTimeString();
    return `${t.substring(0, 7)}.${("0" + date.getMilliseconds()).slice(-3)}`;
}