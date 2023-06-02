import React, { Component } from 'react'

import { CLEAR_LOG, LOG, EXAMPLE_BAM_FILE, EXAMPLE_REF_FILE, DEFAULT_VALS_FILE, DEFAULT_VALS_MAPPING } from './constants'

import './App.css'

import loading from './assets/loading.png'

export class App extends Component {
	constructor(props) {
		super(props)

		this.state = {
			version: '',

			refFile: undefined,
			bamFile: undefined,
			primerFile: undefined,
			refFileValid: true,
			bamFileValid: true,
			primerFileValid: true,

			primerOffset: 0,
			primerOffsetValid: true,
			primerOffsetDefault: 0,

			minBaseQuality: 0,
			minBaseQualityValid: true,
			minBaseQualityDefault: 0,

			minDepth: 0,
			minDepthValid: true,
			minDepthDefault: 0,

			minFreq: 0,
			minFreqValid: true,
			minFreqDefault: 0,

			ambigSymbol: 'N',
			ambigSymbolValid: true,
			ambigSymbolDefault: 'N',

			genPosCounts: false,
			genInsCounts: false,
			CLI: undefined,
			done: false,
			loading: false,
			inputChanged: false
		}
	}

	async componentDidMount() {
		this.setState({
			CLI: await new Aioli(["ViralConsensus/viral_consensus/0.0.1", "minimap2/2.22", "fastp/0.20.1"])
		}, () => {
			CLEAR_LOG()
			LOG("ViralConsensus Online Tool loaded.")
		})

		this.loadDefaultsAndVersion();
	}

