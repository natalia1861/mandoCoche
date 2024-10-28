t <!DOCTYPE html>
t <html lang="es">
t <head>
t    <meta charset="UTF-8">
t    <meta name="viewport" content="width=device-width, initial-scale=1.0">
t    <link rel="stylesheet" href="styleDel.css">
t    <title>Delete Users</title>
t    <script language=JavaScript type="text/javascript" src="xml_http.js"></script>
t </head>
t <form action=usuariosDel.cgi method=post name=usuariosAdd>
t <body>
t        <div id="delete-usuarios" class="formulario">
t            <table>
t              <tr>
c l 1            <td><input type="checkbox" name=user1Del %s>User1</td>
t              </tr>
t              <tr>
c c 2            <td><input type="checkbox" name=user2Del %s>User2</td>
t              </tr>
t              <tr>
c a 3             <td><input type="checkbox" name=user3Del %s>User3</td>
t              </tr>
t              <tr>
c d 4             <td><input type="checkbox" name=user4Del %s>User4</td>
t              </tr>
t              <tr>
c e 5            <td><input type="checkbox" name=user5Del %s>User5</td>
t              </tr>
t            </table>
t            <input type=submit name="set2" value="Save" id="del">
t </body>
t </html>
.