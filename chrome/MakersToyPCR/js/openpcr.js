/* Javascript for OpenPCR
 *
 * http://openpcr.org
 * Copyright (c) 2011 OpenPCR
 */

/*
 * This code is generally broken up into 3 sections, each having to do with the 3 main pages of the OpenPCR interface
 * 1. Home screen + initialization
 * 2. Form screen, entering the PCR protocol
 * 3. Running screen, displaying live information from OpenPCR
 * Extra. Buttons
 */

/**************
 * Home screen*
 ***************/

/* init()
 * Called when the app is loaded.
 * Checks to see if OpenPCR is plugged in (gets the device path if it is) and checks to see if there is an Air update available
 */

// Application Updater code		
// declare the appUpdater variable
console.log("TODO TORI App updates is disabled because Chrome app will be automatically updated by Chrome Web Store.");

var sp2;
var pcrStorage;
function init() {
	console.log("INIT");
	pcrStorage = new Storage();
	$(document).keypress(function(event) {
		return disableEnterKey(event);
	});

	$('#newExperimentButton').on('click', newExperiment);
	console.log($('#newExperimentButton'));
	$('#listSubmitButton').on('click', listSubmit);
	$('#initialStep').on('click', addInitialStep);
	$('#finalStep').on('click', addFinalStep);
	$('#saveForm').on('click', function() {
		$('#Start').click();
	});
	
	prepareButtons();
	sp2 = new Spry.Widget.SlidingPanels('example2');
	// hide Settings button by default
	($("#Settings").hide());

	// get the location of OpenPCR (can be null)
	var deviceLocation = pluggedIn(function(port) {
		var result = !!port;
		var portMessage = (result)?("Device found on port " + port):"Device not found";
		$("#portLabel").html(portMessage);
		
		if (result) {
			window.pluggedIn = true;
			console.log("Set #Start button visible.");
			if($("#Unplugged").is(':visible')){
				// if the "Unplugged" button is visible, switch it to "Start"
				$("#Unplugged").hide();
				$("#Start").show();
				//Read device
			}
		} else {
			// Not plugged
		}

		listExperiments();
		$("#pcrForm").validate();
	});

}

/* listExperiments()
 * Updates the list of Saved Experiments on the home page.
 * Grabs all the files in the Experiments folder and lists them alphabetically
 *
 */
function listExperiments() {
	pcrStorage.loadList(function(experiments) {
		presetsHTML = "<select id='dropdown'>";
		if (experiments && experiments.length > 0) {
			for ( var i = 0; i < experiments.length; i++) {
				var experiment = experiments[i];
				if (experiment.id && experiment.name) {
					presetsHTML += '<option value="' + experiment.id + '">'
							+ experiment.name + "</option>";
				}
			}
		}
		// if blank, add a "No Saved Experiments" item
		if (presetsHTML == "<select id='dropdown'>") {
			presetsHTML += '<option value=1>-none-</option>';
		}

		// close the drop down HTML tags
		presetsHTML += "</select>";
		// update the HTML on the page
		$("#reRun").html(presetsHTML);

	});
}

/* listSubmit()
 * Loads the selected experiment in the list on the home page
 */
function listSubmit() {
	// what is selected in the drop down menu?
	experimentID = $("#dropdown").val();
	// load the selected experiment
	loadExperiment(experimentID);
}

/* pluggedIn()
 * Checks that a volume named "OpenPCR" is mounted on the computer
 * Sets up 2 listeners (MOUNT and UNMOUNT) and then checks to see if OpenPCR is already mounted
 * Returns: deviceLocation (null if not plugged in)
 */
function pluggedIn(callback) {
	console.log("TODO TORI pluggedIn");
	chromeSerial.scan(callback);
}

/* loadExperiment();
 * loads the experiment with the given experimentID
 */
function loadExperiment(experimentID) {
	console.log("loadExperiment id=" + experimentID);
	pcrStorage.loadExperiment(experimentID, function(experiment) {
		// Now we've made all the modifications needed, display the Form page
		sp2.showPanel(1);
		// clear the experiment form
		clearForm();
		// read in the file
		experimentJSON = experiment;
		// loads filen into the Form and moves onto Form page
		experimentToHTML(experimentJSON);
		// update the buttons to make sure everything is ready to re-run an experiment
		reRunButtons();
	});
}

/*  newExperiment()
 * This function is called when the "New Experiment" button is clicked on the Home page
 * This function brings up a blank experiment
 */
function newExperiment() {
	// clear the experiment form
	clearForm();
	// set up the blank experiment
	experimentToHTML(NEW_EXPERIMENT);

	// set interface to have the right buttons
	newExperimentButtons();
	// Now we've made all the modifications needed, display the Form page
	sp2.showPanel(1);
}

/**************
 * Form screen*
 ***************/

/* startOrUnplugged(display)
 * Determines whether to display the "Start" or "Unplugged" button on the Form page.
 * Input: CSS display status of the button
 * Returns: nothing
 */
function startOrUnplugged(display) {
	//pick the Start or Unplugged button based on whether the device is plugged in or not
	// if plugged in then
	if (window.pluggedIn == true) {
		// then we definitely want to hide the "Unplugged" button
		$("#Unplugged").hide();
		// and maybe want to show/hide the "Start" button, whatever was submitted as the "display" var
		$("#Start").css("display", display)
		// and, change the running screen to plugged in
		$("#runningUnplugged").hide();
		$("#runningPluggedIn").show();
	}
	else {
		// else, device is unplugged
		// then we definitely want to hide the "Start" button
		$("#Start").hide();
		// and maybe want to show/hide the "Unplugged" button, whatever was submitted as the "display" var
		$("#Unplugged").css("display", display)
		// change the running screen to unplugged
		$("#runningUnplugged").show();
		$("#runningPluggedIn").hide();
	}
}

/* reRunButtons()
 * puts Form buttons in the state they should be immediately following loading an experiment
 */
function reRunButtons() {
	// Hide the Delete button
	$('#deleteButton').hide();
	// Start with the edit button shown
	$("#editButton").show();
	// Start with the edit buttons hidden
	$(".edit").hide();
	// hide the lid temp fields
	$("#lidContainer").hide();
	// all fields locked
	$("input").attr("readonly", "readonly");
	// and 'More options' hidden
	$('#OptionsButton').hide();
	// Hide the Save button
	$('#Save').hide();
	// Hide the Cancel button
	$('#Cancel').hide();
	// Hide the SaveEdits button
	$('#SaveEdits').hide();
	// Show the Start/Unplugged button
	startOrUnplugged("inline");
	$('#singleTemp').hide();
	// pre and post containers should take care of themselves
}

/* newExperimentButtons()
 * puts Form buttons in the state they should be for a new experiment
 */
