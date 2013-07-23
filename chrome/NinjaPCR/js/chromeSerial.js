var BAUD_RATE = 4800;

var Serial = function () {
	this.port = null;
};

/* Command Code */
var SEND_CMD = 0x10;
var STATUS_REQ = 0x40;
var STATUS_RESP = 0x80;
var START_CODE = 0xFF;
var END_CODE = 0xFE;

Serial.prototype.scan = function (callbackExternal, _wait) {
	var wait = _wait || 100;
	if (this.locked) {
		callbackExternal();
	}
	this.locked = true;
	var self = this;
	chrome.serial.getPorts(function (ports) {
		if (!self.latestPorts) self.latestPorts = [];
		var portsToSearch = [];
		for (var i=0, l=ports.length; i<l; i++) {
			var found = false;
			for (var j=0; j<self.latestPorts.length; j++) {
				if (self.latestPorts[j]==ports[i]) found = true;
			}
			if (!found) {
				// New Port Found!
				console.log("New Port Found! " + ports[i]);
				portsToSearch.push(ports[i]);
			}
		}
		self.latestPorts = ports;
		if (portsToSearch.length>0) {
			window.setTimeout(self._getScanFunc(callbackExternal, portsToSearch), wait);
		}
		else {
			callbackExternal(null);
		}
	});
};
Serial.prototype._getScanFunc = function (callbackExternal, portsToSearch) {
	var self = this;
	return function () {
		new SerialPortScanner(portsToSearch).findPcrPort(
				function (port, connectionId, firmwareVersion) {
					self.port = port;
					self.firmwareVersion = firmwareVersion;
					self.connectionId = connectionId;
					self.locked = false;
					callbackExternal(port);
				});
		
	}
};

Serial.prototype.startWithCommand = function (commandBody) {
	this.complete = false;
	console.log("------------------------------------------------");
	console.log("Start Communication");
	console.log("------------------------------------------------");
	console.log("Program=" + commandBody);
	var port = this.port;
	var options = {
			bitrate:BAUD_RATE
	};
	var self = this;
	var connectionId = self.connectionId;
	if (connectionId<0) {
		console.log("Connection error.");
	} else {
		var data = getFullCommand(commandBody, SEND_CMD);
		chrome.serial.write(connectionId, data, function (sendInfo){
			self.startListeningStatus(port, connectionId);
		});
	}
};
/**
 * Complete
 */
Serial.prototype.stopOnComplete = function () {
	this.complete = true;
};
/**
 * Convert String to Command Byte Array (Including Header)
 */
function getFullCommand(commandBody, /* char */commandCode) {
	var length = commandBody.length;
	var buff = new ArrayBuffer(length + 4);
	var arr = new Uint8Array(buff);
	
	arr[0] = START_CODE;
	arr[1] = commandCode;
	arr[2] = length;
	for (var i=0; i<commandBody.length; i++) {
		arr[i+3] = commandBody.charCodeAt(i);
	}
	arr[arr.length-1] = END_CODE;
	return buff;
}
/**
 * Send "Stop" Command and Wait for Response
 */
Serial.prototype.sendStopCommand = function (command, callback) {
	var self = this;
	var connectionId = self.connectionId;
	if (connectionId<0) {
		console.log("Connection error. ID=" + connectionId);
	} else {
		var data = getFullCommand(command, SEND_CMD);
		chrome.serial.write(connectionId, data, function (sendInfo){
			self.startListeningStatus(self.port, connectionId);
		});
	}
};
/**
 * Request Status and Wait for Response
 */
Serial.prototype.requestStatus = function (callback) {
	this.txBusy = true;
	this.onReceiveStatus = onReceiveStatus;
	var port = this.port;
	var options = {
			bitrate:BAUD_RATE
	};
	var self = this;
	var connectionId = self.connectionId;
	var data = getFullCommand("", STATUS_REQ);
	chrome.serial.write(connectionId, data, function (sendInfo){
		self.txBusy = false;
		self.startListeningStatus(port, connectionId);
	});
};

Serial.prototype.startListeningStatus = function (port, connectionId) {
	this.readMessage = "";
	this.listenStatus(port, connectionId);
};
Serial.prototype.listenStatus = function (port, connectionId) {
	if (this.txBusy || this.complete) {
		return;
	}
	var self = this;
	chrome.serial.read(connectionId, Serial.BYTES_TO_READ, function (readInfo) {
		if (readInfo.bytesRead>0) {
			var arr = new Uint8Array(readInfo.data);
			for (var i=0; i<arr.length; i++) {
				self.processByte(arr[i]);
			}
		}
		self.listenStatus(port, connectionId);
	});
}

