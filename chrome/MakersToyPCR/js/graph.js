var TempGraph = function (container) {
	this.scaleIndex = 2; // Default stale: 30sec
	var options, start, i;
	
	this.container = container;
	this.dataLid = [];
	this.dataBlock = [];
	
	//Initialize Dummy Data
	this.time = 0;
	
	defaultAxis = {min : 0, max : this.getScale()};
	this.options = {
		xaxis:defaultAxis,
		title : 'Temperature'
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

TempGraph.SCALES =  [10, 20, 30, 60, 60*3, 60*8, 60*20, 60*60 ];

TempGraph.prototype.drawGraph = function (opts) {
	var o = Flotr._.extend(Flotr._.clone(this.options), opts || {});
	o.legend = {position:'sw'}; //Set position of legend
	o.yaxis = {
			min:0,
			max:130
		};
	return Flotr.draw(
			this.container, 
			[{data:this.dataLid,label:'Lid'}, {data:this.dataBlock,label:'Block'} ],
			o);
}

TempGraph.prototype.addTime = function (tempLid, tempBlock) {
	this.dataLid.push([this.time, tempLid]);
	this.dataBlock.push([this.time, tempBlock]);
	this.drawCurrentData ();
	this.time ++;
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