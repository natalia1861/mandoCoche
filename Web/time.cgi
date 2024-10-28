t <!DOCTYPE html>
t <html lang="es">
t <head>
t    <meta charset="UTF-8">
t    <meta name="viewport" content="width=device-width, initial-scale=1.0">
t    <link rel="stylesheet" href="styleTime.css">
t    <title>Hour</title>
t 	 <script language=JavaScript type="text/javascript" src="xml_http.js"></script>
t    <script language=JavaScript type="text/javascript">
# Define URL and refresh timeout
t var formUpdate = new periodicObj("time.cgx", 500);
t function plotTimeGraph() {
t  document.getElementById("time_value").value;
t  document.getElementById("date_value").value;
t }
t function periodicUpdateTime() {
t  if(document.getElementById("adChkBox").checked == true) {
t   updateMultiple(formUpdate,plotTimeGraph);
t   ad_elTime = setTimeout(periodicUpdateTime, formUpdate.period);
t  }
t  else
t   clearTimeout(ad_elTime);
t }
t </script></head>
t <form action=time.cgi method=post name=cgi>
t <body>
t    <div class="control-container">
t <table>
t 	<tr>
t                   <td>System Hour:</td>
c j 1               <td><input type="text" readonly id="time_value" value="%s"></td>
t 	</tr>
t               <tr>
t                   <td>System Date:</td>
c j 2               <td><input type="text" readonly id="date_value" value="%s"></td>
t               </tr>
t </table>
# Here begin button definitions
t <p align=center>
t Periodic:<input type="checkbox" id="adChkBox" onclick="periodicUpdateTime()">
t </p>
t    </div>
t </body>
t </html>
t </form>
. End of script must be closed with period.