function newExperimentButtons() {
	// Hide the Delete button
	$('#deleteButton').hide();
	// Start with the edit button hidden
	$("#editButton").hide();
	// Start with the edit buttons hidden
	$(".edit").hide();
	// lid temp hidden
	$("#lidContainer").hide();
	// all fields editable
	$("input").removeAttr("readonly");
	// and 'More options' shown
	$('#OptionsButton').show();
	// Show the Save button
	$('#Save').show();
	// Hide the Cancel button
	$('#Cancel').hide();
	// Hide the SaveEdits button
	$('#SaveEdits').hide();
	// Show the Start/Unplugged button
	startOrUnplugged("inline");
	$('#singleTemp').hide();
	// make sure the "More options" button says so
	$('#OptionsButton').html("More options");
}

/* writeoutExperiment
 * Reads out all the variables from the OpenPCR form into a JSON object to "Save" the experiment
 * Separate function is used to write out the experiment to the device
 */
function writeoutExperiment() {
	// grab the Experiment Name
	experimentName = document.getElementById("ExperimentName").innerHTML;

	// grab the pre cycle variables if any exist
	preArray = [];
	$("#preContainer .textinput").each(function(index, elem) {
		//just throw them in an array for now
		if ($(this) != null)
			preArray.push($(this).val());
	});

	// grab the cycle variables
	cycleArray = [];
	$("#cycleContainer .textinput").each(function(index, elem) {
		//just throw them in an array for now
		cycleArray.push($(this).val());
	});

	// grab the post cycle variables if any exist
	postArray = [];
	$("#postContainer .textinput").each(function(index, elem) {
		//just throw them in an array for now
		postArray.push($(this).val());
	});

	// grab the final hold steps if any exist
	holdArray = [];
	$("#holdContainer .textinput").each(function(index, elem) {
		//just throw them in an array for now
		holdArray.push($(this).val());
	});
	// grab the lid temp
	$("#lidContainer .textinput").each(function(index, elem) {
		lidTemp = $(this).val();
	});

	// Push variables into an experiment JSON object
	var experimentJSON = new Object();
	// Experiment name
	experimentJSON.name = experimentName;
	experimentJSON.steps = [];
	experimentJSON.lidtemp = lidTemp;
	// Pre Steps
	// every step will have 3 elements in preArray (Time, temp, rampDuration)
	preLength = (preArray.length) / 3;
	for (a = 0; a < preLength; a++) {
		experimentJSON.steps.push({
			"type" : "step",
			"name" : "Initial Step",
			"temp" : preArray.shift(),
			"time" : preArray.shift(),
			"rampDuration" : preArray.shift()
		});
	}

	// Cycle and cycle steps
	// the cycle will be a # of cycles as the first element, then temp/time pairs after that
	count = cycleArray.shift();
	if (cycleArray.length > 0 && count > 0) {
		experimentJSON.steps.push({
			"type" : "cycle",
			// add the number of cycles
			"count" : count,
			"steps" : []
		});

		// then add the cycles
		current = experimentJSON.steps.length - 1;

		// every step will have 3 elements in cycleArray (Time, temp, rampDuration)
		cycleLength = (cycleArray.length) / 3;
		for (a = 0; a < cycleLength; a++) {

			experimentJSON.steps[current].steps.push({
				"type" : "step",
				"name" : "Step",
				"temp" : cycleArray.shift(),
				"time" : cycleArray.shift(),
				"rampDuration" : cycleArray.shift()
			});
		}
	}

	// every step will have 3 elements in preArray (Time, temp, rampDuration)
	// a better way to do this would be for a=0, postArray!=empty, a++
	postLength = (postArray.length) / 3;
	for (a = 0; a < postLength; a++) {
		experimentJSON.steps.push({
			"type" : "step",
			"name" : "Final Step",
			"temp" : postArray.shift(),
			"time" : postArray.shift(),
			"rampDuration" : postArray.shift()
		});
	}

	// Final Hold step
	if (holdArray.length > 0) {
		experimentJSON.steps.push({
			"type" : "step",
			"name" : "Final Hold",
			"time" : 0,
			"temp" : holdArray.shift(),
			"rampDuration" : 0
		});
	}

	// return the experiment JSON object
	return experimentJSON;
}

/* Save(name)
 * Writes out the current window.experiment to the app:/Experiments directory
 * Input: name, name of the file to be written out (add .pcr extension)
 */
function Save(name, isNew) {
	console.log("Save " + name + ", isNew=" + isNew);
	// grab the current experiment and update window.experiment
	pcrProgram = writeoutExperiment();
	// update the name of the experiment
	pcrProgram.name = name;
	// turn the pcrProgram into a string
	if (isNew) {
		pcrStorage.insertExperiment(name, pcrProgram, function(result) {
			console.log("result=" + result);
			$('#save_confirmation_dialog').dialog('open');
			// then close it after 1 second
			setTimeout(function() {
				$('#save_confirmation_dialog').dialog('close');
			}, 750);
		});
	}
	else {
		pcrStorage.updateCurrentExperiment(name, pcrProgram, function(result) {
			console.log("result=" + result);
			$('#save_confirmation_dialog').dialog('open');
			// then close it after 1 second
			setTimeout(function() {
				$('#save_confirmation_dialog').dialog('close');
			}, 750);
		});
	}
}

/* experimentToHTML(inputJSON)
 * Takes a given experiment JSON object and loads it into the OpenPCR interface
 */
function experimentToHTML(inputJSON) {
	// store the experiment to the JSON. This can be modified using the interface buttons, sent to OpenPCR, or saved
	window.experiment = inputJSON;
	console.log("experimentToHTML 1");
	// clear the Form
	clearForm();
	// Update the experiment name
	var experimentName = inputJSON.name;
	// only use the first 20 chars of the experimentName
	experimentName = experimentName.slice(0, 18);
	$("#ExperimentName").html(experimentName);

	// for every .steps in the experiment, convert it to HTML
	var experimentHTML = "";
	// break the rest of the experiment up into "pre cycle" (0), "cycle" (1), and "post cycle" (2) sections
	var count = 0;
	// add the lid temperature div but hide it
	$('#lidContainer').hide();
	// max temp 120, min temp 0 (off)
	$('#lidTemp')
			.html(
					'<span class="title">Heated Lid</span>'
							+ '<input type="text" name="lid_temp" id="lid_temp" class="required integer textinput" maxlength="3" min="0" max="120"  value="'
							+ inputJSON.lidtemp + '">');
	// 4 possibile DIVs
	// pre-steps, cycle steps, post-steps, and final hold step
	// Add the experiment to the page	
	for (i = 0; i < inputJSON.steps.length; i++) {
		// pre-cycle to start
		if (count == 0 & inputJSON.steps[i].type == "step"
				&& inputJSON.steps[i].time != 0)
		// if it's for pre-cycle, and not a final hold (0 time)
		{
			// show the preContainer div
			$('#preContainer').show();
			$('#preSteps').append(stepToHTML(inputJSON.steps[i]))
		}

		else if (count == 0 && inputJSON.steps[i].type == "cycle")
		// if it's cycle, put the cycle in the Cycle container
		{
			$('#cycleContainer').show();
			$('#cycleSteps').append(stepToHTML(inputJSON.steps[i]));
			count = 1;
		}

		else if (count == 1 && inputJSON.steps[i].type == "step"
				&& inputJSON.steps[i].time != 0)
		// if it's post (but not a final hold), put the steps in the Post container
		{
			$('#postContainer').show();
			$('#postSteps').append(stepToHTML(inputJSON.steps[i]));
		}

		else if (inputJSON.steps[i].type == "step"
				&& inputJSON.steps[i].time == 0)
		// if it's the final hold (time = 0), put it in the final hold container
		{
			$('#holdContainer').show();
			$('#holdSteps').append(stepToHTML(inputJSON.steps[i]));
		}

	}
	activateDeleteButton();
}

