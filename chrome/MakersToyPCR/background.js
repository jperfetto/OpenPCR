chrome.app.runtime.onLaunched.addListener(function() {
	chrome.app.window.create('MakersToyPCR.html', {
		'bounds': {
			'width': 700,
			'height': 800
		}
	});
});