# Meek-Dimmer
Meek MD1, Atmega328 I2C Zero Cross and Dimming

Toggle On Off command from your Home Automation System
http://192.168.2.39/control?cmd=event,toggle

Set Domoticz Dimmer Slider to a certain value:
http://192.168.2.152:8181/json.htm?type=command&param=switchlight&idx=2&switchcmd=Set%20Level&level=10

Send Feedback from Meek MD1 to Domoticz:
SendToHTTP 192.168.2.152,8181,/json.htm?type=command&param=switchlight&idx=2&switchcmd=Set%20Level&level=[PME#Value]
