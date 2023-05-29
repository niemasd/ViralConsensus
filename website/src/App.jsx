import React, { Component } from 'react'

import { LOG, EXAMPLE_BAM_FILE, EXAMPLE_REF_FILE, DEFAULT_VALS_FILE, DEFAULT_VALS_MAPPING } from './constants'

import './App.css'

import loading from './assets/loading.png'

export class App extends Component {
	constructor(props) {
		super(props)

		this.state = {
			refFile: undefined,
			bamFile: undefined,
			primerFile: undefined,
			primerOffset: 0,
			minBaseQuality: 0,
			minDepth: 0,
			minFreq: 0,
			ambigSymbol: 'N',
			genPosCounts: false,
			genInsCounts: false,
			CLI: undefined,
			done: false,
			loading: false
		}
	}

	async componentDidMount() {
		this.setState({
			CLI: await new Aioli(["ViralConsensus/viral_consensus/0.0.1", "minimap2/2.22", "fastp/0.20.1"])
		}, () => {
			LOG("ViralConsensus Online Tool loaded.", true)
		})

		this.loadDefaults();
	}

	loadDefaults = async () => {
		const defaultTextFile = await (await fetch(DEFAULT_VALS_FILE)).text();
		const defaultText = [...defaultTextFile.matchAll(/#define DEFAULT.*$/gm)].map((line) => line[0].split(' '));
		for (const defaultValue of defaultText) {
			if (DEFAULT_VALS_MAPPING[defaultValue[1]]) {
				if (isNaN(defaultValue[2])) {
					defaultValue[2] = defaultValue[2].replace(/"|'/g, '');
				} else {
					defaultValue[2] = Number(defaultValue[2]);
				}
				this.setState({ [DEFAULT_VALS_MAPPING[defaultValue[1]]]: defaultValue[2] })
			}
		}
	}

	uploadBamFile = (e) => {
		this.setState({ bamFile: e.target.files[0] })
	}

	uploadRefFile = (e) => {
		this.setState({ refFile: e.target.files[0] })
	}

	uploadPrimerFile = (e) => {
		this.setState({ primerFile: e.target.files[0] })
	}

	setPrimerOffset = (e) => {
		this.setState({ primerOffset: e.target.value })
	}

	setMinBaseQuality = (e) => {
		this.setState({ minBaseQuality: e.target.value })
	}

	setMinDepth = (e) => {
		this.setState({ minDepth: e.target.value })
	}

	setMinFreq = (e) => {
		this.setState({ minFreq: e.target.value })
	}

	setAmbigSymbol = (e) => {
		this.setState({ ambigSymbol: e.target.value })
	}

	setGenPosCounts = (e) => {
		this.setState({ genPosCounts: e.target.checked })
	}

	setGenInsCounts = (e) => {
		this.setState({ genInsCounts: e.target.checked })
	}

	loadExampleData = () => {
		this.setState({
			refFile: 'EXAMPLE_DATA',
			bamFile: 'EXAMPLE_DATA'
		})
	}

	validInput = () => {
		let valid = true;
		LOG("Validating input...", true)

		if (!this.state.refFile) {
			LOG("Please upload a reference file.")
			valid = false;
		}

		if (!this.state.bamFile) {
			LOG("Please upload a BAM file.")
			valid = false;
		}

		return valid;
	}

	runViralConsensus = async () => {
		if (!this.validInput()) {
			return;
		}

		const startTime = performance.now();
		LOG("Running ViralConsensus...")
		this.setState({ done: false, loading: true })

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
		} else {
			const fileReader = new FileReader();
			fileReader.onload = async () => {
				await CLI.mount({
					name: this.state.refFile.name,
					data: fileReader.result
				})
			}
			fileReader.readAsText(this.state.refFile);
		}

		// Create example alignments
		if (this.state.bamFile === 'EXAMPLE_DATA') {
			const bamFile = await (await fetch(EXAMPLE_BAM_FILE)).arrayBuffer();
			await CLI.fs.writeFile('alignments.bam', new Uint8Array(bamFile));
		} else {
			const fileReader = new FileReader();
			fileReader.onload = async () => {
				await CLI.fs.writeFile(this.state.bamFile.name, new Uint8Array(fileReader.result));
			}
			fileReader.readAsArrayBuffer(this.state.bamFile);
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
		// await CLI.exec(command);
		console.log(command)
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
				<h1 className="mt-4 mb-5 text-center">ViralConsensus Online Tool</h1>
				<div className="container mt-3">
					<div id="input" className="mx-5">
						<h4 className="mb-3">Input</h4>
						<div className="d-flex flex-column mb-4">
							<label htmlFor="reference-file" className="form-label">Reference File <span className="text-danger">*</span></label>
							<input className="form-control" type="file" id="reference-file" onChange={this.uploadRefFile} />
							{this.state.refFile === 'EXAMPLE_DATA' && <p className="mb-0">Using example <a href={EXAMPLE_REF_FILE} target="_blank" rel="noreferrer">reference file</a>.</p>}
						</div>

						<div className="d-flex flex-column mb-4">
							<label htmlFor="bam-file" className="form-label">Input BAM File <span className="text-danger">*</span></label>
							<input className="form-control" type="file" id="bam-file" onChange={this.uploadBamFile} />
							{this.state.bamFile === 'EXAMPLE_DATA' && <p className="mb-0">Using example <a href={EXAMPLE_BAM_FILE} target="_blank" rel="noreferrer">BAM file</a>.</p>}
						</div>

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

									<label htmlFor="min-base-quality" className="form-label">Number of Bases After Primer to Also Trim</label>
									<input id="primer-offset" className="form-control mb-4" type="number" placeholder="Primer Offset" value={this.state.primerOffset} onChange={this.setPrimerOffset} />

									<label htmlFor="min-base-quality" className="form-label">Min. Base Quality to Count Base in Counts</label>
									<input id="min-base-quality" className="form-control mb-4" type="number" placeholder="Minimum Base Quality" value={this.state.minBaseQuality} onChange={this.setMinBaseQuality} />

									<label htmlFor="min-depth" className="form-label">Min. Depth to Call Base/Insertion in Consensus</label>
									<input id="min-depth" className="form-control mb-4" type="number" placeholder="Minimum Depth" value={this.state.minDepth} onChange={this.setMinDepth} />

									<label htmlFor="min-freq" className="form-label">Min. Frequency to Call Base/Insertion in Consensus</label>
									<input id="min-freq" className="form-control mb-4" type="number" placeholder="Minimum Frequency" value={this.state.minFreq} onChange={this.setMinFreq} />

									<label htmlFor="ambig-symbol" className="form-label">Ambiguous Symbol</label>
									<input id="ambig-symbol" className="form-control mb-4" type="text" placeholder="Ambiguous Symbol" value={this.state.ambigSymbol} onChange={this.setAmbigSymbol} />

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
						<button type="button" className="btn btn-warning mt-4 mb-4" onClick={this.loadExampleData}>Load Example Data</button>
						<button type="button" className="btn btn-primary" onClick={this.runViralConsensus}>Submit</button>
					</div>
					<div id="output" className="form-group">
						<label htmlFor="output-text" className="mb-3"><h4>Console</h4></label>
						<textarea className="form-control" id="output-text" rows="3" disabled></textarea>
						{this.state.loading && <img id="loading" className="mt-3" src={loading} />}
						{this.state.done && <button type="button" className={`btn btn-primary mt-3`} onClick={this.downloadConsensus}>Download Output</button>}
					</div>
				</div>
			</div>
		)
	}
}

export default App