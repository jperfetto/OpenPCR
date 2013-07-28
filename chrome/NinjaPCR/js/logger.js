var Log = {};
Log.Level =
{
		VERBOSE: 1,
		DEBUG: 2,
		INFO: 3,
		WARNING: 4,
		ERROR: 5
};
Log.FILTER_LEVEL = Log.Level.DEBUG;

Log._getDate = function () {

	var time = new Date();
	var str = fillZero(time.getFullYear(),4);
	str += "/";
	str += fillZero(time.getMonth()+1,2);
	str += "/";
	str += fillZero(time.getDate(),2);
	str += " ";
	str += fillZero(time.getHours(),2);
	str += ":";
	str += fillZero(time.getMinutes(),2);
	str += ":";
	str += fillZero(time.getSeconds(),2);
	return str;
}
Log._write = function (message, level, label) {
	if (level >= Log.FILTER_LEVEL)
	{
		console.log ("[NinjaPCR]\t" + "[" + label + "]\t" + Log._getDate() + "\t" + message);
	}
}
Log.v = function (message) { Log._write(message, Log.Level.VERBOSE, "v"); };
Log.d = function (message) { Log._write(message, Log.Level.DEBUG, "d"); };
Log.i = function (message) { Log._write(message, Log.Level.INFO, "i"); };
Log.w = function (message) { Log._write(message, Log.Level.WARNING, "w"); };
Log.e = function (message) { Log._write(message, Log.Level.ERROR, "e"); };