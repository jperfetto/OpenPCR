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
	
	
	if (air.Capabilities.os.toLowerCase().indexOf("Linux") > -1)
						{
						alert("linux");
						} 
	// Application Updater code		
		// declare the appUpdater variable
		var appUpdater = new window.runtime.com.riaspace.nativeApplicationUpdater.NativeApplicationUpdater();
		appUpdater.updateURL = 'http://api.openpcr.org/sw/air/appUpdater_descriptor.xml';
		appUpdater.addEventListener("initialized", function(e)
			{
				//appUpdater.checkNow();
			});
		
		// initialize the appUpdater
		//appUpdater.initialize();

		// when the status comes back
		appUpdater.addEventListener("updateStatus", function(e)
		{
		if (e.available)
			{ 
			//e.preventDefault();
			//alert(e.isDefaultPrevented());
			alert("Update for OpenPCR is Available for Download! Please click 'OK' to restart to app");
			}
		});
		
	function init()
		{
		// hide Settings button by default
		($("#Settings").hide());
		
		// get the location of OpenPCR (can be null)
		var deviceLocation = pluggedIn();
					
		// if OpenPCR is plugged in
		if (deviceLocation != null)
			{
				// get the path for OpenPCR
					var devicePath = new air.File(); 
					devicePath.nativePath = deviceLocation;
				// store the path to a window variable for later use
					window.path = devicePath;
					running(window.path);

			}
			
		// Display the list of Saved Experiments on the home page
			listExperiments();
		// get ready to validate the OpenPCR form
			$("#pcrForm").validate();
		}
	
	/* listExperiments()
	* Updates the list of Saved Experiments on the home page.
	* Grabs all the files in the Experiments folder and lists them alphabetically
	*
	*/
	function listExperiments()
	{
		// Start a drop down menu
		presetsHTML = "<select id='dropdown'>";
		// look for "Experiments" directory
		searchDir = air.File.applicationStorageDirectory.resolvePath("Experiments");
		// create the "Experiments" directory if one doesn't exist already
		searchDir.createDirectory();
		// get a list of all files in the folder
		window.experimentList = searchDir.getDirectoryListing();
		// Loop through and add each filename as "option" values for the drop down
		var presetsList = "";
		for (var f = 0; f < window.experimentList.length; f++)
			{
				if (window.experimentList[f].isDirectory) {
						// if the file is a directory, don't add it to the list
						if (window.experimentList[f].name !="." && window.experimentList[f].name !="..") {}
				} else {
					// get the filename
					fileName = window.experimentList[f].name;
					// take off the .pcr extension
					var experimentName =  fileName.substring(0, fileName.indexOf('.pcr'));
					// if the file was type .pcr, add the filename as an option for the drop down
					if (fileName.length != experimentName.length && experimentName != "")
						{
							presetsHTML += '<option value="' + f + '">' + experimentName + "</option>";
						}
				}
			}
		
		// if blank, add a "No Saved Experiments" item
		if (presetsHTML == "<select id='dropdown'>")
			{
				presetsHTML += '<option value=1>-none-</option>'; 
			}

		// close the drop down HTML tags
		presetsHTML += "</select>";
		// update the HTML on the page
		$("#reRun").html(presetsHTML);
		
	}
	
	/* listSubmit()
	* Loads the selected experiment in the list on the home page
	*/
	function listSubmit()
	{
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
	function pluggedIn()
	{
		alert("pluggedIn");
		var volInfo = air.StorageVolumeInfo.storageVolumeInfo;
		// wait for a USB device to be plugged in
		volInfo.addEventListener(air.StorageVolumeChangeEvent.STORAGE_VOLUME_MOUNT, function(e)
			{
				// if the name is OpenPCR, then set the window variable pluggedIn to "true". otherwise do nothing
				//alert(e.storageVolume.name);
				var pattern = /OPENPCR/;
				// if the nativePath contains "OpenPCR"
				alert("nativePath=" + e.rootDirectory.nativePath + ", storageVolume=" + e.storageVolume.name);
				if (pattern.test(e.rootDirectory.nativePath) || pattern.test(e.storageVolume.name))
					{
					// re-set the path to OpenPCR
					
					// hack, windows doesn't keep good track of the device path, so storing the drive name (i.e. E:) just in case
					window.mountDirectory = e.rootDirectory.nativePath;
					var deviceLocation = e.rootDirectory.nativePath;
					var devicePath = new air.File(); 
					devicePath.nativePath = deviceLocation;
				// store the path to a window variable for later use
					window.path = devicePath;
					running(window.path);
					// update the UI
					if($("#Unplugged").is(':visible'))
						{
						// if the "Unplugged" button is visible, switch it to "Start"
						$("#Unplugged").hide();
						$("#Start").show();
						}
					// and next time we check the status, make sure it shows plugged in
					window.pluggedIn=true;
					// and, set the running page to be normal
					$("#runningUnplugged").hide();
					$("#runningPluggedIn").show();
					
					// Show the Settings button if the status file contains "o="
					var contrast_pattern = /o=/;
					// grab the text from STATUS.TXT
					status_string = readDevice(window.runningFile);
					// if it contains "o=", show the settings button, otherwise don't
					if (contrast_pattern.test(status_string))
						 {
						 ($("#Settings").show());
						 }
					 else
						 {
						 //air.trace("doesn't have an o");
						 }
					
					}
				else
					{
					// otherwise it isn't OpenPCR that was plugged in
					}
			});
		// wait for a USB device to be unplugged
		volInfo.addEventListener(air.StorageVolumeChangeEvent.STORAGE_VOLUME_UNMOUNT, function(e)
			{
				// air doesn't store the name of what was unplugged, so does the nativePath match OpenPCR?
				nativePath = e.rootDirectory.nativePath;
				// if the device unplugged contained "OpenPCR"
				var pattern = /OPENPCR/;
				if (pattern.test(nativePath) || nativePath == window.mountDirectory)
					{
						// hide the Settings button
						($("#Settings").hide());
					
						if($("#Start").is(':visible'))
						{
						// if the "Start" button is visible, hide it and show "Unplugged" instead
						$("#Start").hide();
						$("#Unplugged").show();
						}
						// and next time we check the status, make sure it shows unplugged
						window.pluggedIn=false;
						
						// and, change the running screen to tell the user OpenPCR is unplugged
						$("#runningUnplugged").show();
						$("#runningPluggedIn").hide();
						clearInterval(window.updateRunningPage);
					}
				else
					{
					// otherwise it isn't OpenPCR that was unplugged
					}
			});
			
		// Get a list of the current Volumes mounted on the computer
			var volumesList = air.StorageVolumeInfo.storageVolumeInfo.getStorageVolumes();
		// look in the list for a Volume named "OpenPCR"
			var pattern = /OPENPCR/;
			alert("Checking volumeList... list=" + volumesList.length);
			for (var i = 0; i < volumesList.length; i++)
				{
					alert("volumesList["+i+"].name="+volumesList[i].name);
					var directory = volumesList[i].rootDirectory.nativePath;
					if (pattern.test(volumesList[i].name))
						{
						var deviceLocation = directory;
						window.mountDirectory = directory;
						//alert(directory);
						}
				}
			
			// if device is not plugged in...
				if (deviceLocation == null)
				{
				window.pluggedIn=false;
				// and put a message over the Running screen
				
				}
			// otherwise, make sure the "Start" button is correclty displayed and the Running screen is correctly displayed
				else
				{
				window.pluggedIn=true;
				}
				
			return deviceLocation;
	}
		
	/* loadExperiment();
	* loads the experiment with the given experimentID
	*/
	function loadExperiment(experimentID)
		{
		// Now we've made all the modifications needed, display the Form page
			sp2.showPanel(1);
		// clear the experiment form
		clearForm();
		// given an experiment ID, get the path for that ID
		experimentPath = window.experimentList[experimentID];
		// if the experiment id doesn't exist, exit and do nothing (why would this happen?)
		if (experimentPath == null) { return 0; }
		// read in the file
		experimentJSON = JSON.parse(readFile(experimentPath));
		// loads filen into the Form and moves onto Form page
		experimentToHTML(experimentJSON);
		// update the buttons to make sure everything is ready to re-run an experiment
		reRunButtons();
		}

	/*  newExperiment()
	* This function is called when the "New Experiment" button is clicked on the Home page
	* This function brings up a blank experiment
	*/	
		function newExperiment()
		{
		// clear the experiment form
			clearForm();
		// set up the blank experiment
		var experimentJSON = 
				{
				"name": "New Experiment",
				"steps": [
							{ "type": "cycle",
								"count": "",
								"steps" : [
											{ "type": "step",
												"name": "Denaturing",
											  "time": "",
												"temp": "",
												"rampDuration": 0
											},
											{ "type": "step",
												"name": "Annealing",
											  "time": "",
												"temp": "",
												"rampDuration": 0
											},
											{ "type": "step",
												"name": "Extending",
											  "time": "",
												"temp": "",
												"rampDuration": 0
											}
										]
										},
							{   "type": "step",
								"name": "Final Hold",
								"temp": "4",
							  "time": 0,
								"rampDuration": 0
							}
						],
						"lidtemp": 110
				};
		experimentToHTML(experimentJSON);
		
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
		function startOrUnplugged(display)
		{
			//pick the Start or Unplugged button based on whether the device is plugged in or not
			// if plugged in then
			if (window.pluggedIn==true)
				{
				// then we definitely want to hide the "Unplugged" button
				$("#Unplugged").hide();
				// and maybe want to show/hide the "Start" button, whatever was submitted as the "display" var
				$("#Start").css("display", display)
				// and, change the running screen to plugged in
							$("#runningUnplugged").hide();
							$("#runningPluggedIn").show();
				}
			else 
				{
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
		function reRunButtons()
		{
		// Hide the Delete button
			$('#deleteButton').hide();
		// Start with the edit button shown
			$("#editButton").show();
		// Start with the edit buttons hidden
			$(".edit").hide();
		// hide the lid temp fields
			$("#lidContainer").hide();
		// all fields locked
			$("input").attr("readonly","readonly");
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
		function newExperimentButtons()
		{
		// Hide the Delete button
			$('#deleteButton').hide();
		// Start with the edit button hidden
			$("#editButton").hide();
		// Start with the edit buttons hidden
			$(".edit").hide();
		// lid temp hidden
			$("#lidContainer").hide();
		// all fields editable
			$("input").attr("readonly","");
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
	function writeoutExperiment()
	{
		// grab the Experiment Name
		experimentName = document.getElementById("ExperimentName").innerHTML;
		
		// grab the pre cycle variables if any exist
			preArray = [];
			$("#preContainer .textinput").each(function(index, elem)
					{
					//just throw them in an array for now
					if ($(this) != null) preArray.push($(this).val());
					});
					
		// grab the cycle variables
			cycleArray = [];
			$("#cycleContainer .textinput").each(function(index, elem)
					{
					//just throw them in an array for now
					cycleArray.push($(this).val());
					});
			
		// grab the post cycle variables if any exist
			postArray = [];
			$("#postContainer .textinput").each(function(index, elem)
					{
					//just throw them in an array for now
					postArray.push($(this).val());
					});
					
		// grab the final hold steps if any exist
			holdArray = [];
			$("#holdContainer .textinput").each(function(index, elem)
					{
					//just throw them in an array for now
					holdArray.push($(this).val());
					});
		// grab the lid temp
			$("#lidContainer .textinput").each(function(index, elem)
					{
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
		preLength = (preArray.length)/3;
		for (a=0 ; a < preLength; a++)
				{
				experimentJSON.steps.push(
					{   "type": "step",
						"name": "Initial Step",
						"temp": preArray.shift(),
						"time": preArray.shift(),
						"rampDuration": preArray.shift()
					});
				}
		
		// Cycle and cycle steps
		// the cycle will be a # of cycles as the first element, then temp/time pairs after that
			count = cycleArray.shift(),
			if ( cycleArray.length > 0 && count > 0 )
			{
				experimentJSON.steps.push(
					{  
					"type": "cycle",
					// add the number of cycles
					"count": count,
					"steps": [] 
					});
				
				// then add the cycles
				current = experimentJSON.steps.length-1;
				
				// every step will have 3 elements in cycleArray (Time, temp, rampDuration)
				cycleLength = (cycleArray.length)/3;
				for (a=0 ; a < cycleLength; a++)
					{
						
						experimentJSON.steps[current].steps.push(
						{
							"type": "step",
							"name": "Step",
							"temp": cycleArray.shift(),
							"time": cycleArray.shift(),
							"rampDuration": cycleArray.shift()
						});
					}
			}
			
			
		// every step will have 3 elements in preArray (Time, temp, rampDuration)
		// a better way to do this would be for a=0, postArray!=empty, a++
		postLength = (postArray.length)/3;
		for (a=0 ; a < postLength; a++)
				{
				experimentJSON.steps.push(
				{   "type": "step",
					"name": "Final Step",
					"temp": postArray.shift(),
					"time": postArray.shift(),
					"rampDuration": postArray.shift()
				});
				}
			
		// Final Hold step
			if (holdArray.length > 0)
				{
				experimentJSON.steps.push(
					{   "type": "step",
						"name": "Final Hold",
						"time": 0,
						"temp": holdArray.shift(),
						"rampDuration": 0
					});
				}
					
		// return the experiment JSON object
		return experimentJSON;
		}
		
	/* Save(name)
	* Writes out the current window.experiment to the app:/Experiments directory
	* Input: name, name of the file to be written out (add .pcr extension)
	*/
	function Save(name)
	{
		alert("Save " + name);
		// create the filename
			fileName = name + ".pcr";
		// grab the current experiment and update window.experiment
			pcrProgram = writeoutExperiment();
		// update the name of the experiment
			pcrProgram.name = name;
		// turn the pcrProgram into a string
			pcrProgram = JSON.stringify(pcrProgram, null, '\t');
			alert(pcrProgram);
		// set the destination folder for the file
			fileDestination = air.File.applicationStorageDirectory.resolvePath("Experiments");
		// create the "My Presets" directory if one doesn't exist already
			fileDestination.createDirectory();
		// set the filename
		fileDestination = fileDestination.resolvePath(fileName);
		// write out the file
			var fileStream = new window.runtime.flash.filesystem.FileStream();
			fileStream.open(fileDestination, window.runtime.flash.filesystem.FileMode.WRITE); 
			fileStream.writeUTFBytes(pcrProgram); 
			fileStream.close();
		// show a confirmation screen
		$('#save_confirmation_dialog').dialog('open');
		// then close it after 1 second
		setTimeout(function(){$('#save_confirmation_dialog').dialog('close');}, 750);
	}

	/* experimentToHTML(inputJSON)
		* Takes a given experiment JSON object and loads it into the OpenPCR interface
		*/
		function experimentToHTML(inputJSON)
		{
			// store the experiment to the JSON. This can be modified using the interface buttons, sent to OpenPCR, or saved
			window.experiment = inputJSON;
			// clear the Form
			clearForm();
			// Update the experiment name
			var experimentName = inputJSON.name;
			// only use the first 20 chars of the experimentName
			experimentName = experimentName.slice(0,18);
			$("#ExperimentName").html(experimentName);
			
			// for every .steps in the experiment, convert it to HTML
			var experimentHTML = "";
			// break the rest of the experiment up into "pre cycle" (0), "cycle" (1), and "post cycle" (2) sections
			var count = 0;
			// add the lid temperature div but hide it
			$('#lidContainer').hide();
			// max temp 120, min temp 0 (off)
			$('#lidTemp').html('<span class="title">Heated Lid</span>' + '<input type="text" name="lid_temp" id="lid_temp" class="required integer textinput" maxlength="3" min="0" max="120"  value="' + inputJSON.lidtemp + '">');
			// 4 possibile DIVs
			// pre-steps, cycle steps, post-steps, and final hold step
			// Add the experiment to the page	
			for (i=0; i < inputJSON.steps.length; i++)
					{
					// pre-cycle to start
					if (count==0 & inputJSON.steps[i].type == "step" && inputJSON.steps[i].time != 0)
					// if it's for pre-cycle, and not a final hold (0 time)
						{
						// show the preContainer div
						$('#preContainer').show();
						$('#preSteps').append(stepToHTML(inputJSON.steps[i]))
						}
					
					else if (count==0 && inputJSON.steps[i].type == "cycle")
					// if it's cycle, put the cycle in the Cycle container
					{
						$('#cycleContainer').show();
						$('#cycleSteps').append(stepToHTML(inputJSON.steps[i]));
						count=1;
					}
				
					else if (count==1 && inputJSON.steps[i].type == "step" && inputJSON.steps[i].time != 0)
					// if it's post (but not a final hold), put the steps in the Post container
					{
						$('#postContainer').show();
						$('#postSteps').append(stepToHTML(inputJSON.steps[i]));
					}
					
					else if (inputJSON.steps[i].type == "step" && inputJSON.steps[i].time == 0)
					// if it's the final hold (time = 0), put it in the final hold container
					{
						$('#holdContainer').show();
						$('#holdSteps').append(stepToHTML(inputJSON.steps[i]));
					}
				
				}	
		}
		
		/* stepToHTML(step)
		* Turns a step into HTML. However, this HTML doesn't have a container div/fieldset
		* If the step is a cycle, it will return html with all the cycles represented.
		* If the step is a single step, html with just one cycle is returned
		*/
		
	function stepToHTML(step)
		{
		stepHTML = "";
		// if cycle
		if (step.type=="cycle")
		 {
			// printhe "Number of Cycles" div
			// max 99 cycles
			stepHTML += '<label for="number_of_cycles"></label><div><span class="title">Number of Cycles:</span><input type="text" name="number_of_cycles" id="number_of_cycles" class="required number textinput" maxlength="2" min="0" max="99"  value="' + step.count + '"></div><br />';
			// steps container
			// print each individual step
			for (a=0; a<step.steps.length; a++)
					{
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
					stepHTML += '<div class="step"><span id="step' + step_number + '_name" class="title">' + step_name + ' </span><a class="edit deleteStepButton"><img src="images/minus.png" height="30"></a>' +
						'<table><tr>' +
							'<th><label for="step' + step_number + '_temp">temp:</label> <div class="step' + step_number + '_temp"><input type="text" style="font-weight:normal;" class="required number textinput" name="step' + step_number + '_temp" id="step' + step_number + '_temp" value="' + step_temp + '" maxlength="4" min="-20" max="120" ></div><span htmlfor="openpcr_temp" generated="true" class="units">&deg;C</span> </th>' +
							'<th><label for="step' + step_number + '_time">step duration:</label> <div class=""><input type="text" class="required number textinput"  style="font-weight:normal;" name="step' + step_number + '_time" id="step' + step_number + '_time" value="' + step_time + '" maxlength="4" min="0" max="6000"  ></div><span htmlfor="openpcr_time" generated="true" class="units">sec</span></th>' +
							'<th><label for="step' + step_number + '_rampDuration">ramp duration:</label> <div class=""><input type="text" class="required number textinput"  style="font-weight:normal;" name="step' + step_number + '_rampDuration" id="step' + step_number + '_rampDuration" value="' + step_rampDuration + '" maxlength="6" min="0" max="999999"  ></div><span htmlfor="openpcr_rampDuration" generated="true" class="units">sec</span></th>' +
						'</tr></table></div>';
			
					}
		 }
		// if single step
		else if (step.type=="step")
		 {
		 // make the js code a little easier to read
		 step_number = new Date().getTime();
		 step_name = step.name;
		 step_time = step.time;
		 step_temp = step.temp;
		 step_rampDuration = step.rampDuration;
		 if (step_rampDuration == null)
				 step_rampDuration = 0;
		
		// main HTML, includes name and temp
		stepHTML += '<div class="step"><span id="' + step_number + '" class="title">' + step_name + ' </span><a class="edit deleteStepButton"><img src="images/minus.png" height="30"></a>' +
			'<table cellspacing="20"><tr>' +
				'<th><label>temp:</label> <div><input type="text" style="font-weight:normal;" class="required number textinput" value="'+ step_temp + '" maxlength="4" name="temp_' + step_number + '" min="0" max="120" ></div><span htmlfor="openpcr_temp" generated="true" class="units">&deg;C</span> </th>';
		
		 // if the individual step has 0 time (or blank?) time, then it is a "hold" step and doesn't have a "time" component
		if (step_time != 0)	
				{
				stepHTML += '<th><label>step duration:</label> <div class=""><input type="text" class="required number textinput" style="font-weight:normal;" value="' + step_time + '" name="time_' + step_number + '" maxlength="4" min="0" max="6000"></div><span htmlfor="openpcr_time" generated="true" class="units">sec</span></th>';
				stepHTML += '<th><label>ramp duration:</label> <div class=""><input type="text" class="required number textinput" style="font-weight:normal;" value="' + step_rampDuration + '" name="rampDuration_' + step_number + '" maxlength="6" min="0" max="999999"></div><span htmlfor="openpcr_rampDuration" generated="true" class="units">sec</span></th>';
				}
		 }
		else alert("Error #1986");
		stepHTML += '</tr></table></div>';
		return stepHTML;
		}
	
	/* stepToString(inputJSON)
	* Takes a JSON object and turns it into a string
	* This is used to load an experiment into the OpenPCR device
	*/
	function stepToString(inputJSON)
	{
			var stepString = "";
			// if single step return something like (1[300|95|Denaturing])
			if (inputJSON.type=="step")
			{
//			stepString += "[" + inputJSON.time + "|" + inputJSON.temp + "|" + inputJSON.name.slice(0,13) + "]";			
			stepString += "[" + inputJSON.time + "|" + inputJSON.temp + "|" + inputJSON.name.slice(0,13) + "|" + inputJSON.rampDuration + "]";			
			}
			// if cycle return something like (35,[60|95|Step A],[30|95|Step B],[30|95|Step C])
			else if (inputJSON.type=="cycle")
			 {
				// add the number of Cycles
						stepString += "(";
						stepString += inputJSON.count;
				
				for (a=0; a<inputJSON.steps.length; a++)
						{
//						stepString += "[" + inputJSON.steps[a].time + "|" + inputJSON.steps[a].temp + "|" + inputJSON.steps[a].name.slice(0,13) + "]";
						stepString += "[" + inputJSON.steps[a].time + "|" + inputJSON.steps[a].temp + "|" + inputJSON.steps[a].name.slice(0,13) + "|" + inputJSON.steps[a].rampDuration + "]";
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
	function clearForm()
	{
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
	function disableEnterKey(e)
	{
     var key;      
     if(window.event)
          key = window.event.keyCode; //IE
     else
          key = e.which; //firefox      

     return (key != 13);
	}
	
	function startPCR()
	{
			// check if the form is validated
			if (false == ($("#pcrForm").validate().form()))
				{ return 0;} // if the form is not valid, show the errors
			// command_id will be a random ID, stored to the window for later use
			window.command_id=Math.floor(Math.random()*65534);
			// command id can't be 0 
			// where is OpenPCR
				var devicePath =  window.path;
			alert("devicePath=" + devicePath);
			// name of the output file written to OpenPCR
				var controlFile = devicePath.resolvePath("CONTROL.TXT"); 
			alert("controlFile=" + controlFile);
			// grab all the variables from the form in JSON format
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
			parsedProgram +="&p=";
			window.lessthan20steps = 0;
			for (i=0; i < pcrProgram.steps.length; i++)
				{
					if (pcrProgram.steps[i].type == "step")
					// if it's a step, stepToString will return something like [300|95|Denaturing]
					// then this loop needs to figure out when to add [1(  and )]
						{
							// if the previous element wasn't a step (i.e. null or cycle)
							if (typeof pcrProgram.steps[i-1] == 'undefined' || pcrProgram.steps[i-1].type == "cycle")
							{
							parsedProgram += "(1";
							}
							
						parsedProgram += stepToString(pcrProgram.steps[i]);
						
						// if the next element isn't a step (i.e. null or cycle)
							if (typeof pcrProgram.steps[i+1] == 'undefined' || pcrProgram.steps[i+1].type != "step")
							{
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
						
			// verify that there are no more than 16 top level steps
			air.trace(pcrProgram.steps.length + " : top level steps" );
			air.trace( window.lessthan20steps + " : cycle level steps" );
			var totalSteps = window.lessthan20steps + pcrProgram.steps.length;
			
			// check that the entire protocol isn't >252 bytes
			alert("parsedProgram="+parsedProgram);
			if (parsedProgram.length > 252)
			{
			air.trace("parsedProgram:" + parsedProgram);
			alert("Oops, OpenPCR can't handle protocols longer than 252 characters, and this protocol is " + parsedProgram.length + " characters. The fix? You can try trimming down the name of your protocol or removing unnecessary steps");
			return 0;
			}
			
			// and check there aren't more than 16 steps at the top level
			
			// this is wrong
			// there can be up to 16 initial, 16 cycle, and 16 final steps, but the total can't be more than 30
			// else if (pcrProgram.steps.length > 16)
// 			{
// 				air.trace(parsedProgram);
// 			alert("OpenPCR can handle a maximum of 16 top-level steps, you have " + pcrProgram.steps.length + " steps");
// 				return 0;
// 			}
// 			
			
			// verify the cycle step has no more than 16 steps
			else if ( window.lessthan20steps > 16)
			{
				air.trace(parsedProgram);
			alert("OpenPCR can handle a maximum of 20 cycle steps, you have " + window.lessthan20steps + " steps");
				return 0;
			}
			
			// and check that the total overall is less than 25
			else if ( totalSteps > 25)
			{
				air.trace(parsedProgram);
			alert("OpenPCR can handle a maximum of 25 total steps, you have " + totalSteps + " steps");
				return 0;
			}
			
			//debug
			air.trace(parsedProgram);
			// go to the Running dashboard
			sp2.showPanel(2);
			$("#ex2_p3").hide();
			// go to the top of the page
			scrollTo(0,0);
			//hide the home button on the running page
			$("#homeButton").hide();
			// show the "stop" button
			$("#cancelButton").show();
			$('#starting').dialog('open');
			// write out the file to the OpenPCR device
			var fileStream = new window.runtime.flash.filesystem.FileStream();
			fileStream.open(controlFile, window.runtime.flash.filesystem.FileMode.WRITE); 
			fileStream.writeUTFBytes(parsedProgram);
			fileStream.close();
			// then close windows it after 1 second
			setTimeout(function(){$('#starting').dialog('close');}, 5000);
			setTimeout(function(){$('#ex2_p3').show();}, 5000);
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
	
	function running(path)
		{
		// Find the STATUS.TXT file containing the current OpenPCR data
			window.runningFile = path;
			window.runningFile = window.runningFile.resolvePath("STATUS.TXT");

		// refresh the running page every 1000 ms
			window.updateRunningPage = setInterval(updateRunning,1000);
		}
	
	/* updateRunning()
	* Updates the Running page variables
	*/
	
	function updateRunning()
		{
		updateFile = readDevice(window.runningFile);
		if (updateFile==null || updateFile=="")
			{
			//window.command_id_counter++;
			}
		else
			{
			air.trace("updateFile: " + updateFile + '\n\n');
			// split on &
			var splitonAmp = updateFile.split("&");
			// split on =
			var status = new Array();
			for(i=0;i<splitonAmp.length;i++)
				{
					var data = splitonAmp[i].split("=");
					if(isNaN(parseFloat(data[1])))
						{
						// not a number
						status[data[0]]=data[1];
						}
					else
						{
						// a number
						status[data[0]]=parseFloat(data[1]);
						}
				}		
  			
				// make sure the status isn't blank
				// if command id in the running file doesn't match, check again 50 times and then quit if there is still no match
				if (status["d"]!=window.command_id)
					{
					if (window.command_id_counter > 50)
						{
						//alert("OpenPCR command_id does not match running file, window.command_id_counter =" + window.command_id_counter +  " . This error should not appear\nstatus"+status["d"]+"\nwindow:"+window.command_id);
						// quit
						//air.NativeApplication.nativeApplication.exit();
						}
					window.command_id_counter++;
					// debug
					air.trace("command_id_counter " + window.command_id_counter);
					}
				// if app command id matches the device command id, reset the counter
				if (status["d"]==window.command_id)
				{
				window.command_id_counter = 0;
				air.trace(window.command_id_counter);
				}
				if (Math.random()<0.01)
					status["s"]="complete";
				if (status["s"]=="running" || status["s"]=="lidwait")
				{
					//debug
					air.trace(status["s"]);
					// preset name
					var prog_name = status["n"];
					$("#runningHeader").html(prog_name);
							
					if (status["s"]=="lidwait")
					{
						// if the lid is heating say so
						$("#progressbar").hide();
						$("#cycleNumOfNum").hide();
						$("#timeRemaining").html("");
						$("#minutesRemaining").html("Lid is heating");
						
						// during lidwait, no protocol name is included, so include the protocol name from the previous page
						$("#runningHeader").html(document.getElementById("ExperimentName").innerHTML);
					}
					
					if (status["s"]=="running")
						{
							$("#timeRemaining").html("Time remaining:");
							// otherwise, if running set variable for percentComplete
							// never display less than 2% for UI purposes
							var percentComplete = 100 * status["e"]/(status["e"]+status["r"]);
							if (percentComplete < 2)
								{ percentComplete = 2; }
							// Progress bar
							$("#progressbar").progressbar({ value: percentComplete});
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
							air.trace(status["b"]);
							$("#blockTemp").html(block_temp);
							
						// Current lid temp
							var lid_temp = status["l"];
							$("#lidTemperature").html(lid_temp);
						// For the debugger, write all 8 vars out to the history file
						//writeCSV(document.getElementById("runningHeader").innerHTML, document.getElementById("minutesRemaining").innerHTML, 1, document.getElementById("cycleNumber").innerHTML, document.getElementById("totalCycles").innerHTML, document.getElementById("blockTemp").innerHTML, document.getElementById("lidTemp").innerHTML, document.getElementById("progressbar").innerHTML);
						//writeCSV(prog_name, status["e"], secondsRemaining, 1, current_cycle, total_cycles, block_temp, lid_temp);
						
						}
					else if (status["s"]=="complete")
						{
						// if the status of OpenPCR comes back as "complete"		
						// show the "Home" button
						$("#homeButton").show();
						// hide the cancel button
						$("#cancelButton").hide();
						// hide timeRemaining
						$("#timeRemaining").hide()
						// finish the progress bar
						$("#progressbar").progressbar({ value: 100 });
						// show the completed message
						minutesRemaining='<span style="color:#04B109;">Done!</span>';
						$("#minutesRemaining").html(minutesRemaining);
						// update the "current temp"
						var block_temp = status["b"];
							$("#blockTemp").html(block_temp);
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
						}
					else if (status["status"]=="stopped")
						{
						// nothing
						}
					else if (status["status"]=="error")
						{
						// error
						alert("Error");
						}
				}
		
		// Show the Settings button if the status file contains "o="
				var contrast_pattern = /o=/ ;
				// grab the text from STATUS.TXT
				
				status_string = updateFile;
				air.trace("status: " + status_string);
				
				// if it contains "o=", show the settings button, otherwise don't
				if (contrast_pattern.test(status_string))
					{
					($("#Settings").show());
					}
				else
					{
					air.trace("doesn't have an o");
					
					air.trace(window.runningFile);
					}
					
		}
		

		/* readDevice()
		* Checks the OS (Mac or PC) and runs the appropriate middleman app (NCC) to grab info off the USB drive
		*/
		function readDevice(filePath)
		{
			if (filePath.exists)
			{   
				// are native processes supported?
				if (air.NativeProcess.isSupported)
				 {
					var nativeProcessStartup = new air.NativeProcessStartupInfo();
					nativeProcess = new air.NativeProcess();
					// setup arguments
					var args = new air.Vector["<String>"]();
					// PC or Mac?
					if (air.Capabilities.os.toLowerCase().indexOf("win") > -1)
						{
						processName = air.File.applicationDirectory.resolvePath("ncc.exe");
						//alert("win");
						}
					else if (air.Capabilities.os.toLowerCase().indexOf("mac") > -1)
						{
						// in application directory
						processName = air.File.applicationDirectory.resolvePath("ncc");
						//processName = new air.File("/bin/cat");
						//alert("mac");
						}
					else if (air.Capabilities.os.toLowerCase().indexOf("Linux") > -1)
						{
						// in application directory
						processName = air.File.applicationDirectory.resolvePath("ncc_linx");
						//processName = new air.File("/bin/cat");
						alert("linux");
						} 
					else
						{
						alert("Error #810 - Can't tell if this is a Mac, PC, or Linux");
						}

					nativeProcessStartup.executable = processName;
					
					// add the path as an argument
					args.push(filePath.nativePath);
					nativeProcessStartup.arguments = args;
					nativeProcess.addEventListener(air.ProgressEvent.STANDARD_OUTPUT_DATA,outputHandler);
					// start the process
					nativeProcess.start(nativeProcessStartup);
				 }
				else
				{
				alert("NativeProcess not supported");
				}
				// will return the value of the outputHandler if it's NULL or blank
				if (window.deviceFile!=null || window.deviceFile!="")
				{	return window.deviceFile;	}
			}
			else
			{
				// otherwise do nothing if the file doesn't exist
				//alert("File doesn't exist");
			}
 		}
 		
		/* outputHandler()
		* Grabs stdout from the middleman USB app, used in readDevice()
		*/
		function outputHandler(event)
		{
		window.deviceFile = nativeProcess.standardOutput.readUTFBytes(nativeProcess.standardOutput.bytesAvailable);
		}
	
	
 		/* readFile()
		* Opens a given filestream and reads it into a varaiable
		* (If the file does not exist, should be an error!) 
		*/
		function readFile(filePath)
		{
			stream = new air.FileStream();
			if (filePath.exists) {
				stream.open(filePath, air.FileMode.READ);
			// get the file and put it in a variable
				theFile = stream.readUTFBytes(stream.bytesAvailable);
				stream.close();
				return theFile;
			}
			else
			{
				// if the file is not found, nothing
				air.trace(filePath.name+ " not found");
			}
			
			// what's this for?
			window.nativeWindow.visible = true;
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
			$("#progressbar").progressbar({ value: "0"});
			$("#minutesRemaining").html("");
				
			// Create the string to write out
			var stopPCR = 's=ACGTC&c=stop';
			// contrast
			//// contrast no longer controlled here, delete
			////stopPCR += '&t=50';
			// increment the window.command id and send the new command to the device
			window.command_id++;
			stopPCR += '&d='+ window.command_id;
			air.trace(stopPCR);
			// Write out the STOP command to CONTROL.TXT
			// name of the output file
			var file = window.path.resolvePath("CONTROL.TXT"); 
			// write out all the variables, command id + PCR settings
			var fileStream = new window.runtime.flash.filesystem.FileStream();
			fileStream.open(file, window.runtime.flash.filesystem.FileMode.WRITE); 
			fileStream.writeUTFBytes(stopPCR); 
			fileStream.close();
			// go back to the Form page
			sp2.showPanel(1);
			return false;
		}

	/* humanTime()
	* Input: seconds (integer)
	* Returns: time in a human friendly format, i.e. 2 hours, 10 minutes, 1 hour, 10 minutes, 1 hour, 1 minute, 60 minutes, 1 minute
	*/
	function humanTime(secondsRemaining)
		{
			var timeRemaining="";
			var minutesRemaining = Math.floor(secondsRemaining/60);
			var hoursRemaining = Math.floor(minutesRemaining/60);
				if (hoursRemaining>0)
					{
					timeRemaining+= hoursRemaining + " hour";
					if (hoursRemaining>1) 
						{
						timeRemaining+="s ";
						}
						else {timeRemaining+=" ";}
					timeRemaining+= "<br />";
					minutesRemaining-=(hoursRemaining)*60;
					}
						if (minutesRemaining>1)
								{
								timeRemaining+=minutesRemaining + " minutes";
								}
						else if (minutesRemaining==1)
							{
							timeRemaining+= "1 minute";
							}
						else if (secondsRemaining<=60)
							{
							// should say "less than a minute" but font is too big
							timeRemaining+= "1 minute";
							}
						else if (secondsRemaining==0)
							{
							timeRemaining = "Done!";
								}
			return timeRemaining;
		}
		
/**************
* Buttons     *
***************/

	/*  "About" button on the OpenPCR Home page
	* Displays about info
	*/		
	 $('#About').live('click', function(){
						$('#about_dialog').dialog('open');
				 });
				 
	/*  "Contrast" button on the OpenPCR Home page
	* Sets the contrast for OpenPCR
	*/		
	 $('#Settings').live('click', function(){
						$('#settings_dialog').dialog('open');
				 });
	
	/*  "Home" button on the OpenPCR Form page
	* Goes Home
	*/	
	$('#Home').live('click', function(){
					listExperiments();
					sp2.showPanel(0);
					setTimeout(clearForm,500);
				 });
				 
	/*  "Home" button on the OpenPCR Running page */	
	$('#homeButton').live('click', function(){
					stopPCR();
					listExperiments();
					sp2.showPanel(0);
					setTimeout(clearForm,500);
				 });
		 
	/*  "Start" button on the OpenPCR Form page
	* Sends an experiment to OpenPCR and switches to the Running page
	*/	
	$('#Start').live('click', function(){
			startPCR();
		});
					
	/*  "Save" button on the OpenPCR Form
	 * Ask for a "name" and save the protocol to name.pcr in the user's Experiments folder
	 */		
	 $('#Save').live('click', function(){
				 // Save Dialog
				 // check if the form is validated
						if (false == ($("#pcrForm").validate().form()))
							{ return 0; // if not, don't do anything
							}
				// otherwise, the form is valid. Open the "Save" dialog box
						$('#save_form').dialog('open');
		 });
				 
	/*  "Save" on the OpenPCR Form in EDIT MODE
	* This will overwrite the old experiment with the edited settings
	*/		
	$('#SaveEdits').live('click', function(){
			 // check if the form is validated
					if (false == ($("#pcrForm").validate().form()))
						{ 
							return 0; // if not, don't do anything
						}
			 // Grab the Experiment name, could also do this by reading from the experiments list on the homepage
			 name = document.getElementById("ExperimentName").innerHTML;
			 // Save the file, overwriting the existing file
			 Save(name);
			 // re-load the experiment with the new settings
			loadExperiment(experimentID);
	 });
				 
	/*  "Cancel" button on the OpenPCR Form in EDIT MODE
	* This will cancel any changes made to the form and re-load the experiment as it was last saved
	*/		
	$('#Cancel').live('click', function(){
			// what is selected in the drop down menu on the front page?
			experimentID = $("#dropdown").val();
			// clear the form
			clearForm();
			// load the selected experiment
			loadExperiment(experimentID);
			 });
	
	
	/*  "Edit" button on the OpenPCR Form with a saved experiment
	*/	
	$('#editButton').live('click', function(){
		editButton();
		});
	
	/*  "Delete" button on the OpenPCR Form in EDIT MODE
	*/	
	$('#deleteButton').live('click', function(){
		$('#delete_dialog').dialog('open');
		});
	
	/*  "+ Add Step" button on the OpenPCR Form
	* Add a new blank step to the end of the presets
	*/	
	$('#addStepButton').live('click', function() {
		var location = $(this).parent().attr("id");
		addStep(location);
		});
	
	/*  "- Delete Step" on the OpenPCR Form
	* Delete the step
	*/	
	$('.deleteStepButton').live('click', function() {
		$(this).parent().slideUp('slow', function() {
			// after animation is complete, remove parent step
			$(this).remove();	
	//// if the length is now 0, hide the whole div
			});
	
	});
	
	/*  "More options" button on the OpenPCR Form
	* Display a bunch of options
	*/
	$('#OptionsButton').live('click', function() {
		$(".edit").toggle();
		$("#preContainer").show();
		$("#postContainer").show();
		$("#lidContainer").show();
		// get current state
		buttonText = document.getElementById("OptionsButton").innerHTML;
		// if we're hiding the options and there are no pre-steps or post-steps, hide those sections appropriately
		if (buttonText == 'Less options' && $("#preSteps").html() == "")
				{
			// hide pre steps
				$("#preContainer").hide();
				}
			if (buttonText == 'Less options' && $("#postSteps").html() == "")
				{
			// hide post steps
				$("#postContainer").hide();
				}
		// flip the Options button text between "More options" and "Less options"
		var buttonText = (buttonText != 'More options' ? 'More options' : 'Less options' );
		$('#OptionsButton').html(buttonText);	
		});
		
// Presets page
		/* editButton()
		* Function that is called when the "Edit" button is pressed on a "Saved Preset" page. Makes the "Save preset" and "Cancel" buttons
		* show up, "Add" and "Subtract" steps buttons, and makes all fields editable
		* Returns: nothing
		*/
		function editButton()
		{
			
		// Show the Delete button
		$('#deleteButton').show();
		// Start with the Edit button hidden
			$("#editButton").hide();
		// show the edit buttons
			$(".edit").show();
		// show the lid temp fields
			$("#lidContainer").show();
		// all fields editable
			$("input").attr("readonly","");
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
		
		/* deleteCurrentExperiment()
		* Deletes the currently loaded experiment (whatever was last selected in the list)
		* Called by the delete dialog box
		*/
		function deleteCurrentExperiment()
		{
		// delete the currently loaded Experiment file
		// given an ID, get the path for that ID
		experimentPath = window.experimentList[experimentID];
		// delete the file
		var file = experimentPath; 
		file.deleteFile();
		// show a confirmation screen
		$('#delete_confirmation_dialog').dialog('open');
		// then close it after 1 second
		setTimeout(function(){$('#delete_confirmation_dialog').dialog('close');}, 750);

		// 
		}
		
		/* addStep()
		* Add the HTML for a blank step to the desired css selector div
		*/
		function addStep(location)
		{
			// first off, if the location is cycleContainer, we really want to modify stepsContainer
			if (location == "cycleContainer")
				{
				location = "cycleSteps";
				}
			// add to HTML
			if (location=="preSteps") { step_name="Initial Step" }
			if (location=="postSteps") { step_name="Final Step" }
			if (location=="cycleSteps") { step_name="Step" }
			step_number = new Date().getTime();;
			var step =
				'<div class="step">' +
					'<span class="title">' + step_name + ' </span>' +
					'<a class="edit deleteStepButton"><img src="images/minus.png" height="30"></a>' +
					'<table cellspacing="20">' +
						'<tr>' +
							'<th><label>temp</label><div><input type="text" style="font-weight:normal;" class="required number textinput" value="" name="temp_' + step_number + '" maxlength="4" min="0" max="120" ></div><span htmlfor="openpcr_temp" generated="true" class="units">&deg;C</span> </th>' +
							'<th><label>step duration</label><div class=""><input type="text" class="required number textinput" style="font-weight:normal;" value=""  name="time_' + step_number + '" maxlength="4" min="0" max="1000"></div><span htmlfor="openpcr_time" generated="true" class="units">sec</span></th>' +
							'<th><label>ramp duration</label><div class=""><input type="text" class="required number textinput" style="font-weight:normal;" value=""  name="rampDuration_' + step_number + '" maxlength="6" min="0" max="999999"></div><span htmlfor="openpcr_rampDuration" generated="true" class="units">sec</span></th>' +
						'</tr>' +
					'</table>' +
				'</div>';
				// append a new step to location
				$('#' + location).append(step);
				// make sure the form elements are editable
				$("input").attr("readonly","");
		//// make the window bigger
				// make all the delete buttons shown
				// and if there are any other parts of a "step" that are hide/show, they need to be included here
					$(".edit").show();
					
		}
		
			function addInitialStep()
		{
		// add the step to the preContainer
		addStep("preSteps");
		}
		
	function addFinalStep()
		{
		// add the step to the postContainer
		addStep("postSteps");
		}
		
		/* deleteStep()
		* Delete the parent step
		*/
		function deleteStep()
		{
			// doesn't do anything right now. The delete step button should reference here
		}
						
		// JQUERY UI stuffs

$(function(){
	// About Dialog			
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
		
	// Settings Dialog			
		$('#settings_dialog').dialog({
			autoOpen: false,
			width: 400,
			modal: true,
			draggable: false,
			resizable: false,
			buttons:
				{
				"Apply": function() {
					 // grab the value of the slider
					  contrast = $("#contrast_slider").slider("value");
					// command id
					window.command_id=Math.floor(Math.random()*65534);
					// set the command
					contrast_string = 's=ACGTC&c=cfg&o=' + contrast + '&d=' + command_id;
					
					// trace it
					 air.trace("string: " + contrast_string);
					
					// Write out the  command to CONTROL.TXT
					// name of the output file
					if (window.path != null)
						{
					var file = window.path.resolvePath("CONTROL.TXT"); 
					// write out all the variables, command id + PCR settings
					var fileStream = new window.runtime.flash.filesystem.FileStream();
					fileStream.open(file, window.runtime.flash.filesystem.FileMode.WRITE); 
					fileStream.writeUTFBytes(contrast_string); 
					fileStream.close();
						}
					},
					"OK": function() {
					 // grab the value of the slider
					  contrast = $("#contrast_slider").slider("value");
					  window.command_id=Math.floor(Math.random()*65534);
					  contrast_string = 's=ACGTC&c=cfg&o=' + contrast + '&d=' + command_id;
					 // trace it
					 air.trace("string: " + contrast_string);
					 // Write out the  command to CONTROL.TXT
					// name of the output file
					if (window.path != null)
					{
					var file = window.path.resolvePath("CONTROL.TXT"); 
					// write out all the variables, command id + PCR settings
					var fileStream = new window.runtime.flash.filesystem.FileStream();
					fileStream.open(file, window.runtime.flash.filesystem.FileMode.WRITE); 
					fileStream.writeUTFBytes(contrast_string); 
					fileStream.close();
					}
					 // close the dialog window
					 $(this).dialog("close"); 
					}
				
				}
		});
		
		$(function() {
		$( "#contrast_slider" ).slider(
			{
			min: 1,
			max: 250
			}
			);
		});
		
	// Save Dialog			
		$('#save_form').dialog({
			autoOpen: false,
			width: 300,
			modal: true,
			draggable: false,
			resizable: false,
			position: 'center',
			buttons:
				{
				"Cancel": function() {
					$(this).dialog("close"); 
					 $("#name").val("");
					},
				"Save": function() { 
					 // grab the name from the form
					 name = $("#name").val();
					 // save the current experiment as the given name
					 Save(name);
					 // update the experiment name in the UI
					 $("#ExperimentName").html(name);
					 // close the dialog window
					 $(this).dialog("close"); 
					}
				}
		});
		
	// Save Confirmation Dialog
	$('#save_confirmation_dialog').dialog({
			autoOpen: false,
			width: 300,
			modal: true,
			draggable: false,
			resizable: false
			
		});
		
	// Delete Dialog			
		$('#delete_dialog').dialog({
			autoOpen: false,
			width: 300,
			modal: true,
			draggable: false,
			resizable: false,
			buttons:
				{
				"No": function() {
					$(this).dialog("close"); 
					},
				"Yes": function() { 
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
			autoOpen: false,
			width: 300,
			modal: true,
			draggable: false,
			resizable: false
			
		});
		
	// Stop Dialog			
		$('#stop_dialog').dialog({
			autoOpen: false,
			width: 300,
			modal: true,
			draggable: false,
			resizable: false,
			buttons: {
				"No": function() { 
					$(this).dialog("close"); 
				}, 
				"Yes": function() { 
					$(this).dialog("close"); 
					stopPCR();
				}
			}
		});
		
	// Dialog Link
		$('#stop_link').click(function(){
			$('#stop_dialog').dialog('open');
			return false;
		});
	
	// Starting dialog
	$('#starting').dialog({
		autoOpen: false,
			width: 300,
			modal: true,
			draggable: false,
			resizable: false,
		});
		
	//hover states on the static widgets
		$('#dialog_link, ul#icons li').hover(
			function() { $(this).addClass('ui-state-hover'); }, 
			function() { $(this).removeClass('ui-state-hover'); }
		);
		
});	

// Enter/Return Key clicks "Save" on dialog
	$('#save_form').live('keyup', function(e){
  if (e.keyCode == 13) {
    $(':button:contains("Save")').click();
  }
	});
	
// Debugging function
		/* writeCSV(a,b,c,d,e,f,g,h)
		* Writes out current status to the histor file in comma delimited format
		* Can load this into Excel and see ramp times, etc
		* This should be commented out in actual releases
		*/
		function writeCSV(a,b,c,d,e,f,g,h)
			{
			// take in all 8 variables and write them to a new line in the history file
			fileDestination = air.File.applicationStorageDirectory.resolvePath("history.txt");
				// write out the file
					var fileStream = new window.runtime.flash.filesystem.FileStream();
					fileStream.open(fileDestination, window.runtime.flash.filesystem.FileMode.APPEND); 
					fileStream.writeUTFBytes(a + "," + b + "," + c + "," + d + "," + e + "," + f + "," + g + "," + h + "\n"); 
					fileStream.close();
			}
