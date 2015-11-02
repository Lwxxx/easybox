/* press enter button */
function on_enter_down() {
	if(window.event.keyCode == 13) {
		dds_config();
	}
}

/* connect to dds server */
function dds_config() {
	var ip_text_entry = document.getElementById("dds-ip-text");
	var xhr = new XMLHttpRequest();

	function reload_index_page() {
		if ((xhr.readyState == 4) && (xhr.status == 200)) {
			location.reload();
		}
	};

	xhr.open("POST", "/config", true);
	xhr.onreadystatechange = reload_index_page;
	xhr.send(ip_text_entry.value);
}
