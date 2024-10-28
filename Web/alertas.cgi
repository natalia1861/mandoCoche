t <!DOCTYPE html>
t <html lang="es">
t <head>
t    <meta charset="UTF-8">
t    <meta name="viewport" content="width=device-width, initial-scale=1.0">
t    <link rel="stylesheet" href="styleAlertas.css">
t    <title>Control</title>
t    <script language=JavaScript type="text/javascript" src="xml_http.js"></script>
t </script></head>
t <form action=alertas.cgi method=post name=alertas>
t <body>
t    <div class="control-container">
t           <table>
t               <tr>
t                   <td></td>
t                   <td class="state">State</td>
t               </tr>
t               <tr>
t                   <td>Gas Sensor:</td>
c v 1               <td><input type="text" readonly id="gas_value" value="%s"></td>
t               </tr>
t               <tr>
t                   <td>Fire Sensor:</td>
c v 2               <td><input type="text" readonly id="fuego_value" value="%s"></td>
t               </tr>
t               <tr>
t                   <td>Piezo Sensor:</td>
c v 3               <td><input type="text" readonly id="piezo_value" value="%s"></td>
t               </tr>
t               </tr>
c g 1       		<td><input type="password" name="pwdAlert" id="contraseña" placeholder="Password"></td>
t               </tr>
t           </table>
t           <input type=submit name=set value="Deactivate" id="alert">
t    </div>
t </body>
t </html>
.