var nextByteIndex = 0;
var currentCommand = 0;
var bodyLength = 0;
var remainingBody = 0;
var startFound = false;
var waitingForMessage = true;
var commandBody = [];
//Format: [START_CODE(0xFF)][command][length][ data ][END_CODE(0xEF)]
Serial.prototype.processByte= function(readByte) {
	if (waitingForMessage && START_CODE==readByte) {
		// Start code found.
		startFound = true;
		waitingForMessage = false;
		nextByteIndex++;
	} 
	else if(nextByteIndex==1) {
		// Read command code
		currentCommand = readByte; // SEND_CMD or STATUS_REQ
		nextByteIndex++;
	} 
	else if (nextByteIndex==2) {
		// Read body length
		bodyLength = readByte;
		remainingBody = bodyLength;
		nextByteIndex++;
	} 
	else if (remainingBody>0) {
		// Read Body
		commandBody[nextByteIndex-3] = readByte;
		remainingBody--;
		nextByteIndex++;
	} 
	else if (END_CODE==readByte) {
		// Finish
		this.processPacket();
		this.finishReading();
	} 
	else	{
		// Other messages
		console.log("byte=" + String.fromCharCode(readByte));
		this.finishReading();
	}
}
Serial.prototype.processPacket = function () {
	var message = "";
	for (var i=0; i<bodyLength; i++) {
		message += String.fromCharCode(commandBody[i]);
	};
	console.log(message);
	if (this.onReceiveStatus)
		onReceiveStatus(message);
}
Serial.prototype.finishReading= function() {
	currentCommand = 0;
	bodyLength = 0;
	waitingForMessage = true;
	startFound = false; 
	nextByteIndex=0;
}
Serial.BYTES_TO_READ = 128;
Serial.REMOVE_SIGNAL = new RegExp("(pcr)+");
Serial.prototype.startListeningPort = function (port, connectionId) {
	this.readMessage = "";
	this.listenPort(port, connectionId);
};
Serial.prototype.listenPort = function (port, connectionId) {
	var self = this;
	chrome.serial.read(connectionId, Serial.BYTES_TO_READ, function (readInfo) {
		if (readInfo.bytesRead>0) {
			var message = String.fromCharCode.apply(null, new Uint8Array(readInfo.data));
			self.readMessage += message;
			self.readMessage = self.readMessage.replace(Serial.REMOVE_SIGNAL,'');
			console.log(self.readMessage);
		}
		self.listenPort(port, connectionId);
	});
}

var SerialPortScanner = function (ports) {
	this.ports = ports;
	this.currentPortIndex = 0;
	this.foundPort = null;
};

SerialPortScanner.prototype.findPcrPort = function (callback) {
	if (!this.ports || this.ports.length==0) {
		//No port found. return null.
		callback(null);
		return;
	}
	var self = this;
	// Scan port[currentPortIndex]
	if (this.currentPortIndex==this.ports.length) {
		callback(null, 0);
	} else {
		// Scan next port
		this._scan(function(){
			self.currentPortIndex++;
			if (self.foundPort) {
				console.log("Finish scanning.");
				callback(self.foundPort, self.connectionId, self.firmwareVersion);
			} else {
				self.findPcrPort(callback);
			}
		});
	}
};
var PORT_TO_IGNORE = new RegExp("Bluetooth");
var MESSAGE_FROM_DEVICE = new RegExp("pcr(.+?).");
//Private
SerialPortScanner.prototype._scan = function(callback) {
	var port = this.ports[this.currentPortIndex];
	if (port.match(PORT_TO_IGNORE)) {
		console.log("Ignore port " + port);
		callback(null);
		return;
	}
	var self = this;
	var options = {
			bitrate:BAUD_RATE
	};
	console.log("Opening port " + port);
	chrome.serial.open(port, options, function (openInfo) {
		var connectionId = openInfo.connectionId;
		if (connectionId<0) {
			console.log("Connection error. ID=" + connectionId);
			callback(null);
		} else {
			if (onSerialOpen)
				onSerialOpen(connectionId);
			self.scanStartTimestamp = new Date().getTime();
			self.readMessage = "";
			self._read(connectionId, callback);
		}
	});
};
SerialPortScanner.DURATION_MSEC = 3000;
SerialPortScanner.BYTES_TO_READ = 64;

SerialPortScanner.INITIAL_MESSAGE = "";
for (var i=0; i<512; i++) {
	SerialPortScanner.INITIAL_MESSAGE += "a";
}
// Called from _scan"
SerialPortScanner.prototype._read = function (connectionId, callback) {
	var port = this.ports[this.currentPortIndex];
	var self = this;
	chrome.serial.read(connectionId, SerialPortScanner.BYTES_TO_READ, function (readInfo) {
		if (readInfo.bytesRead>0) {
			var message = String.fromCharCode.apply(null, new Uint8Array(readInfo.data));
			self.readMessage += message;
		}
		
		if (new Date().getTime()-self.scanStartTimestamp<SerialPortScanner.DURATION_MSEC) {
			// Continue reading until time limit
			self._read(connectionId, callback);
		} else {
			// Finish reading and check message
			console.log("Message=" + self.readMessage);
			if (self.readMessage.match(MESSAGE_FROM_DEVICE)) {
				var version = RegExp.$1;
				console.log("Firmware version " + version);
				self.foundPort = port;
				self.firmwareVersion = version;
				self.connectionId = connectionId;
				var data = getArrayBufferForString(SerialPortScanner.INITIAL_MESSAGE);
				chrome.serial.write(connectionId, data, function (sendInfo){
					chrome.serial.read(connectionId, 1024, function(){
						callback();
					});
				});
			} else {
				chrome.serial.close(connectionId, function () {
					console.log("Device was not found on port " + port);
					callback();
				});
			}
		}
	});
};

function getArrayBufferForString(str) {
	var buff = new ArrayBuffer(str.length);
	var arr = new Uint8Array(buff);
	for (var i=0; i<str.length; i++) {
		arr[i] = str.charCodeAt(i);
	}
	return buff;
}
var chromeSerial = new Serial();