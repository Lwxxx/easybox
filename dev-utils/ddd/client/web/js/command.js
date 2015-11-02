/* press enter button */
function on_enter_down() {
	if(window.event.keyCode == 13) {
		exec_command();
	}
}

/* execute a command */
function exec_command() {
	var command_text_entry = document.getElementById("command-text");
	var command_output_entry = document.getElementById("command-output");
	var xhr = new XMLHttpRequest();

	function show_command_output() {
		if ((xhr.readyState == 4) && (xhr.status == 200)) {
			command_output_entry.value += xhr.responseText;
			command_output_entry.value += "\n\n";
			command_output_entry.scrollTop = command_output_entry.scrollHeight;
		}
	};

	/* send execute command to backend*/
	xhr.open("POST", "/command/execute", true);
	xhr.onreadystatechange = show_command_output;
	xhr.send(command_text_entry.value);

	/* clear command text */
	command_text_entry.value="";
}
