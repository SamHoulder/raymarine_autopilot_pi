��          �   %   �      P     Q  /   c  Y  �     �       o   #  P   �  0   �  f     <   |  9   �     �     	          /  4   C  q   x  c   �  j   N  .   �     �  8   �  S   %  %   y     �  @  �     �	  2   
  p  6
  /   �     �  �   �  X   t  7   �  ]     F   c  G   �     �          '     ;  :   O  q   �  x   �  ~   u  :   �     /  =   3  d   q  +   �                                                                                  	                                
                Autopilot Control Autopilot Control display zoom factor           Autopilot Control for Raymarine Smartpilot
Simulates a Remote Control (Commandos from ST600x), by sending Seatalk(1) Data.
Requires Seatalk <-> NMEA Converter. The Sentences must be in the format
$SENTENCENAME,Seatalkbytes in Hex. ( $STALK,84,36,9D,88,40,00,FF,02,06*02 )
send to the NMEA Output Port. $SENTENCENAME named in the Preferences. Autopilot Parameters changeable Autopilot Preferences Change course value back to the last Compass course value, the Course Computer
had, before the Auto Signal lost Counter for Autopilot is in Standby Mode, but without "Standby" Command received Debug all "$STALK" Messages into OPENCPN Logfile Modify RMC Sentence as "$ECRMC" and replace or fill with Variationfield with the value from WMM Plugin NMEA Sentencename from  Seatalk bus converted by SeatalkLink NMEA Sentencename to Seatalk bus converted by SeatalkLink No Parameter selected No Value selected Raymarine Autopilot Raymarine autopilot Raymarine autopilot Control for Raymarine Smartpilot Send "Track" automatic when Autopilot has new Waypoint. Be careful with this normaly you have to accept yourself. Send new "Auto" Command, when No Standby-Information is received from other Instrument or from here Send new "Auto" or "Auto-Wind" Command, when "Standby" is not send from here, but the "Auto" was from here Send new "Auto-Wind" Command when "Wind Shift" Set Time in seconds to wait before sending "Track" sentence. Value, for maximum  "Not recived Standby Commands" not sending "Auto" Command again Write Message Info to OPENCPN Logfile is set to   Project-Id-Version: raymarine_autopilot_pi 1.0.2.0
Report-Msgid-Bugs-To: 
PO-Revision-Date: 2023-02-26 14:15+0100
Last-Translator: 
Language-Team: 
Language: nl
MIME-Version: 1.0
Content-Type: text/plain; charset=UTF-8
Content-Transfer-Encoding: 8bit
Plural-Forms: nplurals=2; plural=(n != 1);
X-Generator: Poedit 3.2.2
 Stuurautomaat controle Stuurautomaatbediening geeft zoomfactor weer       Stuurautomaatbediening voor Raymarine Smartpilot
Simuleert een afstandsbediening (commando's van ST600x), door Seatalk(1)-gegevens te verzenden.
Vereist Seatalk <-> NMEA-converter. De zinnen moeten het formaat hebben
$SENTENCENAME,Seatalkbytes in Hex. ($STALK,84,36,9D,88,40,00,FF,02,06*02 )
verzenden naar de NMEA-uitvoerpoort. $SENTENCENAME genoemd in de Voorkeuren. Stuurautomaatparameters kunnen worden gewijzigd Stuurautomaat voorkeuren Wijzig de koerswaarde terug naar de laatste kompaskoerswaarde, de koerscomputer
had, voordat het automatische signaal verloren ging Teller voor stuurautomaat staat in standby-modus, maar geen "Standby"-commando ontvangen Debug alle "$STALK"-berichten in het OPENCPN-logbestand Wijzig RMC-zin als "$ECRMC" en vervang of vul Variationfield met de waarde van de WMM-plug-in NMEA Sentencename van Seatalk bus geconverteerd door Seatalktranslator NMEA Sentencename naar Seatalk bus geconverteerd door Seatalktranslator Geen parameter geselecteerd Geen waarde geselecteerd Raymarine Autopilot Raymarine Autopilot Raymarine stuurautomaatbediening voor Raymarine Smartpilot Send "Track" automatic when Autopilot has new Waypoint. Be careful with this normaly you have to accept yourself. Stuur een nieuw "Auto"-commando wanneer er geen stand-by-informatie wordt ontvangen van een ander instrument of van hier Stuur een nieuw "Auto"- of "Auto-Wind"-commando, wanneer "Standby" niet van hier wordt verzonden, maar de "Auto" van hier kwam Stuur een nieuw "Auto-Wind" -commando wanneer "Wind Shift" Set Wachttijd in seconden voordat de "Track"-zin wordt verzonden. Waarde, voor maximaal "Niet ontvangen standby-commando's" niet opnieuw verzenden van "Auto"-commando Schrijf berichtinfo naar OPENCPN-logbestand ingesteld op   