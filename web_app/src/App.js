import React, { Component } from 'react';
import logo from './logo.svg';
import back from './back.png';
import './App.css';
import {Line, Bar, Pie} from 'react-chartjs-2';
import AWS from 'aws-sdk'

AWS.config.update({
	region: 'eu-west-1',
	accessKeyId:  "",
 	secretAccessKey: "",
});

var ddb = new AWS.DynamoDB({apiVersion: '2012-08-10'});

var entity = []

function checkUpdate(n){
	if(entity[n].state.counter !== entity[n].state.data.datasets[0].data){
		console.log(n)
		var temp = entity[n].state.data
		temp.datasets[0].data = entity[n].state.counter
		if(entity[n].state.type === 'Timeline'){
			temp.labels = getLabels(new Date());
		}
		entity[n].setState({data: temp});
	}
}

function getLabels(today){
	var a_day = 1000 * 60 * 60 * 24

	return [
		new Date(today - a_day * 6).toLocaleDateString('it-IT'),
		new Date(today - a_day * 5).toLocaleDateString('it-IT'),
		new Date(today - a_day * 4).toLocaleDateString('it-IT'),
		new Date(today - a_day * 3).toLocaleDateString('it-IT'),
		new Date(today - a_day * 2).toLocaleDateString('it-IT'),
		new Date(today - a_day * 1).toLocaleDateString('it-IT'),
		new Date(today).toLocaleDateString('it-IT'),
	]
}

function evaluatePic(n, result, element){
	delete entity[n].state.listItems[element.item_id.N]
	var params = {
		TableName: "Garbage_items",
		Key: {
			"item_id": {N: element.item_id.N}
		},
		UpdateExpression: "set true_class = :r",
		ExpressionAttributeValues:{
			":r": {BOOL: result}
		},
		ReturnValues:"UPDATED_NEW"
	}

	ddb.updateItem(params, (err, data) => {
		console.log(err, data);
	})
	entity[n].setState({listItems: entity[n].state.listItems});
}

class MyChart extends Component{
	constructor(props){
		super(props)
		this.state = {
			data: {
				labels: this.props.labels,
				datasets: [{
					label: this.props.title,
					data: this.props.counter,
					backgroundColor: this.props.backgroundColor,
					borderColor: this.props.borderColor,
					borderWidth: 1,
					lineTension: this.props.lineTension ? this.props.lineTension : undefined
				}]
			},
			options: {
				legend:{
					display: false
				}
			},
			counter: this.props.counter.map((x) => {return x*0}),
			generate_data: this.props.generate_data,
			type: this.props.type,
			listItems: []
		}
		entity[entity.length] = this
		this.retrieveData(entity.length-1)
		this.interval = setInterval(() => {
			this.retrieveData(entity.length-1)
			console.log(this)
			console.log(entity.length-1)
		}, 10000);
	}

	retrieveData(n){
		var params = {
			TableName: "Garbage_items",
		};
		
		ddb.scan(params, function(err, data) {
			if (err) {
				console.log("Error", err);
			}
			else {
				data.Items.forEach(function(element, index, data){
					entity[n].state.generate_data(element, n, index);
				});
			}
			checkUpdate(n);
		});
	}

	render(){
		switch(this.state.type){
			case 'Timeline':
			case 'Line':
				return(
					<div className="mychart">
						<h3>{this.props.title}</h3>
						<Line data={this.state.data} options={this.state.options}/>
					</div>
				)
			case 'Bar':
				return(
					<div className="mychart">
						<h3>{this.props.title}</h3>
						<Bar data={this.state.data} options={this.state.options}/>
					</div>
				)
			case 'Pie':
				return(
					<div className="mychart">
						<h3>{this.props.title}</h3>
						<Pie data={this.state.data}/>
					</div>
				)
			case 'List':
				return(
					<ul>{Object.values(this.state.listItems)}</ul>
				)
			default:
				return(
					<div></div>
				)
		}
	}
}

class App extends Component {
	constructor(props){
		super(props)
		this.state = {
			page: 'home'
		}
	}

