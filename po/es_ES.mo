��          �   %   �      @  	   A  /   K  Y  {     �     �  o     P   {  0   �  f   �  <   d  9   �     �     �            4   +  q   `  c   �  j   6  .   �  8   �  S   	  %   ]     �  @  �     �	  C   �	  �  2
  -   �  "   �  �     g   �  H   �  s   C  B   �  @   �     ;     [     v     �  A   �  �   �  l   e  t   �  7   G  A     c   �  E   %     k                                                                              	                                
                Autopilot Autopilot Control display zoom factor           Autopilot Control for Raymarine Smartpilot
Simulates a Remote Control (Commandos from ST600x), by sending Seatalk(1) Data.
Requires Seatalk <-> NMEA Converter. The Sentences must be in the format
$SENTENCENAME,Seatalkbytes in Hex. ( $STALK,84,36,9D,88,40,00,FF,02,06*02 )
send to the NMEA Output Port. $SENTENCENAME named in the Preferences. Autopilot Parameters changeable Autopilot Preferences Change course value back to the last Compass course value, the Course Computer
had, before the Auto Signal lost Counter for Autopilot is in Standby Mode, but without "Standby" Command received Debug all "$STALK" Messages into OPENCPN Logfile Modify RMC Sentence as "$ECRMC" and replace or fill with Variationfield with the value from WMM Plugin NMEA Sentencename from  Seatalk bus converted by SeatalkLink NMEA Sentencename to Seatalk bus converted by SeatalkLink No Parameter selected No Value selected Raymarine Autopilot Raymarine autopilot Raymarine autopilot Control for Raymarine Smartpilot Send "Track" automatic when Autopilot has new Waypoint. Be careful with this normaly you have to accept yourself. Send new "Auto" Command, when No Standby-Information is received from other Instrument or from here Send new "Auto" or "Auto-Wind" Command, when "Standby" is not send from here, but the "Auto" was from here Send new "Auto-Wind" Command when "Wind Shift" Time in seconds to wait before sending "Track" sentence. Value, for maximum  "Not recived Standby Commands" not sending "Auto" Command again Write Message Info to OPENCPN Logfile is set to   Project-Id-Version: raymarine_autopilot_pi 1.0.2.0
Report-Msgid-Bugs-To: 
PO-Revision-Date: 2023-02-26 14:00+0100
Last-Translator: 
Language-Team: 
Language: es
MIME-Version: 1.0
Content-Type: text/plain; charset=UTF-8
Content-Transfer-Encoding: 8bit
Plural-Forms: nplurals=2; plural=(n != 1);
X-Generator: Poedit 3.2.2
 Control de piloto automático Factor de zoom de la pantalla de control del piloto automático     Control de piloto automático para Raymarine Smartpilot
Simula un Control Remoto (Comandos desde ST600x), mediante el envío de Datos Seatalk(1).
Requiere Seatalk <-> Convertidor NMEA. Las oraciones deben estar en el formato
$SENTENCENAME,Seatalkbytes en hexadecimal. ( $STALK,84,36,9D,88,40,00,FF,02,06*02 )
enviar al puerto de salida NMEA. $SENTENCENAME nombrado en las Preferencias. Parámetros del piloto automático cambiables Preferencias de piloto automático Cambie el valor del rumbo al último valor del rumbo de Compass, la computadora de rumbo
tenía, antes de que la señal automática perdiera El contador del piloto automático está en modo de espera, pero no se ha recibido el comando "Standby" Depurar todos los mensajes "$STALK" en el archivo de registro de OPENCPN Modifique la oración RMC como "$ ECRMC" y reemplace o rellene el campo Variación con el valor del complemento WMM NMEA Sentencename del bus Seatalk convertido por Seatalktranslater Bus NMEA Sentencename a Seatalk convertido por Seatalktranslator Ningún parámetro seleccionado Ningún valor seleccionado Raymarine Autopilot Raymarine Autopilot Control de piloto automático Raymarine para Raymarine Smartpilot Envía "Track" automáticamente cuando Autopilot tiene un nuevo Waypoint. Ojo con esto normalmente te tienes que aceptar a ti mismo. Envíe un nuevo comando "Auto", cuando no se reciba información de espera de otro instrumento o desde aquí Envíe un nuevo comando "Auto" o "Auto-Wind", cuando "Standby" no se envíe desde aquí, pero "Auto" fue desde aquí Envía un nuevo comando "Auto-Wind" cuando "Wind Shift" Iempo en segundos de espera antes de enviar la sentencia "Track". Valor, para un máximo de "Comandos de espera no recibidos" sin enviar el comando "Auto" nuevamente Escribir información de mensaje en el archivo de registro de OPENCPN se establece en    