/* stepToHTML(step)
 * Turns a step into HTML. However, this HTML doesn't have a container div/fieldset
 * If the step is a cycle, it will return html with all the cycles represented.
 * If the step is a single step, html with just one cycle is returned
 */

function stepToHTML(step) {
	stepHTML = "";
	// if cycle
	if (step.type == "cycle") {
		// printhe "Number of Cycles" div
		// max 99 cycles
		stepHTML += '<label for="number_of_cycles"></label><div><span class="title">Number of Cycles:</span><input type="text" name="number_of_cycles" id="number_of_cycles" class="required number textinput" maxlength="2" min="0" max="99"  value="'
				+ step.count + '"></div><br />';
		// steps container
		// print each individual step
		for (a = 0; a < step.steps.length; a++) {
			// make the js code a little easier to read
			step_number = a;
			step_name = step.steps[a].name;
			step_temp = step.steps[a].temp;
			step_time = step.steps[a].time;
			step_rampDuration = step.steps[a].rampDuration;
			if (step_rampDuration == null)
				step_rampDuration = 0;

			// print HTML for the step
			// min,max temp = -20, 105
			// min,max time = 0, 6000, 1 decimal point
			stepHTML += '<div class="step"><span id="step'
					+ step_number
					+ '_name" class="title">'
					+ step_name
					+ ' </span><a class="edit deleteStepButton"><img src="images/minus.png" height="30"></a>'
					+ '<table><tr>'
					+ '<th><label for="step'
					+ step_number
					+ '_temp">temp:</label> <div class="step'
					+ step_number
					+ '_temp"><input type="text" style="font-weight:normal;" class="required number textinput" name="step'
					+ step_number
					+ '_temp" id="step'
					+ step_number
					+ '_temp" value="'
					+ step_temp
					+ '" maxlength="4" min="-20" max="120" ></div><span htmlfor="openpcr_temp" generated="true" class="units">&deg;C</span> </th>'
					+ '<th><label for="step'
					+ step_number
					+ '_time">step duration:</label> <div class=""><input type="text" class="required number textinput"  style="font-weight:normal;" name="step'
					+ step_number
					+ '_time" id="step'
					+ step_number
					+ '_time" value="'
					+ step_time
					+ '" maxlength="4" min="0" max="6000"  ></div><span htmlfor="openpcr_time" generated="true" class="units">sec</span></th>'
					+ '<th><label for="step'
					+ step_number
					+ '_rampDuration">ramp duration:</label> <div class=""><input type="text" class="required number textinput"  style="font-weight:normal;" name="step'
					+ step_number
					+ '_rampDuration" id="step'
					+ step_number
					+ '_rampDuration" value="'
					+ step_rampDuration
					+ '" maxlength="6" min="0" max="999999"  ></div><span htmlfor="openpcr_rampDuration" generated="true" class="units">sec</span></th>'
					+ '</tr></table></div>';

		}
	}
	// if single step
	else if (step.type == "step") {
		// make the js code a little easier to read
		step_number = new Date().getTime();
		step_name = step.name;
		step_time = step.time;
		step_temp = step.temp;
		step_rampDuration = step.rampDuration;
		if (step_rampDuration == null)
			step_rampDuration = 0;

		// main HTML, includes name and temp
		stepHTML += '<div class="step"><span id="'
				+ step_number
				+ '" class="title">'
				+ step_name
				+ ' </span><a class="edit deleteStepButton"><img src="images/minus.png" height="30"></a>'
				+ '<table cellspacing="20"><tr>'
				+ '<th><label>temp:</label> <div><input type="text" style="font-weight:normal;" class="required number textinput" value="'
				+ step_temp
				+ '" maxlength="4" name="temp_'
				+ step_number
				+ '" min="0" max="120" ></div><span htmlfor="openpcr_temp" generated="true" class="units">&deg;C</span> </th>';

		// if the individual step has 0 time (or blank?) time, then it is a "hold" step and doesn't have a "time" component
		if (step_time != 0) {
			stepHTML += '<th><label>step duration:</label> <div class=""><input type="text" class="required number textinput" style="font-weight:normal;" value="'
					+ step_time
					+ '" name="time_'
					+ step_number
					+ '" maxlength="4" min="0" max="6000"></div><span htmlfor="openpcr_time" generated="true" class="units">sec</span></th>';
			stepHTML += '<th><label>ramp duration:</label> <div class=""><input type="text" class="required number textinput" style="font-weight:normal;" value="'
					+ step_rampDuration
					+ '" name="rampDuration_'
					+ step_number
					+ '" maxlength="6" min="0" max="999999"></div><span htmlfor="openpcr_rampDuration" generated="true" class="units">sec</span></th>';
		}
	}
	else
		chromeUtil.alert("Error #1986");
	stepHTML += '</tr></table></div>';
	return stepHTML;
}

/* stepToString(inputJSON)
 * Takes a JSON object and turns it into a string
 * This is used to load an experiment into the OpenPCR device
 */
