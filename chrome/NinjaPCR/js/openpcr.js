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

var LATEST_FIRMWARE_VERSION = "1.0.6";
var MIN_FINAL_HOLD_TEMP = 16;


var COLOR_HEATING = "red";
var COLOR_COOLING  = "blue";
var COLOR_STOP = "black";

/**************
 * Home screen*
 ***************/

/* init()
 * Called when the app is loaded.
 * Checks to see if OpenPCR is plugged in (gets the device path if it is) and checks to see if there is an Air update available
 */

var sp2;
var pcrStorage;
function init() {
	pcrStorage = new Storage();
	$(document).keypress(function(event) {
		return disableEnterKey(event);
	});
	prepareButtons();
	sp2 = new Spry.Widget.SlidingPanels('example2');
	
	// hide Settings button by default
	($("#Settings").hide());

	// Scan serial ports and look for a device
	scanPortsAndDisplay();
	// Get experiments from the local storage
	listExperiments();
	
	// i18n
	localize();
}

function checkPlug () {
	scanPortsAndDisplay(2500);
};
function scanPortsAndDisplay (delay) {
	chromeSerial.scan(function(port) {
		var deviceFound = !!port;
		
		var portMessage = (deviceFound)?
				(chrome.i18n.getMessage('deviceFound').replace('___PORT___',port)):chrome.i18n.getMessage('deviceNotFound');
		$("#portLabel").html(portMessage);
		
		if (deviceFound) {
			$("#runningUnplugged").hide();
			$("#runningPluggedIn").show();
			if (!window.checkPlugInterval) {
				window.clearInterval(window.checkPlugInterval);
			}
			window.pluggedIn = true;
			
			if($("#Unplugged").is(':visible')){
				// if the "Unplugged" button is visible, switch it to "Start"
				$("#Unplugged").hide();
				$("#Start").show();
			}
			// Alert Firmware Update
			checkFirmwareVersion(chromeSerial.firmwareVersion);
		} else {
			$("#runningUnplugged").show();
			$("#runningPluggedIn").hide();
			// Not plugged in.
			Log.d('Send "request_status" command and check ongoing experiment (TODO)');
			chromeSerial.scanOngoingExperiment (function () {
				//if (!window.checkPlugInterval) {window.checkPlugInterval = setInterval(checkPlug, 2000); }
				});
		}
	}, delay);
}


