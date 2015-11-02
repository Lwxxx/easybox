<html>
  <head>
	<title>DDW</title>
	<link rel="stylesheet" type="text/css" href="css/command.css">
	<script type="text/javascript" src="js/command.js"></script>
  </head>
  <body onkeydown="on_enter_down()">
	<textarea id="command-output" readonly="readonly"></textarea>
	<br />
	<table>
	  <tr>
		<td>
		  <input id="command-text" type="text" />
		</td>
		<td>
		  <input id="command-button" type="button" onclick="exec_command()" value="run" />
		</td>
	  </tr>
	</table>
	<br />
	<a href="./index">[Back]</a>
  </body>
</html>
