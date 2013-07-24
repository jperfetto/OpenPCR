var Log = {};
Log.Level =
{
		VERBOSE: 1,
		DEBUG: 2,
		INFO: 3,
		WARNING: 4,
		ERROR: 5
};
Log.FILTER_LEVEL = Log.Level.WARNING;

Log._write = function (message, level, label)
{
	if (level >= Log.FILTER_LEVEL)
	{
		console.log ("[Blocker]\t" + "[" + label + "]\t" + new Date() + "\t" + message);
	}
}
Log.v = function (message) { Log._write(message, Log.Level.VERBOSE, "v"); };
Log.d = function (message) { Log._write(message, Log.Level.DEBUG, "d"); };
Log.i = function (message) { Log._write(message, Log.Level.INFO, "i"); };
Log.w = function (message) { Log._write(message, Log.Level.WARNING, "w"); };
Log.e = function (message) { Log._write(message, Log.Level.ERROR, "e"); };