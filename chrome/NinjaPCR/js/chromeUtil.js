var chromeUtil = {
};
chromeUtil.getOS = function () {
		Log.v("getOS OS=" + navigator.appVersion);
		return navigator.appVersion;
};
chromeUtil.alert = function (message) {
	Log.v("Alert " + message);
	if (!chromeUtil.alertDialogInit) {
		chromeUtil.alertDialogInit = true;
		$('#alert_dialog').dialog({
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
	}
	$('#alert_dialog_content')[0].innerHTML = message;
	$('#alert_dialog').dialog('open');
};

/* humanTime()
 * Input: seconds (integer)
 * Returns: time in a human friendly format, i.e. 2 hours, 10 minutes, 1 hour, 10 minutes, 1 hour, 1 minute, 60 minutes, 1 minute
 */
function humanTime(secondsRemaining) {
	var timeRemaining = "";
	var minutesRemaining = Math.floor(secondsRemaining / 60);
	var hoursRemaining = Math.floor(minutesRemaining / 60);
	if (hoursRemaining > 0) {
		timeRemaining += hoursRemaining + " " + chrome.i18n.getMessage((hoursRemaining>1)?'hours':'hour');
		timeRemaining += " ";
		minutesRemaining -= (hoursRemaining) * 60;
	}
	if (minutesRemaining > 1) {
		timeRemaining += minutesRemaining + " " + chrome.i18n.getMessage('minutes');
	}
	else if (minutesRemaining == 1) {
		timeRemaining += chrome.i18n.getMessage('minute1');
	}
	else if (secondsRemaining <= 60) {
		// should say "less than a minute" but font is too big
		timeRemaining += chrome.i18n.getMessage('minute1');
	}
	else if (secondsRemaining == 0) {
		timeRemaining = chrome.i18n.getMessage('done');
	}
	return timeRemaining;
};

function clockTime (totalSec) {
	totalSec = Math.floor(totalSec);
	var sec = totalSec%60;
	var min = Math.floor(totalSec/60)%60;
	var hour = Math.floor(totalSec/(60*60))%60;
	return fillZero(hour, 2) + ":" +
		fillZero(min, 2) + ":" +
		fillZero(sec, 2);
}

chromeUtil.alertUpdate = function (currentVersion, latestVersion) {
	var message = chrome.i18n.getMessage('firmwareVersionDialog')
		.replace("___LATEST_VERSION___", latestVersion)
		.replace("___INSTALLED_VERSION___", currentVersion);
	Log.v(message);
	$('#update_dialog_content')[0].innerHTML = message;
	$('#update_dialog').show();
}

var Storage = function () {
	this.data = {};
	this.experiments = [];
	this.data["experiments"] = this.experiments;
};
var STORAGE_KEY_EXPERIMENT_DATA = "pcrData";
var STORAGE_KEY_EXPERIMENT_LIST = "experimentList";
var STORAGE_KEY_EXPERIMENT_PREFIX = "pcr_";

Storage.prototype.loadList = function (callback) {
	Log.v("Storage.prototype.loadList");
	var self = this;
	chrome.storage.sync.get(STORAGE_KEY_EXPERIMENT_LIST, function (data) {
		Log.v("Load done. data=" + data);
		if (data[STORAGE_KEY_EXPERIMENT_LIST])
			self.experiments = JSON.parse(data[STORAGE_KEY_EXPERIMENT_LIST]);
		else
			self.experiments = [];
		if (self.experiments && self.experiments.length>0) {
			Log.v("Storage.loadList Experiment List Found.");
			for (var i=0; i<self.experiments.length; i++) {
				Log.v(self.experiments[i].name);
			}
			callback(self.experiments);
		} else {
			//Empty
			Log.v("Empty. Add default experiment and save.");
			self.insertDefaultExperiment(callback);
		}
	});
};
Storage.prototype.insertDefaultExperiment = function (callback) {
	var self = this;
	this.insertExperiment (DEFAULT_EXPERIMENT.name, DEFAULT_EXPERIMENT, function () {
		Log.i("Default experiment was inserted.");
		self.loadList(callback);
	});
}
Storage.prototype.loadExperiment = function (experimentId, callback) {
	var key = this.getKeyForId(experimentId);
	var self = this;
	chrome.storage.sync.get(key, function(data){
		var dataStr = data[key];
		Log.v("Data str=" + dataStr);
		var experiment = null;
		
		if (dataStr!=null) {
			try {
				experiment = JSON.parse(dataStr);
			} catch (e) {
				Log.e(e);
			}
		}
		self.currentExperimentId = experimentId;
		self.currentExperiment = experiment;
		callback(experiment);
	});
	
};
Storage.prototype.clearAllData = function () {
	Log.v("clearAllData");
	chrome.storage.sync.clear (function(){
		Log.v("Done.");
	});
};
Storage.prototype.generateId = function () {
	return new Date().getTime();
};
Storage.prototype.getKeyForId = function (id) {
	return STORAGE_KEY_EXPERIMENT_PREFIX + id;
};
Storage.prototype.updateCurrentExperiment = function (name, newData, callback) {
	var id = this.currentExperimentId;
	var key = this.getKeyForId(id);
	for (var i=0; i<this.experiments.length; i++) {
		var experiment = this.experiments[i];
		if (experiment.id==id) {
			Log.v("Old name=" + experiment.name);
			this.experiments[i].name = name;
		}
	}
	var storageObj = {};
	storageObj[STORAGE_KEY_EXPERIMENT_LIST] = JSON.stringify(this.experiments, null, '');
	var self = this;
	chrome.storage.sync.set(storageObj, function() {
			Log.v('Experiment "'+name+'" saved');
			var detailStorageObj = {};
			detailStorageObj[key]  = JSON.stringify(newData, null, '');
			chrome.storage.sync.set(detailStorageObj, function() {
				chrome.storage.sync.get(key, function(data){
					var dataStr = data[key];
					callback("success");
				});
			});
		});
};
Storage.prototype.deleteCurrentExperiment = function (callback) {
	Log.v("deleteCurrentExperiment " + this.currentExperimentId);

	for (var i=0; i<this.experiments.length; i++) {
		var experiment = this.experiments[i];
		if (experiment.id==this.currentExperimentId) {
			Log.v("Remove " + i);
			this.experiments.splice(i, 1);
			break;
		}
	}
	var storageObj = {};
	storageObj[STORAGE_KEY_EXPERIMENT_LIST] = JSON.stringify(this.experiments, null, '');
	var self = this;
	chrome.storage.sync.set(storageObj, function() {
			Log.v('List saved.');
			var detailStorageObj = {};
			var key = self.getKeyForId(self.currentExperimentId);
			chrome.storage.sync.remove(key, function() {
				Log.v('Detail data removed.');
				callback();
			});
		});
}
Storage.prototype.insertExperiment = function (name, experiment, callback) {
	var id = this.generateId();
	var experimentData = {
			"name":name,
			"id":id
	};
	this.experiments.push(experimentData);
	var storageObj = {};
	storageObj[STORAGE_KEY_EXPERIMENT_LIST] = JSON.stringify(this.experiments, null, '');
	var self = this;
	chrome.storage.sync.set(storageObj, function() {
			Log.v('Experiment "'+name+'" saved');
			var detailStorageObj = {};
			var key = self.getKeyForId(id);
			detailStorageObj[key]  = JSON.stringify(experiment, null, '');
			chrome.storage.sync.set(detailStorageObj, function() {
				chrome.storage.sync.get(key, function(data){
					var dataStr = data[key];
					callback("success");
				});
			});
		});
};
Storage.prototype.updateExperiment = function (experiment) {
	Log.v("Storage#updateExperiment");
};

Storage.prototype.getLogFileName = function () {
	Log.d("Storage.getLogFileName experiment=" + this.currentExperiment);
	var time = new Date();
	var experimentName = (this.currentExperiment)?this.currentExperiment.name:'New Experiment';
	var fileName = experimentName.replace(/ /g, "_");
	fileName += "_";
	fileName += fillZero(time.getFullYear(),4);
	fileName += fillZero(time.getMonth()+1,2);
	fileName += fillZero(time.getDate(),2);
	fileName += fillZero(time.getHours(),2);
	fileName += fillZero(time.getMinutes(),2);
	fileName += ".csv";
	return fileName;
}
function fillZero(num, length) {
	var str = ""+num;
	while (str.length<length) {
		str = "0" + str;
	}
	return str;
}
