var MESSAGE_FROM_DEVICE = new RegExp("pcr([0-9\\.]+?)\\n");

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
				Log.d("Finish scanning.");
				callback(self.foundPort, self.connectionId, self.firmwareVersion);
			} else {
				self.findPcrPort(callback);
			}
		});
	}
};

//Private
SerialPortScanner.prototype._scan = function(callback) {
	var port = this.ports[this.currentPortIndex];
	if (port.match(PORT_TO_IGNORE)) {
		Log.d("Ignore port " + port);
		callback(null);
		return;
	}
	var self = this;
	var options = {
			bitrate:BAUD_RATE
	};
	Log.v("Opening port " + port);
	chrome.serial.open(port, options, function (openInfo) {
		var connectionId = openInfo.connectionId;
		if (connectionId<0) {
			Log.e("Connection error. ID=" + connectionId);
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
			Log.d("Message=" + self.readMessage);
			if (self.readMessage.match(MESSAGE_FROM_DEVICE)) {
				var version = RegExp.$1;
				Log.i("Device found. Firmware version " + version);
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
					Log.d("Device was not found on port " + port);
					callback();
				});
			}
		}
	});
};
