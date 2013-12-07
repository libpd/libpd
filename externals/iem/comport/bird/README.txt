bird - PD external for unix/windows to use the flock of birds

 (c) 1998-2005  Winfried Ritsch (see LICENCE.txt)
 Institute for Electronic Music - Graz

the external comport is also needed for interfacing.

Please see testbird.pd for more help.

USE: There should be a external bird.dll for windows, bird.pd_linux for linux and so on.

just copy it to the extra folder of your pd Installation or working directory. 

compile:

 Unix (Linux):   
  make pd_linux,  make pd_irix5, make pd_irix6
  should produce a bird.pd_linux, ....
  

 Windows: use nmake or just use Fast Build under MSVC++
   nmake pd_nt 


If you have improvements or questions feel free to contact me under
ritsch _at_ iem.at