	loadDefaultsAndVersion = async () => {
		const defaultTextFile = await (await fetch(DEFAULT_VALS_FILE)).text();
		const defaultText = [...defaultTextFile.matchAll(/#define DEFAULT.*$/gm)].map((line) => line[0].split(' '));
		for (const defaultValue of defaultText) {
			if (DEFAULT_VALS_MAPPING[defaultValue[1]]) {
				if (isNaN(defaultValue[2])) {
					defaultValue[2] = defaultValue[2].replace(/"|'/g, '');
				} else {
					defaultValue[2] = Number(defaultValue[2]);
				}
				this.setState({ [DEFAULT_VALS_MAPPING[defaultValue[1]] + "Default"]: defaultValue[2], [DEFAULT_VALS_MAPPING[defaultValue[1]]]: defaultValue[2] })
			}
		}
		
		const version = 'v' + defaultTextFile.matchAll(/#define VERSION.*$/gm).next().value[0].split(' ')[2].replace(/"|'/g, '');
		this.setState({ version })
	}

	uploadBamFile = (e) => {
		this.setState({ bamFile: e.target.files[0], inputChanged: true })
	}

	uploadRefFile = (e) => {
		this.setState({ refFile: e.target.files[0], inputChanged: true })
	}

	uploadPrimerFile = (e) => {
		this.setState({ primerFile: e.target.files[0], inputChanged: true })
	}

	setPrimerOffset = (e) => {
		let primerOffsetValid = true;

		this.setState({ primerOffset: e.target.value, primerOffsetValid, inputChanged: true })
	}

	setMinBaseQuality = (e) => {
		let minBaseQualityValid = true;

		if (e.target.value < 0) {
			minBaseQualityValid = false;
		}

		this.setState({ minBaseQuality: e.target.value, minBaseQualityValid, inputChanged: true })
	}

	setMinDepth = (e) => {
		let minDepthValid = true;

		if (e.target.value < 0) {
			minDepthValid = false;
		}

		this.setState({ minDepth: e.target.value, minDepthValid, inputChanged: true })
	}

	setMinFreq = (e) => {
		let minFreqValid = true;

		if (e.target.value < 0 || e.target.value > 1) {
			minFreqValid = false;
		}

		this.setState({ minFreq: e.target.value, minFreqValid, inputChanged: true })
	}

	setAmbigSymbol = (e) => {
		let ambigSymbolValid = true;

		if (e.target.value.length !== 1) {
			ambigSymbolValid = false;
		}

		this.setState({ ambigSymbol: e.target.value, ambigSymbolValid, inputChanged: true })
	}

	setGenPosCounts = (e) => {
		this.setState({ genPosCounts: e.target.checked, inputChanged: true })
	}

	setGenInsCounts = (e) => {
		this.setState({ genInsCounts: e.target.checked, inputChanged: true })
	}

	loadExampleData = () => {
		this.setState(prevState => {
			return {
				refFile: 'EXAMPLE_DATA',
				bamFile: 'EXAMPLE_DATA',
				refFileValid: true,
				bamFileValid: true,
				inputChanged: prevState.refFile !== 'EXAMPLE_DATA' || prevState.bamFile !== 'EXAMPLE_DATA'
			}
		})
	}

	validInput = () => {
		let valid = true;
		let refFileValid = true;
		let bamFileValid = true;
		// Note: Other input validation is done in the setters

		CLEAR_LOG()
		LOG("Validating input...")

		if (!this.state.refFile) {
			refFileValid = false;
		}

		if (!this.state.bamFile) {
			bamFileValid = false;
		}

		valid = refFileValid && bamFileValid && this.state.primerOffsetValid && this.state.minBaseQualityValid && this.state.minDepthValid && this.state.minFreqValid && this.state.ambigSymbolValid;

		this.setState({ refFileValid, bamFileValid })

		return valid;
	}

	runViralConsensus = async () => {
		if (!this.validInput()) {
			LOG("Invalid input. Please check your input and try again.")
			return;
		}

		const startTime = performance.now();
		LOG("Running ViralConsensus...")
		this.setState({ done: false, loading: true, inputChanged: false })

		const CLI = this.state.CLI;
		let command = `viral_consensus -i ${this.state.bamFile?.name ?? 'alignments.bam'} -r ${this.state.refFile?.name ?? 'ref.fas'} -o consensus.fa`;

		// Delete old files
		// TODO: is there a better way to delete a file other than unlink?
		if (await CLI.ls('consensus.fa')) {
			await CLI.fs.unlink('consensus.fa');
		}

		if (await CLI.ls('positionCounts.tsv')) {
			await CLI.fs.unlink('positionCounts.tsv');
		}

		if (await CLI.ls('insertionCounts.tsv')) {
			await CLI.fs.unlink('insertionCounts.tsv');
		}

		// Create example reference fasta file
		if (this.state.refFile === 'EXAMPLE_DATA') {
			const refFile = await (await fetch(EXAMPLE_REF_FILE)).text();
			await CLI.mount({
				name: 'ref.fas',
				data: refFile
			})
			// await CLI.mount([{ name: "ref.fas", url: EXAMPLE_REF_FILE }])
			// console.log(await CLI.cat('ref.fas'))
		} else {
			await CLI.mount([this.state.refFile])
		}

		// Create example alignments
		if (this.state.bamFile === 'EXAMPLE_DATA') {
			await CLI.mount([{ name: "alignments.bam", url: EXAMPLE_BAM_FILE }])
		} else {
			await CLI.mount([this.state.bamFile])
		}

		// Create example primer file
		if (this.state.primerFile) {
			const fileReader = new FileReader();
			fileReader.onload = async () => {
				await CLI.fs.writeFile(this.state.primerFile.name, new Uint8Array(fileReader.result));
			}
			fileReader.readAsArrayBuffer(this.state.primerFile);
			// TODO: make validation works
			command += ` -p ${this.state.primerFile.name} -po ${this.state.primerOffset}`;
		}

		// Set parameters
		command += ` -q ${this.state.minBaseQuality} -d ${this.state.minDepth} -f ${this.state.minFreq} -a ${this.state.ambigSymbol}`;

		// Set output files
		if (this.state.genPosCounts) {
			command += ' -op positionCounts.tsv';
		}

		if (this.state.genInsCounts) {
			command += ' -oi insertionCounts.tsv';
		}

		// Generate consensus genome
		LOG("Executing command: " + command)
		await CLI.exec(command);
		const consensusFile = await CLI.ls('consensus.fa');
		if (!consensusFile || consensusFile.size === 0) {
			LOG("Error: No consensus genome generated. Please check your input files.")
			this.setState({ loading: false })
			return;
		}

		this.setState({ done: true, loading: false })
		LOG(`Done! Time Elapsed: ${((performance.now() - startTime) / 1000).toFixed(3)} seconds`);
	}

	downloadConsensus = async () => {
		await this.downloadFile('consensus.fa');
		await this.downloadFile('positionCounts.tsv');
		await this.downloadFile('insertionCounts.tsv');
	}

	downloadFile = async (fileName) => {

		const CLI = this.state.CLI;
		if (!(await CLI.ls(fileName))) {
			return;
		}

		const fileBlob = await CLI.download(fileName);

		const element = document.createElement("a");
		element.href = fileBlob;
		element.download = fileName;
		document.body.appendChild(element);
		element.click();
		document.body.removeChild(element);

		LOG(`Downloaded ${fileName}`)
	}

	render() {
		return (
			<div className="App pb-5">
				<h1 className="mt-4 mb-5 text-center">ViralConsensus Online Tool {this.state.version}</h1>
				<div className="mt-3" id="container">
					<div id="input" className="ms-5 me-4">
						<h4 className="mb-3">Input</h4>
						<div className="d-flex flex-column mb-4">
							<label htmlFor="reference-file" className="form-label">Reference File <span className="text-danger">*</span></label>
							<input className={`form-control ${!this.state.refFileValid && 'is-invalid'}`} type="file" id="reference-file" onChange={this.uploadRefFile} />
							{this.state.refFile === 'EXAMPLE_DATA' && <p className="mb-0">Using example <a href={EXAMPLE_REF_FILE} target="_blank" rel="noreferrer">reference file</a>.</p>}
						</div>

						<div className="d-flex flex-column mb-4">
							<label htmlFor="bam-file" className="form-label">Input BAM File <span className="text-danger">*</span></label>
							<input className={`form-control ${!this.state.bamFileValid && 'is-invalid'}`} type="file" id="bam-file" onChange={this.uploadBamFile} />
							{this.state.bamFile === 'EXAMPLE_DATA' && <p className="mb-0">Using example <a href={EXAMPLE_BAM_FILE} target="_blank" rel="noreferrer">BAM file</a>.</p>}
						</div>

						<button type="button" className="btn btn-warning mb-3" onClick={this.loadExampleData}>Load Example Data Files</button>
						{(this.state.bamFile === 'EXAMPLE_DATA' || this.state.refFile === 'EXAMPLE_DATA') && <h6 className="mb-5 text-center text-success">
							Using example data file(s)!
						</h6>}

						<div className="accordion accordion-flush mb-4" id="optional-args">
							<div className="accordion-item">
								<h2 className="accordion-header">
									<button className="accordion-button collapsed" type="button" data-bs-toggle="collapse" data-bs-target="#opt-args-collapse" aria-expanded="false" aria-controls="opt-args-collapse">
										Optional Arguments
									</button>
								</h2>
								<div id="opt-args-collapse" className="accordion-collapse collapse pt-4" data-bs-parent="#optional-args">
									<div className="d-flex flex-column mb-4">
										<label htmlFor="primer-file" className="form-label">Primer (BED) File</label>
										<input className="form-control" type="file" id="primer-file" onChange={this.uploadPrimerFile} />
									</div>

									<label htmlFor="min-base-quality" className="form-label">Primer Offset</label>
									<input id="primer-offset" className={`form-control ${!this.state.primerOffsetValid && 'is-invalid'}`} type="number" placeholder="Primer Offset" value={this.state.primerOffset} onChange={this.setPrimerOffset} />
									<div className="form-text mb-4">Number of bases after primer to also trim (default: {this.state.primerOffsetDefault})</div>

									<label htmlFor="min-base-quality" className="form-label">Minimum Base Quality</label>
									<input id="min-base-quality" className={`form-control ${!this.state.minBaseQualityValid && 'is-invalid'}`} type="number" placeholder="Minimum Base Quality" value={this.state.minBaseQuality} onChange={this.setMinBaseQuality} />
									<div className="form-text mb-4">Min. base quality to count base in counts (default: {this.state.minBaseQualityDefault})</div>

									<label htmlFor="min-depth" className="form-label">Minimum Depth</label>
									<input id="min-depth" className={`form-control ${!this.state.minDepthValid && 'is-invalid'}`} type="number" placeholder="Minimum Depth" value={this.state.minDepth} onChange={this.setMinDepth} />
									<div className="form-text mb-4">Min. depth to call base in consensus (default: {this.state.minDepthDefault})</div>

									<label htmlFor="min-freq" className="form-label">Minimum Frequency</label>
									<input id="min-freq" className={`form-control ${!this.state.minFreqValid && 'is-invalid'}`} type="number" placeholder="Minimum Frequency" value={this.state.minFreq} onChange={this.setMinFreq} />
									<div className="form-text mb-4">Min. frequency to call base/insertion in consensus (default: {this.state.minFreqDefault})</div>

									<label htmlFor="ambig-symbol" className="form-label">Ambiguous Symbol</label>
									<input id="ambig-symbol" className={`form-control ${!this.state.ambigSymbolValid && 'is-invalid'}`} type="text" placeholder="Ambiguous Symbol" value={this.state.ambigSymbol} onChange={this.setAmbigSymbol} />
									<div className="form-text mb-4">Symbol to use for ambiguous bases (default: {this.state.ambigSymbolDefault})</div>

									<div className="form-check">
										<label className="form-check-label" htmlFor="output-pos-counts">
											Generate Position Counts
										</label>
										<input className="form-check-input" type="checkbox" name="output-pos-counts" id="output-pos-counts" checked={this.state.genPosCounts} onChange={this.setGenPosCounts} />
									</div>
									<div className="form-check">
										<label className="form-check-label" htmlFor="output-ins-counts">
											Generate Insertion Counts
										</label>
										<input className="form-check-input" type="checkbox" name="output-ins-counts" id="output-ins-counts" checked={this.state.genInsCounts} onChange={this.setGenInsCounts} />
									</div>
								</div>
							</div>
						</div>
						<button type="button" className="btn btn-primary" onClick={this.runViralConsensus}>Submit</button>
					</div>
					<div id="output" className="form-group ms-4 me-5">
						<label htmlFor="output-text" className="mb-3"><h4>Console</h4></label>
						<textarea className="form-control" id="output-text" rows="3" disabled></textarea>
						{this.state.loading && <img id="loading" className="mt-3" src={loading} />}
						{this.state.done && <button type="button" className={`btn btn-primary mt-4`} onClick={this.downloadConsensus}>Download Output</button>}
						{this.state.done && this.state.inputChanged && <p className="text-danger text-center mt-4">Warning: Form input has changed since last run, run again to download latest output files.</p>}
					</div>
				</div>
				<footer className="text-center">
					Web-based implementation of <a href="https://www.github.com/niemasd/ViralConsensus" target="_blank" rel="noreferrer">ViralConsensus</a> using WebAssembly and <a href="https://biowasm.com/" target="_blank" rel="noreferrer">Biowasm</a>.<br />
					Special thank you to Robert Aboukhalil for his support.<br />
				</footer>
			</div>
		)
	}
}

export default App