function checkFirmwareVersion (version) {
	Log.v("Firmware version=" + version + ", Latest version=" + LATEST_FIRMWARE_VERSION);
	if (version==LATEST_FIRMWARE_VERSION) {
		Log.v("The firmware is up to date.");
	} else {
		Log.v("Please update the firmware!");
		chromeUtil.alertUpdate(version, LATEST_FIRMWARE_VERSION);
	}
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
			presetsHTML += '<option value=1>-'+chrome.i18n.getMessage('none')+'-</option>';
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


/* loadExperiment();
 * loads the experiment with the given experimentID
 */
function loadExperiment(experimentID) {
	Log.v("loadExperiment id=" + experimentID);
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
	Log.d("############ startOrUnplugged ##########");
	//pick the Start or Unplugged button based on whether the device is plugged in or not
	// if plugged in then
	if (window.pluggedIn == true) {
		// then we definitely want to hide the "Unplugged" button
		$("#Start").css("display", display);
		$("#Unplugged").hide();
	}
	else {
		// else, device is unplugged
		// then we definitely want to hide the "Start" button
		$("#Start").hide();
		$("#Unplugged").css("display", display);
	}
}

/* reRunButtons()
 * puts Form buttons in the state they should be immediately following loading an experiment
 */
function reRunButtons() {
	Log.d("reRunButtons");
	// Hide the Delete button
	$('#deleteButton').show();
	// Start with the edit button shown
	$("#editButton").show();
	// Start with the edit buttons hidden
	$(".edit").show();
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
	$('#OptionsButton').html(chrome.i18n.getMessage('moreOptions'));
}/* disableEnterKey(e)
 * The Enter/Return key doesn't do anything right now
 */
function disableEnterKey(e) {
	Log.v("disableEnterKey");
	var key;
	if (window.event)
		key = window.event.keyCode; //IE
	else
		key = e.which; //firefox      

	return (key != 13);
}

function programToDeviceCommand (pcrProgram) {
	// now parse it out
	// Start with the signature
	var parsedProgram = "s=ACGTC";
	// Command
	parsedProgram += "&c=start";
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
	return parsedProgram;
}

var experimentLogger = null;
function startPCR() {
	experimentLogger = new ExperimentLogger();
	experimentLog = [];
	// check if the form is validated
	if (false == ($("#pcrForm").validate().form())) {
		return 0;
	} // if the form is not valid, show the errors
	// command_id will be a random ID, stored to the window for later use
	window.command_id = Math.floor(Math.random() * 65534);
	// command id can't be 0 
	// where is OpenPCR
	var devicePort = chromeSerial.port;
	Log.v("devicePort=" + devicePort);
	
	pcrProgram = writeoutExperiment();
	var parsedProgram = programToDeviceCommand (pcrProgram);	
	// verify that there are no more than 16 top level steps
	Log.v(pcrProgram.steps.length + " : top level steps");
	Log.v(window.lessthan20steps + " : cycle level steps");
	var totalSteps = window.lessthan20steps + pcrProgram.steps.length;

	// check that the entire protocol isn't >252 bytes
	Log.v("parsedProgram=" + parsedProgram);
	if (parsedProgram.length > 512) {
		chromeUtil
				.alert(chrome.i18n.getMessage('lengthLimit').replace('___LENGTH___', parsedProgram.length));
		return 0;
	}

	// verify the cycle step has no more than 16 steps
	else if (window.lessthan20steps > 16) {
		Log.v(parsedProgram);
		chromeUtil.alert(chrome.i18n.getMessage('stepLimit').replace('___STEPS___',window.lessthan20steps));
		return 0;
	}

	// and check that the total overall is less than 25
	else if (totalSteps > 25) {
		Log.v(parsedProgram);
		chromeUtil.alert(chrome.i18n.getMessage('totalStepLimit').replace('___STEPS___',totalSteps));
		return 0;
	}

	//debug
	Log.v(parsedProgram);
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
	experimentLogger.start();
	running();
	
	// then close windows it after 1 second
	setTimeout(function() {
		$('#starting').dialog('close');
	}, 5000);
	setTimeout(function() {
		$('#ex2_p3').show();
	}, 100);
	// also, reset the command_id_counter
	window.command_id_counter = 0;
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

function messageToStatus (message) {
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
	return status;
}

// Process Status update
function onReceiveStatus(message) {
	var status = messageToStatus(message);
	experimentLog.push(status);
	experimentLogger.log(status);
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
	Log.v(stopPCR);
	// Send out the STOP command by serial
	chromeSerial.stopOnComplete();
	chromeSerial.sendStopCommand(stopPCR, function(){
	});
	window.clearInterval(window.updateRunningPage);
	createCSV();
	$("#homeButton").show();
	// go back to the Form page
	//sp2.showPanel(1);
	return false;
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

	$('#newExperimentButton').on('click', newExperiment);
	$('#listSubmitButton').on('click', listSubmit);
	$('#initialStep').on('click', addInitialStep);
	$('#finalStep').on('click', addFinalStep);
	$('#saveForm').on('click', function() {
		$('#Start').click();
	});
	$('#appVersion').html(chrome.runtime.getManifest().version);
	
	/*  "About" button on the OpenPCR Home page
	 * Displays about info
	 */
	$('#About').on('click', function() {

		$('#about_dialog').dialog({
			autoOpen : false,
			width : 400,
			modal : true,
			draggable : false,
			resizable : false,
			buttons : {
				"Close" : function() {
					$(this).dialog("close");
				}
			}
		});
		$('#about_dialog').dialog('open');
	});

	/*  "Contrast" button on the OpenPCR Home page
	 * Sets the contrast for OpenPCR
	 */
	$('#Settings').on('click', function() {
		$('#settings_dialog').dialog('open');
	});

	$('#OpenDownloadPage').on('click', function () {
		window.open(chrome.i18n.getMessage('downloadUrl'));
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
		if (graph) graph.clear();
		listExperiments();
		sp2.showPanel(0);
		setTimeout(clearForm, 500);
	});

	/*  "Start" button on the OpenPCR Form page
	 * Sends an experiment to OpenPCR and switches to the Running page
	 */
	$('#Start').on('click', function() {
		Log.d("#Start.click");
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
		save(name, false);
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
		Log.v("deleteStepButton");
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
						buttonText = document.getElementById("OptionsButton").value;
						// if we're hiding the options and there are no pre-steps or post-steps, hide those sections appropriately
						if (buttonText == chrome.i18n.getMessage('lessOptions')
								&& $("#preSteps").html() == "") {
							// hide pre steps
							$("#preContainer").hide();
						}
						if (buttonText == chrome.i18n.getMessage('lessOptions')
								&& $("#postSteps").html() == "") {
							// hide post steps
							$("#postContainer").hide();
						}
						// flip the Options button text between "More options" and "Less options"
						var buttonText = (buttonText != chrome.i18n.getMessage('moreOptions') ? chrome.i18n.getMessage('moreOptions')
								: chrome.i18n.getMessage('lessOptions'));
						$('#OptionsButton')[0].value = buttonText;
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
		$("#editButton").show();
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
			.dialog( //TODO Control LCD
					{
						autoOpen : false,
						width : 400,
						modal : true,
						draggable : false,
						resizable : false,
						buttons : {
							"Apply" : function() { //TODO localize
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
								Log.v("string: " + contrast_string);

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
							"OK" : function() { //TODO localize
								// grab the value of the slider
								contrast = $("#contrast_slider")
										.slider("value");
								window.command_id = Math
										.floor(Math.random() * 65534);
								contrast_string = 's=ACGTC&c=cfg&o=' + contrast
										+ '&d=' + command_id;
								// trace it
								Log.v("string: " + contrast_string);
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
		buttons : { //TODO localize
			"Cancel" : function() {
				$(this).dialog("close");
				$("#name").val("");
			},
			"Save" : function() {
				// grab the name from the form
				name = $("#name").val();
				// save the current experiment as the given name
				save(name, true);
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
		buttons : { //TODO localize
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
		buttons : { //TODO localize
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

