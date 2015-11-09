function monitorLoad() {
	var cpuChart = new SmoothieChart({
		grid: {	lineWidth: 1,
				millisPerLine: 400,
				verticalSections: 5 },
		minValue: 0,
		maxValue: 100 });
	var lavgChart = new SmoothieChart({
		grid: {	lineWidth: 1,
				millisPerLine: 400,
				verticalSections: 5 },
		minValue: 0,
		maxValue: 20 });
	var memChart = new SmoothieChart({
		grid: {	lineWidth: 1,
				millisPerLine: 400,
				verticalSections: 5 },
		minValue: 0,
		maxValue: 100 });
	var cpuInfo   = new TimeSeries();
	var lavg1min  = new TimeSeries();
	var lavg5min  = new TimeSeries();
	var lavg15min = new TimeSeries();
	var memInfo   = new TimeSeries();
	var xhr       = new XMLHttpRequest();

	/* cpu usage chart */
	cpuChart.streamTo(document.getElementById("cpu-usage"), 1000);
	cpuChart.addTimeSeries(cpuInfo, {
		strokeStyle: 'rgba(255, 0, 0, 1)',
		fillStyle: 'rgba(255, 0, 0, 0.2)',
		lineWidth: 4
	});

	/* cpu load average chart */
	lavgChart.streamTo(document.getElementById("cpu-lavg"), 1000);
	lavgChart.addTimeSeries(lavg1min, {
		strokeStyle: 'rgba(0, 0, 255, 1)',
		fillStyle: 'rgba(0, 0, 255, 0.2)',
		lineWidth: 4
	});
	lavgChart.addTimeSeries(lavg5min, {
		strokeStyle: 'rgba(255, 255, 0, 1)',
		fillStyle: 'rgba(255, 255, 0, 0.2)',
		lineWidth: 4
	});
	lavgChart.addTimeSeries(lavg15min, {
		strokeStyle: 'rgba(255, 0, 0, 1)',
		fillStyle: 'rgba(255, 0, 0, 0.2)',
		lineWidth: 4
	});

	/* memory usage chart */
	memChart.streamTo(document.getElementById("mem-usage"), 1000);
	memChart.addTimeSeries(memInfo, {
		strokeStyle: 'rgba(0, 255, 0, 1)',
		fillStyle: 'rgba(0, 255, 0, 0.2)',
		lineWidth: 4
	});

	function addInfoToChart() {
		if ((xhr.readyState == 4) && (xhr.status == 200)) {
			var responseJson = JSON.parse(xhr.responseText);
			var currTime     = new Date().getTime();

			cpuInfo.append(currTime, responseJson.cpu);
			memInfo.append(currTime, responseJson.mem);
			lavg1min.append(currTime, responseJson.lavg_1min);
			lavg5min.append(currTime, responseJson.lavg_5min);
			lavg15min.append(currTime, responseJson.lavg_15min);
		}
	};

	/* set a timer to get monitor data */
	setInterval(function() {
		xhr.onreadystatechange = addInfoToChart;
		xhr.open("POST", "/monitor/data", true);
		xhr.send("");
	}, 1000);
}

function monitorToggle() {
	var toggleButton = document.getElementById("toggle-button");
	var xhr = new XMLHttpRequest();

	if ("connect" === toggleButton.value) {
		xhr.open("POST", "/monitor/start", true);
		xhr.send("")
		toggleButton.value = "stop";
	} else if ("stop" === toggleButton.value) {
		xhr.open("POST", "/monitor/stop", true);
		xhr.send("")
		toggleButton.value = "connect";
	}
}
