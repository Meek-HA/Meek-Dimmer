# Meek-Dimmer 
Meek MD1, Atmega328 I2C Zero Cross and Dimming for Meek MD1 ( http://www.meek-ha.com/ ).


Toggle On Off command from your Home Automation System
http://192.168.2.39/control?cmd=event,on  = Domoticz Dimmer Slider "On Action:"
http://192.168.2.39/control?cmd=event,off = Domoticz Dimmer Slider "Off Action:"
http://192.168.2.39/control?cmd=event,toggle = Toggle dimmer on/off

Set Domoticz Dimmer Slider to a certain value:
http://192.168.2.152:8181/json.htm?type=command&param=switchlight&idx=2&switchcmd=Set%20Level&level=10

Send Feedback from Meek MD1 to Domoticz:
SendToHTTP 192.168.2.152,8181,/json.htm?type=command&param=switchlight&idx=2&switchcmd=Set%20Level&level=[PME#Value]

Send a specifik PWM/Dim value (Between UpperLimit & LowerLimit):
http://192.168.2.39/control?cmd=EXTPWM,5,350

Set limiter for your lighting system;
UpperLimit range 4000-4999 ,  Default UpperLimit=990:
http://192.168.2.39/control?cmd=EXTPWM,5,4990

LowerLimit range 3000-3999 ,  Default LowerLimit=0:
http://192.168.2.39/control?cmd=EXTPWM,5,3000

Set delay for each dim level transition, range 5000,5999 ,  Default=10 :
http://192.168.2.39/control?cmd=EXTPWM,5,5010
