��          �   %   �      p  	   q     {  /   �  Y  �          7  o   M  P   �  0     f   ?  <   �  9   �          3     E     Y  4   m  q   �  c     j   x  .   �       8        O  S   U  %   �     �  D  �      
     4
  H   G
  �  �
  -     #   E  �   i  d     F   t  p   �  9   ,  >   f     �     �     �     �  @   	  �   J  u   �     Q  S   �     %  @   ,     m  i   s  C   �     !                                                                            
                    	                           Autopilot Autopilot Control Autopilot Control display zoom factor           Autopilot Control for Raymarine Smartpilot
Simulates a Remote Control (Commandos from ST600x), by sending Seatalk(1) Data.
Requires Seatalk <-> NMEA Converter. The Sentences must be in the format
$SENTENCENAME,Seatalkbytes in Hex. ( $STALK,84,36,9D,88,40,00,FF,02,06*02 )
send to the NMEA Output Port. $SENTENCENAME named in the Preferences. Autopilot Parameters changeable Autopilot Preferences Change course value back to the last Compass course value, the Course Computer
had, before the Auto Signal lost Counter for Autopilot is in Standby Mode, but without "Standby" Command received Debug all "$STALK" Messages into OPENCPN Logfile Modify RMC Sentence as "$ECRMC" and replace or fill with Variationfield with the value from WMM Plugin NMEA Sentencename from  Seatalk bus converted by SeatalkLink NMEA Sentencename to Seatalk bus converted by SeatalkLink No Parameter selected No Value selected Raymarine Autopilot Raymarine autopilot Raymarine autopilot Control for Raymarine Smartpilot Send "Track" automatic when Autopilot has new Waypoint. Be careful with this normaly you have to accept yourself. Send new "Auto" Command, when No Standby-Information is received from other Instrument or from here Send new "Auto" or "Auto-Wind" Command, when "Standby" is not send from here, but the "Auto" was from here Send new "Auto-Wind" Command when "Wind Shift" Set Time in seconds to wait before sending "Track" sentence. Track Value, for maximum  "Not recived Standby Commands" not sending "Auto" Command again Write Message Info to OPENCPN Logfile is set to   Project-Id-Version: raymarine_autopilot_pi 1.0.000.0
Report-Msgid-Bugs-To: 
PO-Revision-Date: 2023-02-26 14:00+0100
Last-Translator: 
Language-Team: 
Language: fr_FR
MIME-Version: 1.0
Content-Type: text/plain; charset=UTF-8
Content-Transfer-Encoding: 8bit
Plural-Forms: nplurals=2; plural=(n > 1);
X-Generator: Poedit 3.2.2
 Raymarine Autopilot Pilote automatique Facteur de zoom de l'écran de contrôle du pilote automatique           Commande de pilote automatique pour Smartpilot Raymarine
Simule une commande à distance (Commandos du ST600x), en envoyant des données Seatalk(1).
Nécessite le convertisseur NMEA de Seatalk <->. Les phrases doivent être au format
$SENTENCENAME,Seatalkbytes en Hex. ( $STALK,84,36,9D,88,40,00,FF,02,06*02 )
envoyer au port de sortie NMEA. $SENTENCENAME nommé dans les Préférences. Paramètres du pilote automatique modifiables Préférences du pilote automatique Changer la valeur de la trajectoire pour revenir à la dernière valeur de la trajectoire du compas, que le calculateur de route
avant la perte du signal automatique Le compteur du pilote automatique est en mode veille, mais sans que la commande "veille" soit reçue Déboguer tous les messages "$STALK" dans le fichier journal d'OPENCPN Modifiez la phrase RMC comme "$ECRMC" et remplacez ou remplissez le champ Variation avec la valeur du plugin WMM NMEA Sentencename du bus Seatalk converti par SeatalkLink Conversion de NMEA Sentencename en bus Seatalk par SeatalkLink Aucun paramètre sélectionné Aucune valeur sélectionnée Raymarine Autopilot Raymarine Autopilot Pilote automatique Raymarine Contrôle pour Smartpilot Raymarine Envoyer un "Track" automatique quand le pilote automatique a un nouveau waypoint. Soyez prudent avec ceci, vous devez normalement vous accepter. Envoyer une nouvelle commande "Auto", lorsqu'aucune information de veille n'est reçue d'un autre instrument ou d'ici Envoyer une nouvelle commande "Auto" ou "Auto-Wind", lorsque "Standby" n'est pas envoyé d'ici, mais que "Auto" l'a été d'ici Envoyer une nouvelle commande "Auto-Wind" lorsque "Wind Shift" (changement de vent) Mettre Temps en secondes à attendre avant d'envoyer la phrase "Track". Track Valeur, pour le maximum de "Commandes de veille non reçues" ne pas envoyer à nouveau la commande "Auto" Écrire les informations du message dans le fichier journal OPENCPN est réglé sur   