function stepToString(inputJSON) {
	var stepString = "";
	// if single step return something like (1[300|95|Denaturing])
	if (inputJSON.type == "step") {
		//			stepString += "[" + inputJSON.time + "|" + inputJSON.temp + "|" + inputJSON.name.slice(0,13) + "]";			
		stepString += "[" + inputJSON.time + "|" + inputJSON.temp + "|"
				+ inputJSON.name.slice(0, 13) + "|" + inputJSON.rampDuration
				+ "]";
	}
	// if cycle return something like (35,[60|95|Step A],[30|95|Step B],[30|95|Step C])
	else if (inputJSON.type == "cycle") {
		// add the number of Cycles
		stepString += "(";
		stepString += inputJSON.count;

		for (a = 0; a < inputJSON.steps.length; a++) {
			//						stepString += "[" + inputJSON.steps[a].time + "|" + inputJSON.steps[a].temp + "|" + inputJSON.steps[a].name.slice(0,13) + "]";
			stepString += "[" + inputJSON.steps[a].time + "|"
					+ inputJSON.steps[a].temp + "|"
					+ inputJSON.steps[a].name.slice(0, 13) + "|"
					+ inputJSON.steps[a].rampDuration + "]";
		}
		// close the stepString string
		stepString += ")";
	}
	//alert(stepString);
	return stepString;
}

/* clearForm()
 * Reset all elements on the Forms page
 */
function clearForm() {
	// empty everything
	$('#preSteps').empty();
	$('#cycleSteps').empty();
	$('#postSteps').empty();
	$('#holdSteps').empty();
	$('#lidTemp').empty();
	// hide everything
	$('#preContainer').hide();
	$('#cycleContainer').hide();
	$('#postContainer').hide();
	$('#holdContainer').hide();
	$('#lidContainer').hide();

	// reset the size of the DIV to 700 px
	//defaultHeight = "700";
	//$(".SlidingPanelsContent").height(defaultHeight);
	//$(".SlidingPanels").height(defaultHeight);
}

/* disableEnterKey(e)
 * The Enter/Return key doesn't do anything right now
 */
function disableEnterKey(e) {
	console.log("disableEnterKey");
	var key;
	if (window.event)
		key = window.event.keyCode; //IE
	else
		key = e.which; //firefox      

	return (key != 13);
}

function startPCR() {
	experimentLog = [];
	// check if the form is validated
	if (false == ($("#pcrForm").validate().form())) {
		return 0;
	} // if the form is not valid, show the errors
	// command_id will be a random ID, stored to the window for later use
	window.command_id = Math.floor(Math.random() * 65534);
	// command id can't be 0 
	// where is OpenPCR
	var devicePort = chromeSerial.getPort();
	console.log("devicePort=" + devicePort);
	/*
	// name of the output file written to OpenPCR
	var controlFile = devicePath.resolvePath("CONTROL.TXT");
	console.log("controlFile=" + controlFile);
	// grab all the variables from the form in JSON format
	*/
	pcrProgram = writeoutExperiment();
	// now parse it out
	// Start with the signature
	var parsedProgram = "s=ACGTC";
	// Command
	parsedProgram += "&c=start";
	// Contrast
	//// contrast no longer controlld here, delete this
	////parsedProgram += "&t=50";
	// Command id 
	parsedProgram += "&d=" + window.command_id;
	// Lid Temp NO DECIMALS. Not handeled by UI currently, but just making sure it doesn't make it to OpenPCR
	parsedProgram += "&l=" + Math.round(pcrProgram.lidtemp);
	// Name
	parsedProgram += "&n=" + pcrProgram.name
	// get all the variables from the pre-cycle, cycle, and post-cycle steps
	parsedProgram += "&p=";
	window.lessthan20steps = 0;
	for (i = 0; i < pcrProgram.steps.length; i++) {
		if (pcrProgram.steps[i].type == "step")
		// if it's a step, stepToString will return something like [300|95|Denaturing]
		// then this loop needs to figure out when to add [1(  and )]
		{
			// if the previous element wasn't a step (i.e. null or cycle)
			if (typeof pcrProgram.steps[i - 1] == 'undefined'
					|| pcrProgram.steps[i - 1].type == "cycle") {
				parsedProgram += "(1";
			}

			parsedProgram += stepToString(pcrProgram.steps[i]);

			// if the next element isn't a step (i.e. null or cycle)
			if (typeof pcrProgram.steps[i + 1] == 'undefined'
					|| pcrProgram.steps[i + 1].type != "step") {
				parsedProgram += ")";
			}
		}

		else if (pcrProgram.steps[i].type == "cycle")
		// if it's a cycle add the prefix for the number of steps, then each step
		{
			// for example, this should return (35[30,95,Denaturing][60,55,Annealing][60,72,Extension])
			parsedProgram += stepToString(pcrProgram.steps[i]);
			window.lessthan20steps = pcrProgram.steps[i].steps.length;
		}
	}
	console.log("parsedProgram=" + parsedProgram);
	// verify that there are no more than 16 top level steps
	console.log(pcrProgram.steps.length + " : top level steps");
	console.log(window.lessthan20steps + " : cycle level steps");
	var totalSteps = window.lessthan20steps + pcrProgram.steps.length;

	// check that the entire protocol isn't >252 bytes
	console.log("parsedProgram=" + parsedProgram);
	if (parsedProgram.length > 252) {
		chromeUtil
				.alert("Oops, OpenPCR can't handle protocols longer than 252 characters, and this protocol is "
						+ parsedProgram.length
						+ " characters. The fix? You can try trimming down the name of your protocol or removing unnecessary steps");
		return 0;
	}

	// and check there aren't more than 16 steps at the top level

	// this is wrong
	// there can be up to 16 initial, 16 cycle, and 16 final steps, but the total can't be more than 30
	// else if (pcrProgram.steps.length > 16)
	// 			{
	// 				console.log(parsedProgram);
	// 			alert("OpenPCR can handle a maximum of 16 top-level steps, you have " + pcrProgram.steps.length + " steps");
	// 				return 0;
	// 			}
	// 			

	// verify the cycle step has no more than 16 steps
	else if (window.lessthan20steps > 16) {
		console.log(parsedProgram);
		chromeUtil
				.alert("OpenPCR can handle a maximum of 20 cycle steps, you have "
						+ window.lessthan20steps + " steps");
		return 0;
	}

	// and check that the total overall is less than 25
	else if (totalSteps > 25) {
		console.log(parsedProgram);
		chromeUtil
				.alert("OpenPCR can handle a maximum of 25 total steps, you have "
						+ totalSteps + " steps");
		return 0;
	}

	//debug
	console.log(parsedProgram);
	// go to the Running dashboard
	sp2.showPanel(2);
	$("#ex2_p3").hide();
	// go to the top of the page
	scrollTo(0, 0);
	//hide the home button on the running page
	$("#homeButton").hide();
	$("#download").hide();
	// show the "stop" button
	$("#cancelButton").show();
	$('#starting').dialog('open');
	// write out the file to the OpenPCR device
	chromeSerial.startWithCommand(parsedProgram);
	running();
	
	// then close windows it after 1 second
	setTimeout(function() {
		$('#starting').dialog('close');
	}, 5000);
	setTimeout(function() {
		$('#ex2_p3').show();
	}, 5000);
	// also, reset the command_id_counter
	window.command_id_counter = 0;
	// load the OpenPCR Running page
	//running(path);
}

