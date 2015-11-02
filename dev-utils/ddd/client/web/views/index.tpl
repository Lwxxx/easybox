<html>
  <head>
	<title>DDW</title>
	<script type="text/javascript" src="js/config.js"></script>
  </head>
  <body onkeydown="on_enter_down()">
	<h1>DDW</h1>
	<hr/>
	<a href="./command">command</a> <br/>
	<a href="./monitor">monitor</a> <br/>
	<a href="./terminal">terminal</a> <br/>
	<a href="./log">log</a> <br/>
	<a href="./setting">settinge</a> <br/>
	<a href="./about">about</a> <br/>
	<hr/>
	% if ipaddr == '':
	<table>
	  <tr>
		<td>
		  <p>DDS IP Address: </p>
		</td>
		<td>
		  <input id="dds-ip-text" type="text" />
		</td>
		<td>
		  <input id="connect-button" type="button" value="set" onclick="dds_config()" />
		</td>
	  </tr>
	</table>
	% else:
	<p>dds ip address is: {{ipaddr}}</p>
	% end
  </body>
</html>
