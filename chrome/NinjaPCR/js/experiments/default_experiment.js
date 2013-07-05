var DEFAULT_EXPERIMENT = {
	"name": "Simple test",
	"steps": [
		{
			"type": "step",
			"name": "95C for 30 sec",
			"temp": "95",
			"time": "30",
			"rampDuration": "0"
		},
		{
			"type": "cycle",
			"count": "2",
			"steps": [
				{
					"type": "step",
					"name": "Denaturing",
					"temp": "95",
					"time": "30",
					"rampDuration": "0"
				},
				{
					"type": "step",
					"name": "Annealing",
					"temp": "55",
					"time": "30",
					"rampDuration": "0"
				},
				{
					"type": "step",
					"name": "Extending",
					"temp": "72",
					"time": "60",
					"rampDuration": "0"
				}
			]
		},
		{
			"type": "step",
			"name": "Final Hold",
			"time": 0,
			"temp": "20",
			"rampDuration": "0"
		}
	],
	"lidtemp": "110"
};