/*****************
 * Running screen *
 ******************/

/* running(path)
 * Controls the "running" page of OpenPCR. Reads updates from the running.pcr control file on OpenPCR continuously
 * Input: path, the location of the running.pcr control file
 */

function running() {
	// refresh the running page every 1000 ms
	window.updateRunningPage = setInterval(updateRunning, 1000);
}

/* updateRunning()
 * Updates the Running page variables
 */

function updateRunning() {
	chromeSerial.requestStatus(onReceiveStatus);
}
var experimentLog;

var COLOR_HEATING = "red";
var COLOR_COOLING  = "blue";
var COLOR_STOP = "black";

// Process Status update
function onReceiveStatus(message) {
	{
		// split on &
		var splitonAmp = message.split("&");
		// split on =
		var status = new Array();
		for (i = 0; i < splitonAmp.length; i++) {
			var data = splitonAmp[i].split("=");
			if (isNaN(parseFloat(data[1]))) {
				// not a number
				status[data[0]] = data[1];
			}
			else {
				// a number
				status[data[0]] = parseFloat(data[1]);
			}
		}

		// make sure the status isn't blank
		// if command id in the running file doesn't match, check again 50 times and then quit if there is still no match
		if (status["d"] != window.command_id) {
			if (window.command_id_counter > 50) {
				//alert("OpenPCR command_id does not match running file, window.command_id_counter =" + window.command_id_counter +  " . This error should not appear\nstatus"+status["d"]+"\nwindow:"+window.command_id);
				// quit
				//air.NativeApplication.nativeApplication.exit();
			}
			window.command_id_counter++;
			// debug
		}
		// if app command id matches the device command id, reset the counter
		if (status["d"] == window.command_id) {
			window.command_id_counter = 0;
		}
		//if (Math.random()<0.05) status["s"]="complete"; //TODO debug

		var statusLid = status["x"].toFixed(1);
		var statusPeltier = status["y"].toFixed(1);
		$("#deviceStatusLid").html((statusLid>0)?"Heating":"Stop");
		$("#deviceStatusLid").css("color", (statusLid>0)?COLOR_HEATING:COLOR_STOP);
		{
			var color;
			var text;
			if (statusPeltier>0) {
				color = COLOR_HEATING;
				text = "Heating";
			}
			else if (statusPeltier<0) {
				color = COLOR_COOLING;
				text = "Cooling"
			} 
			else {
				color = COLOR_STOP;
				text = "Stop";
			}
			$("#deviceStatusPeltier").html(text);
			$("#deviceStatusPeltier").css("color",color);
			
		}
		
		if (status["s"] == "running" || status["s"] == "lidwait") {
			//debug
			// preset name
			var prog_name = status["n"];
			$("#runningHeader").html(prog_name);

			if (status["s"] == "lidwait") {
				// if the lid is heating say so
				$("#progressbar").hide();
				$("#cycleNumOfNum").hide();
				$("#timeRemaining").html("");
				$("#minutesRemaining").html("Lid is heating");

				// during lidwait, no protocol name is included, so include the protocol name from the previous page
				$("#runningHeader").html(
						document.getElementById("ExperimentName").innerHTML);
			}

			if (status["s"] == "running") {
				$("#timeRemaining").html("Time remaining:");
				// otherwise, if running set variable for percentComplete
				// never display less than 2% for UI purposes
				var percentComplete = 100 * status["e"]
						/ (status["e"] + status["r"]);
				if (percentComplete < 2) {
					percentComplete = 2;
				}
				// Progress bar
				$("#progressbar").progressbar({
					value : percentComplete
				});
				$("#progressbar").show();

				// Time Remaining
				var secondsRemaining = status["r"];
				var timeRemaining = humanTime(secondsRemaining);
				$("#minutesRemaining").html(timeRemaining);
			}
			// Current step name
			var current_step = status["p"];
			$("#currentStep").html(current_step);

			// Current cycle # of #
			$("#cycleNumOfNum").show();

			var current_cycle = status["c"];
			$("#cycleNumber").html(current_cycle);
			// Total # of cycles
			var total_cycles = status["u"];
			$("#totalCycles").html(total_cycles);

			// Current temp
			var block_temp = status["b"].toFixed(1);
			$("#blockTemperature").html(block_temp);

			// Current lid temp
			var lid_temp = status["l"].toFixed(1);
			$("#lidTemperature").html(lid_temp);
			// For the debugger, write all 8 vars out to the history file
			//writeCSV(document.getElementById("runningHeader").innerHTML, document.getElementById("minutesRemaining").innerHTML, 1, document.getElementById("cycleNumber").innerHTML, document.getElementById("totalCycles").innerHTML, document.getElementById("blockTemp").innerHTML, document.getElementById("lidTemp").innerHTML, document.getElementById("progressbar").innerHTML);
			//writeCSV(prog_name, status["e"], secondsRemaining, 1, current_cycle, total_cycles, block_temp, lid_temp);
			//for (var t=0; t<10; t++) { //Performance Test
				graph.addTime(lid_temp,block_temp);
				
			//}
			$('#meterBlock')[0].value = block_temp;
			$('#meterLid')[0].value = lid_temp;
		}
		else if (status["s"] == "complete") {
			chromeSerial.stopOnComplete();
			// if the status of OpenPCR comes back as "complete"		
			// show the "Home" button
			$("#homeButton").show();
			// hide the cancel button
			$("#cancelButton").hide();
			// hide timeRemaining
			$("#timeRemaining").hide()
			// finish the progress bar
			$("#progressbar").progressbar({
				value : 100
			});
			// show the completed message
			minutesRemaining = '<span style="color:#04B109;">Done!</span>';
			$("#minutesRemaining").html(minutesRemaining);
			// update the "current temp"
			var block_temp = status["b"];
			$("#blockTemperature").html(block_temp);
			// update the lid temp
			var lid_temp = status["l"];
			$("#lidTemperature").html(lid_temp);
			// replace the "cycle # of total#" span with "PCR took..."
			$("#cycleNumOfNum").html("PCR took " + humanTime(status["e"]));
			// i.e. hide the "Holding for 10 sec", just show "Holding"
			$("#stepRemaining").hide();
			// Current step name
			var current_step = status["p"];
			$("#currentStep").html(current_step);
			createCSV();
		}
		else if (status["status"] == "stopped") {
			// nothing
		}
		else if (status["status"] == "error") {
			// error
			chromeUtil.alert("Error");
		}
		// add to experimentLog;
		experimentLog.push(status);
	}

	// Show the Settings button if the status file contains "o="
	var contrast_pattern = /o=/;
	// grab the text from STATUS.TXT

	status_string = message;

	// if it contains "o=", show the settings button, otherwise don't
	if (contrast_pattern.test(status_string)) {
		($("#Settings").show());
	}
	else {
		console.log("doesn't have an o");
		console.log(window.runningFile);
	}

}

