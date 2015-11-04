/* connect to dds server */
function ddsConnect() {
	var ipTextEntry = document.getElementById("dds-ip-text");
	var connectXhr  = new XMLHttpRequest();
	var startXhr    = new XMLHttpRequest();

	function reloadIndexPage() {
		if ((connectXhr.readyState == 4) && (connectXhr.status == 200)) {
			location.reload();
		}
	};

	connectXhr.open("POST", "/dds", true);
	connectXhr.onreadystatechange = reloadIndexPage;
	connectXhr.send(ipTextEntry.value);

	startXhr.open("POST", "/monitor/start", true);
	startXhr.send("")
}
