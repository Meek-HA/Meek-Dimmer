commandArray = {}
DomDevice = 'Meek MD1';
IP = '192.168.2.39';
PIN = "5";
if devicechanged[DomDevice] then
   if(devicechanged[DomDevice]=='Off') then
     print ("OFF dimm = "..uservariables['dimm']);
     CalcValue = 6001;
   else if(devicechanged[DomDevice]=='On') then
        DomValue = uservariables['dimm'];
        print ("ON dimm = "..uservariables['dimm']);
        CalcValue = DomValue;
      else
         print("Other");
         DomValue = otherdevices_svalues[DomDevice];
         CalcValue = (DomValue+8000);
         commandArray['Variable:dimm'] = tostring(CalcValue);
         print ("dimm Level = "..uservariables['dimm']);
   end
   end
   runcommand = "curl http://" .. IP .. "/control?cmd=EXTPWM,"  ..PIN.. "," .. CalcValue .. " ";
   os.execute(runcommand);
print("PWM calculated value= "..CalcValue);
end
return commandArray
