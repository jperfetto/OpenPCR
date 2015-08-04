var TempGraph = function (container) {
	this.scaleIndex = 4; // Default stale: 30sec
	var options, start, i;
	
	this.container = container;
	this.dataLid = [];
	this.dataBlock = [];
	
	//Initialize Dummy Data
	this.time = 0;
	
	defaultAxis = {min : 0, max : this.getScale()};
	this.options = {
		xaxis:defaultAxis,
		title : chrome.i18n.getMessage('temperature')
	};
	var self = this;

	this.graph = this.drawGraph();

	function initializeDrag(e) {
		start = self.graph.getEventPosition(e);
		Flotr.EventAdapter.observe(document, 'mousemove', move);
		Flotr.EventAdapter.observe(document, 'mouseup', stopDrag);
	}

	function move(e) {
		var end = self.graph.getEventPosition(e), xaxis = self.graph.axes.x, 
		offset = start.x- end.x;

		self.graph = self.drawGraph( {
			xaxis : {
				min : xaxis.min + offset,
				max : xaxis.max + offset
			}
		});
		Flotr.EventAdapter.observe(self.graph.overlay, 'mousedown', initializeDrag);
	}

	function stopDrag() {
		Flotr.EventAdapter.stopObserving(document, 'mousemove', move);
	}
	Flotr.EventAdapter.observe(this.graph.overlay, 'mousedown', initializeDrag);
};
TempGraph.prototype.getScale = function () {
	return TempGraph.SCALES[this.scaleIndex];
};

TempGraph.SCALES =  [10, 20, 30, 60, 60*3, 60*8, 60*20, /*1h*/60*60, /*2h*/60*60*2, /*4h*/60*60*2, /*8h*/60*60*8 ];

TempGraph.prototype.drawGraph = function (opts) {
	var o = Flotr._.extend(Flotr._.clone(this.options), opts || {});
	o.legend = {position:'sw'}; //Set position of legend
	o.yaxis = {
			min:0,
			max:130
		};
	return Flotr.draw(
			this.container, 
			[{data:this.dataLid,label:chrome.i18n.getMessage('lid')}, {data:this.dataBlock,label:chrome.i18n.getMessage('block')} ],
			o);
}

TempGraph.prototype.addTime = function (elapsedSec, tempLid, tempBlock) {
	this.dataLid.push([elapsedSec, tempLid]);
	this.dataBlock.push([elapsedSec, tempBlock]);
	this.drawCurrentData ();
	this.time = elapsedSec;
};

TempGraph.prototype.clear = function () {
	this.dataLid = [];
	this.dataBlock = [];
	this.time = 0;
	this.drawCurrentData ();
};

TempGraph.prototype.drawCurrentData = function () {
	var min = Math.max(0, this.time-this.getScale());
	max = min + this.getScale();
	var axis = {
			min : min,
			max : max
		};
	this.graph = this.drawGraph( {
		xaxis : axis
	});
	
}
TempGraph.prototype.changeScale = function (delta) {
	this.scaleIndex = Math.max( Math.min (this.scaleIndex+delta, TempGraph.SCALES.length-1), 0);
	this.drawCurrentData();
};
var graph = new TempGraph(document.getElementById("graph_container"));