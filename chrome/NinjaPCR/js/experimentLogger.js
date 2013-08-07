/**
 * ExperimentLogger
 * -Store status list
 * -Show status as...
 * -- Graph
 * -- Meters
 * -- CSV
 */
var ExperimentLogger = function(){};
ExperimentLogger.prototype.start = function () {
	Log.d("ExperimentLogger.start")
	this.startTime = new Date();
	this.experimentLog = [];
};
ExperimentLogger.prototype.displayElapsedSec = function (elapsedSec) {
	document.getElementById('elapsedTime').innerHTML = clockTime(elapsedSec);
};
ExperimentLogger.prototype.log = function (status) {
	if (!status) return;
	Log.d("Elapsed time(msec)=" + (new Date().getTime()-this.startTime.getTime()));
	var elapsedSec = (new Date().getTime()-this.startTime.getTime())/1000;
	this.displayElapsedSec(elapsedSec);
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
	//if (Math.random()<0.1) status["s"]="complete"; //TODO debug

	var statusLid = status["x"].toFixed(1);
	var statusPeltier = status["y"].toFixed(1);
	$("#deviceStatusLid").html((statusLid>0)?chrome.i18n.getMessage('statusHeating'):chrome.i18n.getMessage('statusStop'));
	$("#deviceStatusLid").css("color", (statusLid>0)?COLOR_HEATING:COLOR_STOP);
	{
		var color;
		var text;
		if (statusPeltier>0) {
			color = COLOR_HEATING;
			text = chrome.i18n.getMessage('statusHeating');
		}
		else if (statusPeltier<0) {
			color = COLOR_COOLING;
			text = chrome.i18n.getMessage('statusCooling');
		} 
		else {
			color = COLOR_STOP;
			text = chrome.i18n.getMessage('statusStop');
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
			$("#minutesRemaining").html(chrome.i18n.getMessage('lidHeating'));

			// during lidwait, no protocol name is included, so include the protocol name from the previous page
			$("#runningHeader").html(
					document.getElementById("ExperimentName").innerHTML);
		}

		if (status["s"] == "running") {
			$("#timeRemaining").html(chrome.i18n.getMessage('timeRemaining'));
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
		graph.addTime(elapsedSec, lid_temp,block_temp);
		$('#meterBlock')[0].value = block_temp;
		$('#meterLid')[0].value = lid_temp;
	}
	else if (status["s"] == "complete") {
		// Current temp
		var block_temp = status["b"].toFixed(1);
		$("#blockTemperature").html(block_temp);
		// Current lid temp
		var lid_temp = status["l"].toFixed(1);
		$("#lidTemperature").html(lid_temp);
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
		minutesRemaining = '<span style="color:#04B109;">'+chrome.i18n.getMessage('done')+'</span>';
		$("#minutesRemaining").html(minutesRemaining);
		// update the "current temp"
		var block_temp = status["b"];
		$("#blockTemperature").html(block_temp);
		// update the lid temp
		var lid_temp = status["l"];
		$("#lidTemperature").html(lid_temp);
		// replace the "cycle # of total#" span with "PCR took..."
		$("#cycleNumOfNum").html(chrome.i18n.getMessage('tookTime').replace('___TIME___', humanTime(status["e"])));
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
};
