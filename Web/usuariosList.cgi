t <!DOCTYPE html>
t <html lang="es">
t <head>
t    <meta charset="UTF-8">
t    <meta name="viewport" content="width=device-width, initial-scale=1.0">
t    <link rel="stylesheet" href="styleList.css">
t    <title>List Users</title>
t    <script language=JavaScript type="text/javascript" src="xml_http.js"></script>
t </head>
t <form action=usuariosList.cgi method=post name=usuariosList>
t <body>
t        <div id="lista-usuarios" class="formulario">
t            <table>
t              <tr>
t                <td>User 1:</td>
c h 1            <td><input type="text" readonly id="user1_value" value="%s" ></td>
t              </tr>
t              <tr>
t                <td>User 2:</td>
c h 2            <td><input type="text" readonly id="user2_value" value="%s"></td>
t              </tr>
t              <tr>
t                <td>User 3:</td>
c h 3            <td><input type="text" readonly id="user3_value" value="%s"></td>
t              </tr>
t              <tr>
t                <td>User 4:</td>
c h 4            <td><input type="text" readonly id="user4_value" value="%s"></td>
t              </tr>
t              <tr>
t                <td>User 5:</td>
c h 5           <td><input type="text" readonly id="user5_value" value="%s"></td>
t              </tr>
t            </table>
t  			 <input type=button value="Refresh" onclick="location='/usuariosList.cgi'">
t        </div>
t </body>
t </html>
.