@echo off

set PD_PATCH=%1
set PD_INSTALL=C:\Programme\pd-0.39-2\bin
set PD_AUDIO=-r 44100 -audiobuf 80 -channels 2
set PD_MIDI=-nomidi
set PD_OPTIONS=-font 10
set PD_PATH=-path iemabs
set PD_LIB=-lib iemlib1;iemlib2;iem_mp3;iem_t3_lib

@echo starting pd ...
%PD_INSTALL%\pd %PD_AUDIO% %PD_MIDI% %PD_OPTIONS% %PD_PATH% %PD_LIB% %PD_PATCH%
