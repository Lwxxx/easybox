<html>
  <head>
	<title>DDW</title>
	<script type="text/javascript" src="js/smoothie.js"></script>
	<script type="text/javascript" src="js/dds.js"></script>
	<script type="text/javascript" src="js/monitor.js"></script>
  </head>
  <body onload="monitorLoad()">
	<h1>DDW</h1>
	<hr/>
	<br/>
	% if ipaddr == '':
	<p>DDS IP Address: </p>
	<input id="dds-ip-text" type="text" />
	<input id="dds-connect" type="button" value="connect" onclick="ddsConnect()" />
	% else:
	<p>dds ip address is: {{ipaddr}}</p>
	<input id="toggle-button" type="button" value="stop" onclick="monitorToggle()" />
	% end
	<br/>
	<br/>
	<table cellspacing="20">
	  <tr>
		<td>CPU Usage:</td>
		<td>CPU Load Average:</td>
		<td>MEM Usage:</td>
	  </tr>
	  <tr>
		<td>
		  <canvas id="cpu-usage" width="400" height="200"></canvas>
		</td>
		<td>
		  <canvas id="cpu-lavg" width="400" height="200"></canvas>
		</td>
		<td>
		  <canvas id="mem-usage" width="400" height="200"></canvas>
		</td>
	  </tr>
	</table>
  </body>
</html>
