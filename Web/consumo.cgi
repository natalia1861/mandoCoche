t <!DOCTYPE html>
t <html lang="es">
t <head>
t    <meta charset="UTF-8">
t    <meta name="viewport" content="width=device-width, initial-scale=1.0">
t    <link rel="stylesheet" href="styleConsumo.css">
t    <title>Bajo Consumo</title>
t    <script language=JavaScript type="text/javascript" src="xml_http.js"></script>
t </head>
t <form action=consumo.cgi method=post name=consumo>
t <body>
t    <div class="control-container">
t       <div class="label-container1">
t           <label for="operacion">BAJO CONSUMO</label>
t       </div>
t       <div class="label-container2">
t           <label for="activo">Activo:</label>
t        	<select name="ctrlConsumo" id="operaciones" onchange="submit();">
c b c            <option %s>Disable</option><option %s>Enable</option>
t        	</select>
t       </div>
t    </div>
t </body>
t </html>
.