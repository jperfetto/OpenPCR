
/* experimentToHTML(inputJSON)
 * Takes a given experiment JSON object and loads it into the OpenPCR interface
 */
function experimentToHTML(inputJSON) {
	// store the experiment to the JSON. This can be modified using the interface buttons, sent to OpenPCR, or saved
	window.experiment = inputJSON;
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
					'<span class="title">'+chrome.i18n.getMessage('heaterLid')+'</span>'
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

var STEP_NAMES = [
              	["stepStep","Step"],
            	["stepDenaturing","Denaturing"],
            	["stepAnnealing","Annealing"],
            	["stepExtending","Extending"],
            	["stepFinalHold","Final Hold"],
            	["stepInitialStep","Initial Step"],
            	["stepFinalStep","Final Step"]
                  ];
function localizeStepName (stepName) {
	for (var i=0; i<STEP_NAMES.length; i++) {
		if (STEP_NAMES[i][1]==stepName)
			return chrome.i18n.getMessage(STEP_NAMES[i][0]);
	}
	return stepName;
}
function globalizeStepName (_stepName) {
	var stepName = _stepName.replace(/^[ \t\n]+/g,'').replace(/[ \t\n]+$/g, '');
	for (var i=0; i<STEP_NAMES.length; i++) {
		if (chrome.i18n.getMessage(STEP_NAMES[i][0])==stepName)
			return STEP_NAMES[i][1];
	}
	return stepName;
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
		stepHTML += '<label for="number_of_cycles"></label><div><span class="title">'+chrome.i18n.getMessage('numberOfCycles')+':</span><input type="text" name="number_of_cycles" id="number_of_cycles" class="required number textinput" maxlength="2" min="0" max="99"  value="'
				+ step.count + '"></div><br />';
		// steps container
		// print each individual step
		for (a = 0; a < step.steps.length; a++) {
			// make the js code a little easier to read
			step_number = a;
			step_name = localizeStepName(step.steps[a].name);
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
					+ '_name" class="title step_name">'
					+ step_name
					+ ' </span><a class="edit deleteStepButton"><img src="images/minus.png" height="30"></a>'
					+ '<table><tr>'
					+ '<th><label for="step'
					+ step_number
					+ '_temp">'+chrome.i18n.getMessage('tempShort')+':</label> <div class="step'
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
					+ '_time">'+chrome.i18n.getMessage('stepDuration')+':</label> <div class=""><input type="text" class="required number textinput"  style="font-weight:normal;" name="step'
					+ step_number
					+ '_time" id="step'
					+ step_number
					+ '_time" value="'
					+ step_time
					+ '" maxlength="4" min="0" max="6000"  ></div><span htmlfor="openpcr_time" generated="true" class="units">'+chrome.i18n.getMessage('sec')+'</span></th>'
					+ '<th><label for="step'
					+ step_number
					+ '_rampDuration">'+chrome.i18n.getMessage('rampDuration')+':</label> <div class=""><input type="text" class="required number textinput"  style="font-weight:normal;" name="step'
					+ step_number
					+ '_rampDuration" id="step'
					+ step_number
					+ '_rampDuration" value="'
					+ step_rampDuration
					+ '" maxlength="6" min="0" max="999999"  ></div><span htmlfor="openpcr_rampDuration" generated="true" class="units">'+chrome.i18n.getMessage('sec')+'</span></th>'
					+ '</tr></table></div>';

		}
	}
	// if single step
	else if (step.type == "step") {
		// make the js code a little easier to read
		step_number = new Date().getTime();
		step_name = localizeStepName(step.name);
		step_time = step.time;
		step_temp = step.temp;
		step_rampDuration = step.rampDuration;
		if (step_rampDuration == null)
			step_rampDuration = 0;

		// main HTML, includes name and temp
		stepHTML += '<div class="step"><span id="'
				+ step_number
				+ '" class="title step_name">'
				+ step_name
				+ ' </span><a class="edit deleteStepButton"><img src="images/minus.png" height="30"></a>'
				+ '<table cellspacing="20"><tr>'
				+ '<th><label>'+chrome.i18n.getMessage('tempShort')+':</label> <div><input type="text" style="font-weight:normal;" class="required number textinput" value="'
				+ step_temp
				+ '" maxlength="4" name="temp_'
				+ step_number
				+ '" min="'+MIN_FINAL_HOLD_TEMP+'" max="120" ></div><span htmlfor="openpcr_temp" generated="true" class="units">&deg;C</span> </th>';

		// if the individual step has 0 time (or blank?) time, then it is a "hold" step and doesn't have a "time" component
		if (step_time != 0) {
			stepHTML += '<th><label>'+chrome.i18n.getMessage('stepDuration')+':</label> <div class=""><input type="text" class="required number textinput" style="font-weight:normal;" value="'
					+ step_time
					+ '" name="time_'
					+ step_number
					+ '" maxlength="4" min="0" max="6000"></div><span htmlfor="openpcr_time" generated="true" class="units">'+chrome.i18n.getMessage('sec')+'</span></th>';
			stepHTML += '<th><label>ramp duration:</label> <div class=""><input type="text" class="required number textinput" style="font-weight:normal;" value="'
					+ step_rampDuration
					+ '" name="rampDuration_'
					+ step_number
					+ '" maxlength="6" min="0" max="999999"></div><span htmlfor="openpcr_rampDuration" generated="true" class="units">'+chrome.i18n.getMessage('sec')+'</span></th>';
		}
	}
	else
		chromeUtil.alert(chrome.i18n.getMessage('error')+" #1986");
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
		step_name = localizeStepName("Initial Step");
	}
	if (location == "postSteps") {
		step_name = localizeStepName("Final Step");
	}
	if (location == "cycleSteps") {
		step_name = localizeStepName("Step");
	}
	step_number = new Date().getTime();
	;
	var step = '<div class="step">'
			+ '<span class="title step_name">'
			+ step_name
			+ ' </span>'
			+ '<a class="edit deleteStepButton"><img src="images/minus.png" height="30"></a>'
			+ '<table cellspacing="20">'
			+ '<tr>'
			+ '<th><label>'+chrome.i18n.getMessage('tempShort')+'</label><div><input type="text" style="font-weight:normal;" class="required number textinput" value="" name="temp_'
			+ step_number
			+ '" maxlength="4" min="0" max="120" ></div><span htmlfor="openpcr_temp" generated="true" class="units">&deg;C</span> </th>'
			+ '<th><label>'+chrome.i18n.getMessage('stepDuration')+'</label><div class=""><input type="text" class="required number textinput" style="font-weight:normal;" value=""  name="time_'
			+ step_number
			+ '" maxlength="4" min="0" max="1000"></div><span htmlfor="openpcr_time" generated="true" class="units">'+chrome.i18n.getMessage('sec')+'</span></th>'
			+ '<th><label>'+chrome.i18n.getMessage('rampDuration')+'</label><div class=""><input type="text" class="required number textinput" style="font-weight:normal;" value=""  name="rampDuration_'
			+ step_number
			+ '" maxlength="6" min="0" max="999999"></div><span htmlfor="openpcr_rampDuration" generated="true" class="units">'+chrome.i18n.getMessage('sec')+'</span></th>'
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



