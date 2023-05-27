import React, { Component } from 'react'

import { LOG, EXAMPLE_BAM_FILE, EXAMPLE_REF_FILE } from './constants'

import './App.css'

import loading from './assets/loading.png'

export class App extends Component {
	constructor(props) {
		super(props)

		this.state = {
			refFile: undefined,
			bamFile: undefined,
			CLI: undefined,
			done: false,
			loading: false
		}
	}

	async componentDidMount() {
		this.setState({
			CLI: await new Aioli(["ViralConsensus/viral_consensus/0.0.1", "minimap2/2.22", "fastp/0.20.1", "coreutils/wc/8.32"])
		}, () => {
			LOG("ViralConsensus Online Tool loaded.", true)
		})
	}

	uploadBamFile = (e) => {
		this.setState({ bamFile: e.target.files[0] })
	}

	uploadRefFile = (e) => {
		this.setState({ refFile: e.target.files[0] })
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

		// Delete old files
		// TODO: is there a better way to delete a file other than unlink?
		if (await CLI.ls('consensus.fa')) {
			await CLI.fs.unlink('consensus.fa');
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

		// Generate consensus genome
		await CLI.exec(`viral_consensus -i ${this.state.bamFile?.name ?? 'alignments.bam'} -r ${this.state.refFile?.name ?? 'ref.fas'} -o consensus.fa`);
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
		const CLI = this.state.CLI;
		const file = await CLI.cat('consensus.fa');

		const element = document.createElement("a");
		const fileBlob = new Blob([file], { type: 'text/plain' });
		element.href = URL.createObjectURL(fileBlob);
		element.download = "consensus.fa";
		document.body.appendChild(element);
		element.click();
		document.body.removeChild(element);

		LOG("Downloaded consensus.fa")
	}

	render() {
		return (
			<div className="App">
				<h1 className="my-3 text-center">ViralConsensus Online Tool</h1>
				<div className="container mt-3">
					<div id="input" className="mx-5">
						<div className="d-flex flex-column mb-5">
							<label htmlFor="referenceFile" className="form-label">Reference File</label>
							<input className="form-control" type="file" id="referenceFile" onChange={this.uploadRefFile} />
							{this.state.refFile === 'EXAMPLE_DATA' && <p className="mb-0">Using example <a href={EXAMPLE_REF_FILE} target="_blank" rel="noreferrer">reference file.</a></p>}
						</div>
						<div className="d-flex flex-column mb-5">
							<label htmlFor="bamFile" className="form-label">Input BAM File</label>
							<input className="form-control" type="file" id="bamFile" onChange={this.uploadBamFile} />
							{this.state.bamFile === 'EXAMPLE_DATA' && <p className="mb-0">Using example <a href={EXAMPLE_BAM_FILE} target="_blank" rel="noreferrer">BAM file.</a></p>}
						</div>
						<button type="button" className="btn btn-warning mb-5" onClick={this.loadExampleData}>Load Example Data</button>
						<button type="button" className="btn btn-primary" onClick={this.runViralConsensus}>Submit</button>
					</div>
					<div id="output" className="form-group">
						<label htmlFor="outputText" className="mb-3"><h4>Console</h4></label>
						<textarea className="form-control" id="outputText" rows="3" disabled></textarea>
						{this.state.loading && <img id="loading" className="mt-3" src={loading} />}
						{this.state.done && <button type="button" className={`btn btn-primary mt-3`} onClick={this.downloadConsensus}>Download Output</button>}
					</div>
				</div>
			</div>
		)
	}
}

export default App