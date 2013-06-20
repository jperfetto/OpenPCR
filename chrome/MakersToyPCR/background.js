var connectionId = null;
chrome.app.runtime.onLaunched.addListener(function() {
	chrome.app.window.create(
		'MakersToyPCR.html', 
		{
			'bounds': {
				'width': 700,
				'height': 800
		},
	},
	function (createdWindow) {
		createdWindow.contentWindow.onSerialOpen = function (val) {
			console.log("onSerialOpen connectionId=" + val);
			connectionId = val;
		}
	});
});

chrome.runtime.onSuspend.addListener(function() {
	console.log("Shutting down the app...");
	if (connectionId) {
		chrome.serial.close(connectionId, function () {
			console.log("Closed serial connection. connectionId=" + connectionId);
		});
	}
});
