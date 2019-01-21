<?php

$testdata 	= html_entity_decode($_POST["xml"]);
$port		= "8012";

$sock 		= socket_create(AF_INET6, SOCK_DGRAM, 0) or die("Failed to create socket");
socket_set_option($sock,SOL_SOCKET,SO_RCVTIMEO,array("sec"=>1,"usec"=>0));
socket_connect($sock, "localhost", $port) or die("Failed to connect to socket");
socket_write($sock, $testdata);

$output		= socket_read($sock, 2**16);
echo		htmlspecialchars("<xml>".$output."</xml>".PHP_EOL);
#echo socket_strerror(socket_last_error($sock));
socket_close($sock);

?>