/* readDevice()
 * Checks the OS (Mac or PC) and runs the appropriate middleman app (NCC) to grab info off the USB drive
 */
function readDevice(filePath) {
	if (filePath.exists) {
		// are native processes supported?
		if (air.NativeProcess.isSupported) {
			var nativeProcessStartup = new air.NativeProcessStartupInfo();
			nativeProcess = new air.NativeProcess();
			// setup arguments
			var args = new air.Vector["<String>"]();
			// PC or Mac?
			if (air.Capabilities.os.toLowerCase().indexOf("win") > -1) {
				processName = air.File.applicationDirectory
						.resolvePath("ncc.exe");
				//alert("win");
			}
			else if (air.Capabilities.os.toLowerCase().indexOf("mac") > -1) {
				// in application directory
				processName = air.File.applicationDirectory.resolvePath("ncc");
				//processName = new air.File("/bin/cat");
				//alert("mac");
			}
			else if (air.Capabilities.os.toLowerCase().indexOf("Linux") > -1) {
				// in application directory
				processName = air.File.applicationDirectory
						.resolvePath("ncc_linx");
				//processName = new air.File("/bin/cat");
				//alert("linux");
			}
			else {
				chromeUtil
						.alert("Error #810 - Can't tell if this is a Mac, PC, or Linux");
			}

			nativeProcessStartup.executable = processName;

			// add the path as an argument
			args.push(filePath.nativePath);
			nativeProcessStartup.arguments = args;
			nativeProcess.addEventListener(
					air.ProgressEvent.STANDARD_OUTPUT_DATA, outputHandler);
			// start the process
			nativeProcess.start(nativeProcessStartup);
		}
		else {
			chromeUtil.alert("NativeProcess not supported");
		}
		// will return the value of the outputHandler if it's NULL or blank
		if (window.deviceFile != null || window.deviceFile != "") {
			return window.deviceFile;
		}
	}
	else {
		// otherwise do nothing if the file doesn't exist
		//alert("File doesn't exist");
	}
}

/* outputHandler()
 * Grabs stdout from the middleman USB app, used in readDevice()
 */
function outputHandler(event) {
	window.deviceFile = nativeProcess.standardOutput
			.readUTFBytes(nativeProcess.standardOutput.bytesAvailable);
}


/* StopPCR()
 * This function is called when the Stop button (Running page) is clicked and confirmed
 * Or when the "Return to home screen" button is clicked
 * Returns: boolean
 */
function stopPCR() {
	// Stop reading the STATUS.TXT file

	// Clear the values in the Running page
	$("#runningHeader").html("");
	$("#progressbar").progressbar({
		value : "0"
	});
	$("#minutesRemaining").html("");

	// Create the string to write out
	var stopPCR = 's=ACGTC&c=stop';
	// contrast
	//// contrast no longer controlled here, delete
	////stopPCR += '&t=50';
	// increment the window.command id and send the new command to the device
	window.command_id++;
	stopPCR += '&d=' + window.command_id;
	console.log(stopPCR);
	// Write out the STOP command to CONTROL.TXT
	// name of the output file
	//var file = window.path.resolvePath("CONTROL.TXT");
	// write out all the variables, command id + PCR settings
	/*var fileStream = new window.runtime.flash.filesystem.FileStream();
	fileStream.open(file, window.runtime.flash.filesystem.FileMode.WRITE);
	fileStream.writeUTFBytes(stopPCR);
	fileStream.close();
	*/
	chromeSerial.sendStopCommand(stopPCR, function(){
	});
	// go back to the Form page
	sp2.showPanel(1);
	return false;
}

/* humanTime()
 * Input: seconds (integer)
 * Returns: time in a human friendly format, i.e. 2 hours, 10 minutes, 1 hour, 10 minutes, 1 hour, 1 minute, 60 minutes, 1 minute
 */
function humanTime(secondsRemaining) {
	var timeRemaining = "";
	var minutesRemaining = Math.floor(secondsRemaining / 60);
	var hoursRemaining = Math.floor(minutesRemaining / 60);
	if (hoursRemaining > 0) {
		timeRemaining += hoursRemaining + " hour";
		if (hoursRemaining > 1) {
			timeRemaining += "s ";
		}
		else {
			timeRemaining += " ";
		}
		timeRemaining += "<br />";
		minutesRemaining -= (hoursRemaining) * 60;
	}
	if (minutesRemaining > 1) {
		timeRemaining += minutesRemaining + " minutes";
	}
	else if (minutesRemaining == 1) {
		timeRemaining += "1 minute";
	}
	else if (secondsRemaining <= 60) {
		// should say "less than a minute" but font is too big
		timeRemaining += "1 minute";
	}
	else if (secondsRemaining == 0) {
		timeRemaining = "Done!";
	}
	return timeRemaining;
}

function _deleteStep () {
	$(this).parent().slideUp('slow', function() {
		// after animation is complete, remove parent step
		$(this).remove();
		//// if the length is now 0, hide the whole div
	});
	
}

function activateDeleteButton() {
	$('.deleteStepButton').on('click', function() {
		$(this).parent().slideUp('slow', function() {
			// after animation is complete, remove parent step
			$(this).remove();
			//// if the length is now 0, hide the whole div
		});

	});
} 
/**************
 * Buttons     *
 ***************/
