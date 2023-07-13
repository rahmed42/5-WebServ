<!DOCTYPE html>
<html>

	<head>
		<meta http-equiv="content-type" content="text/html; charset=UTF-8">
		<title>CGI</title>
		<link rel="stylesheet" href="/loc_pages/index.css">
	</head>

	<body>
		<h1>CGI</h1>
		<p>
			<?php
				echo "Hello World!<br>";
				print_r($_GET);
				print_r($_POST);
			?>
		</p>
		<p style="text-align: center; vertical-align: middle; background-color: transparent;">
			<a href="/test">
				<span style="font-family: Helvetica,Arial,sans-serif;">
					<input name="GO TO INDEX" value="GO TO INDEX" type="button">
				</span>
			</a>
		</p>
	</body>
</html>