  	render() {
		if (this.state.page === 'home'){
			window.history.pushState('home', 'Smart Recycling');
			return (
				<div className="App">
					<header className="App-header">
						<img src={logo} className="App-logo" alt="logo" />
						<h1 className="App-title">Smart Recycling</h1>
					</header>
					<p className="App-intro">
						Homepage
					</p>
					<p className='requirements'>
						Un cassonetto dei rifiuti intelligente dispone di un sensore di imaging, in
						grado di fotografare un rifiuto e, in base a un algoritmo decisionale implementato in
						Cloud, decidere se permetterne lo smaltimento, sbloccando una serratura, o non
						permetterlo (opzionale: tramite un display a video il cassonetto può indicare la
						corretta opzione di smaltimento per i rifiuti non consentiti). Un'applicazione web
						mostrerà, in tempo reale, statistiche sul riconoscimento degli oggetti e permetterà di
						verificare manualmente le immagini inviate. Il progetto sarà implementato su Cloud AWS,
						tramite l'uso dei servizi opportuni. Gli oggetti connessi dovranno supportare il
						concetto di digital twin (gestito tramite Shadow su AWS IoT). I requisti di dettaglio
						sulla funzionalità implementato e/o sulla architettura HW/SW e/o sulla componentistica
						utilizzata verranno concordati con i docenti su proposta degli studenti.
					</p>
					<p className='requirements'>
						Gli studenti del gruppo assegnato a tale progetto dovranno operare a livello
						hardware/software al fine di conseguire l'obiettivo richiesto, accompagnando il loro
						lavoro con idonea documentazione scritta che riporti, in modo chiaro e dettagliato, le
						attività svolte, le problematiche affrontate e la soluzione proposta.
					</p>
					<div className="chart">
						<button id="button_left"
							onClick={
								() => {
									this.setState({page: 'evaluate'});
								}
							}> <span>Go to evaluation</span> </button>

						<button id="button_right"
							onClick={
								() => {
									this.setState({page: 'statistics'});
								}
							}> <span>Go to statistics</span> </button>
					</div>
				</div>
			);
		}
		else if (this.state.page === 'evaluate'){
			return (
				<div className="App">
					<header className="App-header">
						<img className="back" src={back} alt="back" header="50px" width="50px" 
							onClick={
								() => {
									this.setState({page: 'home'});
								}
							}
						/>
						<img src={logo} className="App-logo" alt="logo" />
						<h1 className="App-title">Smart Recycling</h1>
					</header>
					<p className="App-intro">
						Evaluations
					</p>
					<div className="chart">
						<MyChart type = 'List'
							title = 'Total evaluations'
							counter = {[0]}
							generate_data = {
								(element, n, index) => {
									if(element.true_class === undefined){
										var component = React.createElement('li', {key: element.item_id.N.toString()}, 
											React.createElement('div', {className: 'element'},
												React.createElement('div', {className: 'img'},
													React.createElement('img', {height:'200px', width:'200px', src:'https://esit-smart-recycling.s3.amazonaws.com/'+element.file_name.S})
												),
												React.createElement('div', {className: "details"},
													React.createElement('p', {}, "Took on: " + new Date(element.item_id.N * 1000).toDateString(),
														React.createElement('span', {}, 'Classified as: ' + element.assigned_class.S),
														React.createElement('span', {}, 'Image name: ' + element.file_name.S)
													)
												),
												React.createElement('div', {className: 'evalButtons'},
													React.createElement('button', {className: 'acceptButton', onClick: () => evaluatePic(n, true, element)}, 'Accept'),
													React.createElement('button', {className: 'denyButton', onClick: () => evaluatePic(n, false, element)}, 'Deny')
												)
											)
										)

										var item_id = element.item_id.N

										entity[n].state.listItems[item_id] = component
									}
								}	
							}/>
					</div>
				</div>
			);
		}
		else{
			return (
				<div className="App">
					<header className="App-header">
						<img className="back" src={back} alt="back" header="50px" width="50px" 
							onClick={
								() => {
									this.setState({page: 'home'});
								}
							}
						/>
						<img src={logo} className="App-logo" alt="logo" />
						<h1 className="App-title">Smart Recycling</h1>
					</header>
					<p className="App-intro">
						Evaluations statistics
					</p>
					<div className="chart">
						<MyChart type = 'Line'
							title = 'Total evaluations'
							counter={[0, 0, 0, 0, 0]} 
							labels={['Paper', 'Glass', 'Organic', 'Plastic', 'Inorganic']}
							backgroundColor = 'rgba(0, 0, 255, 0.2)'
							borderColor = 'rgba(0, 0, 255, 1)'
							generate_data = {
								(element, n, index) => {
									var i = 4;
									switch(element.assigned_class.S){
										case 'Paper':
											i = 0;
											break;
										case 'Glass':
											i = 1;
											break;
										case 'Organic':
											i = 2;
											break;
										case 'Plastic':
											i = 3;
											break;
										default:
											i = 4;
											break;
									}

									entity[n].state.counter[i]++;
								}	
							}/>

						<MyChart type = 'Timeline'
							title = 'Evaluations per day (last week)'
							counter={[0, 0, 0, 0, 0, 0, 0]} 
							labels={getLabels(new Date().getTime())}
							backgroundColor = 'rgba(255, 150, 0, 0.2)'
							borderColor = 'rgba(255, 150, 0, 1)'
							lineTension = {0}
							generate_data = {
								(element, n, index) => {
									var one_day = 1000 * 60 * 60 * 24
									var today = new Date().getTime()
									var delta = Math.trunc((today - element.item_id.N*1000) / one_day)

									if (delta < 7){
										if (entity[n].state.counter[7 - delta] === undefined) entity[n].state.counter[delta] = 0
										entity[n].state.counter[6 - delta]++;
									}
								}	
							}/>

						<MyChart type = 'Bar'
							title = 'Total evaluations'
							counter={[0, 0, 0, 0, 0]} 
							labels={['Paper', 'Glass', 'Organic', 'Plastic', 'Inorganic']}
							backgroundColor = {['rgba(255, 255, 0, 0.2)', 'rgba(0, 255, 0, 0.2)', 'rgba(200, 200, 200, 0.2)', 'rgba(0, 0, 255, 0.2)', 'rgba(0, 0, 0, 0.2)']}
							borderColor = {['rgba(255, 255, 0, 1)', 'rgba(0, 255, 0, 1)', 'rgba(200, 200, 200, 1)', 'rgba(0, 0, 255, 1)', 'rgba(0, 0, 0, 1)']}
							generate_data = {
								(element, n, index) => {
									var i = 4;
									switch(element.assigned_class.S){
										case 'Paper':
											i = 0;
											break;
										case 'Glass':
											i = 1;
											break;
										case 'Organic':
											i = 2;
											break;
										case 'Plastic':
											i = 3;
											break;
										default:
											i = 4;
											break;
									}
									entity[n].state.counter[i]++;
								}	
							}/>

						<MyChart type = 'Pie'
							title = 'Evaluation correctness'
							counter={[0, 0, 0, 0, 0]} 
							labels={['Correct', 'Wrong', 'Waiting']}
							backgroundColor = {['rgba(0, 200, 0, 0.2)', 'rgba(200, 0, 0, 0.2)', 'rgba(0, 0, 0, 0.2)']}
							borderColor = {['rgba(0, 200, 0, 1)', 'rgba(200, 0, 0, 1)', 'rgba(0, 0, 0, 1)']}
							generate_data = {
								(element, n, index) => {
									var i = 2;
									if(element.true_class){
										switch(element.true_class.BOOL){
											case true:
												i = 0;
												break;
											case false:
												i = 1;
												break;
											default:
												i = 2;
										}
									}
									else{
										i = 2;
									}
									entity[n].state.counter[i]++;
								}	
							}/>
					</div>
				</div>
			);}
	}
}

export default App;