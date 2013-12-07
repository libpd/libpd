@echo off

set PD_PATCH=%1
set PD_INSTALL=C:\Programme\pd\bin
set PD_AUDIO=-r 44100 -audiobuf 80 -channels 2
set PD_MIDI=-nomidi
set PD_OPTIONS=-font 10
set PD_PATH=-path C:/Programme/pd/externs/iemlib_R1.16/iemabs -path C:/Programme/pd/externs/iemlib_R1.16/lib -path C:/Programme/pd/externs/zexy
set PD_LIB=-lib iemlib1 -lib iemlib2 -lib iemgui -lib zexy


@echo starting pd ...
%PD_INSTALL%\pd %PD_AUDIO% %PD_MIDI% %PD_OPTIONS% %PD_PATH% %PD_LIB% %PD_PATCH%
