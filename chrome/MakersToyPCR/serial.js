var select = null;
var connectionId = 0;
var selectedPort = null;
var reading = false;

function init () {
	select = document.getElementById('ports');
	
	document.getElementById('open').addEventListener('click', openPort);
	document.getElementById('write').addEventListener('click', writePort);
	document.getElementById('close').addEventListener('click', closePort);

	document.getElementById('clear').addEventListener('click', 
			function () {document.getElementById('textRead').value = '';});
	
	chrome.serial.getPorts(function (ports) {
		for (var i=0; i<ports.length; i++) {
			var port = ports[i];
			select.appendChild(new Option(port, port));
		}
	});
}

function openPort () {
	selectedPort = select.childNodes[select.selectedIndex].value;
	var baudRate = parseInt(document.getElementById('baud').value);
	console.log("Port " + selectedPort + " is selected. Baud=" + baudRate);
	var options = {
			bitrate:baudRate
	};
	chrome.serial.open(selectedPort, options, function (openInfo) {
		connectionId = openInfo.connectionId;
		console.log("openInfo.connectionId=" + connectionId);
		startReadingPort();
	});
	
}
function closePort () {
	chrome.serial.close(connectionId, function () {
		reading = false;
	});
}

var reading = false;
function startReadingPort () {
	reading = true;
	readPort();
}
function readPort() {
	var bytesToRead= 64;
	chrome.serial.read(connectionId, bytesToRead, function (readInfo) {
		if (readInfo.bytesRead==0) {
		} else {
			var arr = new Uint8Array(readInfo.data);
			
			document.getElementById('textRead').value += (String.fromCharCode.apply(null, arr));
		}
		if (reading)
			readPort();
	});
}
function writePort () {
	var data = getArrayBufferForString(document.getElementById('textWrite').value);
	chrome.serial.write(connectionId, data, function (sendInfo){
		document.getElementById('textWrite').value = '';
		})
}
function getArrayBufferForString(str) {
	var buff = new ArrayBuffer(str.length * 2);
	var arr = new Uint16Array(buff);
	for (var i=0; i<str.length; i++) {
		arr[i] = str.charCodeAt(i);
	}
	return buff;
}
window.onload = init;