function prepareButtons() {
	
	//$('#download').on('click', saveCSV);
	/*  "About" button on the OpenPCR Home page
	 * Displays about info
	 */
	$('#About').on('click', function() {
		$('#about_dialog').dialog('open');
	});

	/*  "Contrast" button on the OpenPCR Home page
	 * Sets the contrast for OpenPCR
	 */
	$('#Settings').on('click', function() {
		$('#settings_dialog').dialog('open');
	});

	/*  "Home" button on the OpenPCR Form page
	 * Goes Home
	 */
	$('#Home').on('click', function() {
		listExperiments();
		sp2.showPanel(0);
		setTimeout(clearForm, 500);
	});

	/*  "Home" button on the OpenPCR Running page */
	$('#homeButton').on('click', function() {
		stopPCR();
		listExperiments();
		sp2.showPanel(0);
		setTimeout(clearForm, 500);
	});

	/*  "Start" button on the OpenPCR Form page
	 * Sends an experiment to OpenPCR and switches to the Running page
	 */
	$('#Start').on('click', function() {
		startPCR();
	});

	/*  "Save" button on the OpenPCR Form
	 * Ask for a "name" and save the protocol to name.pcr in the user's Experiments folder
	 */
	$('#Save').on('click', function() {
		// Save Dialog
		// check if the form is validated
		if (false == ($("#pcrForm").validate().form())) {
			return 0; // if not, don't do anything
		}
		// otherwise, the form is valid. Open the "Save" dialog box
		$('#save_form').dialog('open');
	});

	/*  "Save" on the OpenPCR Form in EDIT MODE
	 * This will overwrite the old experiment with the edited settings
	 */
	$('#SaveEdits').on('click', function() {
		// check if the form is validated
		if (false == ($("#pcrForm").validate().form())) {
			return 0; // if not, don't do anything
		}
		// Grab the Experiment name, could also do this by reading from the experiments list on the homepage
		name = document.getElementById("ExperimentName").innerHTML;
		// Save the file, overwriting the existing file
		Save(name, false);
		// re-load the experiment with the new settings
		loadExperiment(experimentID);
	});

	/*  "Cancel" button on the OpenPCR Form in EDIT MODE
	 * This will cancel any changes made to the form and re-load the experiment as it was last saved
	 */
	$('#Cancel').on('click', function() {
		// what is selected in the drop down menu on the front page?
		experimentID = $("#dropdown").val();
		// clear the form
		clearForm();
		// load the selected experiment
		loadExperiment(experimentID);
	});

	/*  "Edit" button on the OpenPCR Form with a saved experiment
	 */
	$('#editButton').on('click', function() {
		editButton();
	});

	/*  "Delete" button on the OpenPCR Form in EDIT MODE
	 */
	$('#deleteButton').on('click', function() {
		$('#delete_dialog').dialog('open');
	});

	/*  "+ Add Step" button on the OpenPCR Form
	 * Add a new blank step to the end of the presets
	 */
	$('#addStepButton').on('click', function() {
		var location = $(this).parent().attr("id");
		addStep(location);
	});
	/*  "- Delete Step" on the OpenPCR Form
	 * Delete the step
	 */
	$('.deleteStepButton').on('click', function() {
		console.log("deleteStepButton");
		$(this).parent().slideUp('slow', function() {
			// after animation is complete, remove parent step
			$(this).remove();
			//// if the length is now 0, hide the whole div
		});

	});
	/**
	 * Clear all data
	 */
	$('#ClearData').on('click', function() {
		pcrStorage.clearAllData();
	});
	/*
	 * Graph Scale Button 
	 */
	$('#graph_plus').on('click', 
			function(){graph.changeScale(-1);}
	);
	$('#graph_minus').on('click', 
			function(){graph.changeScale(1);}
	);
	
	/*  "More options" button on the OpenPCR Form
	 * Display a bunch of options
	 */
	$('#OptionsButton')
			.on(
					'click',
					function() {
						$(".edit").toggle();
						$("#preContainer").show();
						$("#postContainer").show();
						$("#lidContainer").show();
						// get current state
						buttonText = document.getElementById("OptionsButton").innerHTML;
						// if we're hiding the options and there are no pre-steps or post-steps, hide those sections appropriately
						if (buttonText == 'Less options'
								&& $("#preSteps").html() == "") {
							// hide pre steps
							$("#preContainer").hide();
						}
						if (buttonText == 'Less options'
								&& $("#postSteps").html() == "") {
							// hide post steps
							$("#postContainer").hide();
						}
						// flip the Options button text between "More options" and "Less options"
						var buttonText = (buttonText != 'More options' ? 'More options'
								: 'Less options');
						$('#OptionsButton').html(buttonText);
					});

	// Presets page
	/* editButton()
	 * Function that is called when the "Edit" button is pressed on a "Saved Preset" page. Makes the "Save preset" and "Cancel" buttons
	 * show up, "Add" and "Subtract" steps buttons, and makes all fields editable
	 * Returns: nothing
	 */
	function editButton() {

		// Show the Delete button
		$('#deleteButton').show();
		// Start with the Edit button hidden
		$("#editButton").hide();
		// show the edit buttons
		$(".edit").show();
		// show the lid temp fields
		$("#lidContainer").show();
		// all fields editable
		$("input").removeAttr("readonly");
		// and 'More options' hidden
		$('#OptionsButton').hide();
		// hide the Save button
		$('#Save').hide();
		// show the Cancel button
		$('#Cancel').show();
		// show the SaveEdits button
		$('#SaveEdits').show();
		// Hide the Start/Unplugged button
		startOrUnplugged("none");
		// show the Single Temp mode button
		$('#singleTemp').show();
		// show the Add Step buttons
		$("#preContainer").show();
		$("#postContainer").show();
	}
}
/* deleteCurrentExperiment()
 * Deletes the currently loaded experiment (whatever was last selected in the list)
 * Called by the delete dialog box
 */
function deleteCurrentExperiment() {
	// delete the currently loaded Experiment file
	// given an ID, get the path for that ID
	/*
	experimentPath = window.experimentList[experimentID];
	// delete the file
	var file = experimentPath;
	file.deleteFile();
	*/
	// show a confirmation screen
	pcrStorage.deleteCurrentExperiment  (
	function () {
		$('#delete_confirmation_dialog').dialog('open');
		// then close it after 1 second
		setTimeout(function() {
			$('#delete_confirmation_dialog').dialog('close');
		}, 750);
	});

	// 
}

/* addStep()
 * Add the HTML for a blank step to the desired css selector div
 */
function addStep(location) {
	// first off, if the location is cycleContainer, we really want to modify stepsContainer
	if (location == "cycleContainer") {
		location = "cycleSteps";
	}
	// add to HTML
	if (location == "preSteps") {
		step_name = "Initial Step"
	}
	if (location == "postSteps") {
		step_name = "Final Step"
	}
	if (location == "cycleSteps") {
		step_name = "Step"
	}
	step_number = new Date().getTime();
	;
	var step = '<div class="step">'
			+ '<span class="title">'
			+ step_name
			+ ' </span>'
			+ '<a class="edit deleteStepButton"><img src="images/minus.png" height="30"></a>'
			+ '<table cellspacing="20">'
			+ '<tr>'
			+ '<th><label>temp</label><div><input type="text" style="font-weight:normal;" class="required number textinput" value="" name="temp_'
			+ step_number
			+ '" maxlength="4" min="0" max="120" ></div><span htmlfor="openpcr_temp" generated="true" class="units">&deg;C</span> </th>'
			+ '<th><label>step duration</label><div class=""><input type="text" class="required number textinput" style="font-weight:normal;" value=""  name="time_'
			+ step_number
			+ '" maxlength="4" min="0" max="1000"></div><span htmlfor="openpcr_time" generated="true" class="units">sec</span></th>'
			+ '<th><label>ramp duration</label><div class=""><input type="text" class="required number textinput" style="font-weight:normal;" value=""  name="rampDuration_'
			+ step_number
			+ '" maxlength="6" min="0" max="999999"></div><span htmlfor="openpcr_rampDuration" generated="true" class="units">sec</span></th>'
			+ '</tr>' + '</table>' + '</div>';
	// append a new step to location
	$('#' + location).append(step);
	// make sure the form elements are editable
	$("input").removeAttr("readonly");
	//// make the window bigger
	// make all the delete buttons shown
	// and if there are any other parts of a "step" that are hide/show, they need to be included here
	activateDeleteButton();
	$(".edit").show();

}