/* writeoutExperiment
 * Reads out all the variables from the OpenPCR form into a JSON object to "save" the experiment
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
	cycleNameArray = [];
	$("#cycleContainer .textinput").each(function(index, elem) {
		//just throw them in an array for now
		cycleArray.push($(this).val());
	});
	$("#cycleContainer .step_name").each (function(index, elem){
		var stepName = globalizeStepName($(this).text());
		Log.v("Step Name=" + stepName);
		cycleNameArray.push(stepName);
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
				"name" : cycleNameArray.shift(),
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

/* save(name)
 * Writes out the current window.experiment to the app:/Experiments directory
 * Input: name, name of the file to be written out (add .pcr extension)
 */
function save(name, isNew, callback) {
	Log.v("save " + name + ", isNew=" + isNew);
	// grab the current experiment and update window.experiment
	pcrProgram = writeoutExperiment();
	console.log("pcrProgram=" + pcrProgram);
	// update the name of the experiment
	pcrProgram.name = name;
	// turn the pcrProgram into a string
	if (isNew) {
		pcrStorage.insertExperiment(name, pcrProgram, function(result) {
			Log.v("result=" + result);
			if (callback) {
				callback();
			}
			$('#save_confirmation_dialog').dialog('open');
			// then close it after 1 second
			setTimeout(function() {
				$('#save_confirmation_dialog').dialog('close');
			}, 750);
		});
	}
	else {
		pcrStorage.updateCurrentExperiment(name, pcrProgram, function(result) {
			Log.v("result=" + result);
			if (callback) {
				callback();
			}
			$('#save_confirmation_dialog').dialog('open');
			// then close it after 1 second
			setTimeout(function() {
				$('#save_confirmation_dialog').dialog('close');
			}, 750);
		});
	}
}

