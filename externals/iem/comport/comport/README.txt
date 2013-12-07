comport - PD external for unix/windows to use the serial ports

 (c) 1998-2005  Winfried Ritsch (see LICENCE.txt)
 Institute for Electronic Music - Graz

on Windows the COM0, COM1, ... are used and 
under Unix devices /dev/ttyS0, /dev/ttyS1, ...
and new on unix /dev/USB0, ... and can be accessed via a Index.

Please see testcomport.pd for more help.

USE: There should be a external comport.dll for windows, comport.pd_linux for linux and so on.

just copy it to the extra folder of your pd Installation or working directory. 
Please see testcomport.pd for more help.

compile:

 Unix (Linux):   
  make pd_linux,  make pd_irix5, make pd_irix6, make pd_darwin
  should produce a comport.pd_linux, ....
  

 Windows: use nmake or just use Fast Build under MSVC++
   nmake pd_nt 



If you have improvements or questions feel free to contact me under
ritsch _at_ iem.at