function addInitialStep() {
	// add the step to the preContainer
	addStep("preSteps");
}

function addFinalStep() {
	// add the step to the postContainer
	addStep("postSteps");
}

/* deleteStep()
 * Delete the parent step
 */
function deleteStep() {
	// doesn't do anything right now. The delete step button should reference here
}

// JQUERY UI stuffs

$(function() {
	// About Dialog			
	/*
		$('#about_dialog').dialog({
			autoOpen: false,
			width: 300,
			modal: true,
			draggable: false,
			resizable: false,
			buttons:
				{
				"OK": function() {
					$(this).dialog("close"); 
					}
				}
		});
	 */
	// Settings Dialog			
	$('#settings_dialog')
			.dialog(
					{
						autoOpen : false,
						width : 400,
						modal : true,
						draggable : false,
						resizable : false,
						buttons : {
							"Apply" : function() {
								// grab the value of the slider
								contrast = $("#contrast_slider")
										.slider("value");
								// command id
								window.command_id = Math
										.floor(Math.random() * 65534);
								// set the command
								contrast_string = 's=ACGTC&c=cfg&o=' + contrast
										+ '&d=' + command_id;

								// trace it
								console.log("string: " + contrast_string);

								// Write out the  command to CONTROL.TXT
								// name of the output file
								if (window.path != null) {
									var file = window.path
											.resolvePath("CONTROL.TXT");
									// write out all the variables, command id + PCR settings
									var fileStream = new window.runtime.flash.filesystem.FileStream();
									fileStream
											.open(
													file,
													window.runtime.flash.filesystem.FileMode.WRITE);
									fileStream.writeUTFBytes(contrast_string);
									fileStream.close();
								}
							},
							"OK" : function() {
								// grab the value of the slider
								contrast = $("#contrast_slider")
										.slider("value");
								window.command_id = Math
										.floor(Math.random() * 65534);
								contrast_string = 's=ACGTC&c=cfg&o=' + contrast
										+ '&d=' + command_id;
								// trace it
								console.log("string: " + contrast_string);
								// Write out the  command to CONTROL.TXT
								// name of the output file
								if (window.path != null) {
									var file = window.path
											.resolvePath("CONTROL.TXT");
									// write out all the variables, command id + PCR settings
									var fileStream = new window.runtime.flash.filesystem.FileStream();
									fileStream
											.open(
													file,
													window.runtime.flash.filesystem.FileMode.WRITE);
									fileStream.writeUTFBytes(contrast_string);
									fileStream.close();
								}
								// close the dialog window
								$(this).dialog("close");
							}

						}
					});

	$(function() {
		$("#contrast_slider").slider({
			min : 1,
			max : 250
		});
	});

	// Save Dialog			
	$('#save_form').dialog({
		autoOpen : false,
		width : 300,
		modal : true,
		draggable : false,
		resizable : false,
		position : 'center',
		buttons : {
			"Cancel" : function() {
				$(this).dialog("close");
				$("#name").val("");
			},
			"Save" : function() {
				// grab the name from the form
				name = $("#name").val();
				// save the current experiment as the given name
				Save(name, true);
				// update the experiment name in the UI
				$("#ExperimentName").html(name);
				// close the dialog window
				$(this).dialog("close");
			}
		}
	});

	// Save Confirmation Dialog
	$('#save_confirmation_dialog').dialog({
		autoOpen : false,
		width : 300,
		modal : true,
		draggable : false,
		resizable : false

	});

	// Delete Dialog			
	$('#delete_dialog').dialog({
		autoOpen : false,
		width : 300,
		modal : true,
		draggable : false,
		resizable : false,
		buttons : {
			"No" : function() {
				$(this).dialog("close");
			},
			"Yes" : function() {
				// delete the current selected experiment
				deleteCurrentExperiment();
				// Since the experiment was deleted, go to the home screen
				// refresh the list of Presets
				listExperiments();
				// Home screen
				sp2.showPanel(0);
				// close this window
				$(this).dialog("close");
			}
		}
	});

	// Delete Confirmation Dialog
	$('#delete_confirmation_dialog').dialog({
		autoOpen : false,
		width : 300,
		modal : true,
		draggable : false,
		resizable : false

	});

	// Stop Dialog			
	$('#stop_dialog').dialog({
		autoOpen : false,
		width : 300,
		modal : true,
		draggable : false,
		resizable : false,
		buttons : {
			"No" : function() {
				$(this).dialog("close");
			},
			"Yes" : function() {
				$(this).dialog("close");
				stopPCR();
			}
		}
	});
	
	// Dialog Link
	$('#stop_link').click(function() {
		$('#stop_dialog').dialog('open');
		return false;
	});

	// Starting dialog
	$('#starting').dialog({
		autoOpen : false,
		width : 300,
		modal : true,
		draggable : false,
		resizable : false,
	});

	//hover states on the static widgetson the static widgets
		$('#dialog_link, ul#icons li').hover(
			function() { $(this).addClass('ui-state-hover'); }, 
			function() { $(this).removeClass('ui-state-hover'); }
		);

});

// Enter/Return Key clicks "Save" on dialog
$('#save_form').on('keyup', function(e) {
	if (e.keyCode == 13) {
		$(':button:contains("Save")').click();
	}
});

function createCSV () {
	var TAB = encodeURIComponent("\t");
	var RET = encodeURIComponent("\n");
	content = 
	   "Command ID" + TAB //d
	 + "Status" + TAB //s
	 + "Lid Temp" + TAB //l
	 + "Block Temp" + TAB //b
	 + "Therm State" + TAB //t
	 + "Elapsed Time" + TAB //e
	 + "Remaining Time" + TAB //r
	 + "Num of Cycles" + TAB //u
	 + "Current Step" + RET;//c
	var params = ["d","s","l","b","t","e","r","u","c"];
	for (var i=0; i<experimentLog.length; i++) {
		var line = experimentLog[i];
		for (var j=0; j<params.length; j++) {
			if (j!=0) 
				content += TAB;
			if (line[params[j]]!=null)
				content += line[params[j]];
		}
		content += RET;
	}
	$("#download")[0].href = "data:application/octet-stream," + content;
	var fileName = pcrStorage.getLogFileName();
	$("#download")[0].download = fileName;
	$("#download").show();
}

$(document).ready(init);
