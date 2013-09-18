Installation-guide for iemlib under windows (win_xp, win2k, win_me, win_nt or win98)

iemlib_R1.17 is written by Thomas Musil from IEM Graz Austria
 and it is compatible to miller puckette's pd-0.38-3 to pd-0.39-2.
see also LICENCE.txt, GnuLGPL.txt and README.txt.

1.) download and copy iemlib_R1.17_win.zip into your pd-installation-directory
 ( e.g.C:\Programs\pd\iemlib_R1.17_win.zip )


2.) unzip iemlib_R1.17_win.zip
 winzip create a directory    C:\Programs\pd\iemlib_R1.17\

3.) install external library folders
 copy from C:\Programs\pd\iemlib_R1.17\iemlib1 to C:\Programs\pd\extra
 copy from C:\Programs\pd\iemlib_R1.17\iemlib2 to C:\Programs\pd\extra
 copy from C:\Programs\pd\iemlib_R1.17\iem_mp3 to C:\Programs\pd\extra
 copy from C:\Programs\pd\iemlib_R1.17\iem_t3_lib to C:\Programs\pd\extra

4.) install pd abstraction folder
 copy from C:\Programs\pd\iemlib_R1.17\iemabs to C:\Programs\pd\extra

5.) add to your pd_start.bat the following configuration-options:

 -path C:\Programs\pd\extra\iemabs  -lib iemlib1;iemlib2;iem_mp3;iem_t3_lib

or create a textfile pd_start.bat with this content:

@echo off
set PD_INSTALL=C:\Programme\pd\bin
set PD_AUDIO=-r 44100 -audiobuf 160
set PD_MIDI=-nomidi
set PD_PATH=-path C:/Programme/pd/extra/iemabs
set PD_LIB=-lib iemlib1;iemlib2;iem_mp3;iem_t3_lib
@echo starting pd ...
%PD_INSTALL%\pd %PD_AUDIO% %PD_MIDI% %PD_PATH% %PD_LIB%




How to compile iemlib under windows

after installing iemlib and if You are owner of a VCC++ compiler:

4.a) edit makefile_win of iemlib1
 change to directory C:\Programs\pd\iemlib_R1.17\iemlib1\src
 and edit following lines of makefile_win and save
 ( VIS_CPP_PATH = ""  and  PD_INST_PATH = "" )

4.b) compile iemlib1
 doubleclick iemlib1.dsw, VCC++ will start,
 Menue: build/configuration = Release,
 Menue: build/build all ,
 ( after this command, iemlib1.dll will be created in C:\Programs\pd\iemlib_R1.17\iemlib1 ).

5.a) edit makefile_win of iemlib2
 change to directory C:\Programs\pd\iemlib_R1.17\iemlib2\src
 and edit following lines of makefile_win and save
 ( VIS_CPP_PATH = ""  and  PD_INST_PATH = "" )

5.b) compile iemlib2
 doubleclick iemlib2.dsw, VCC++ will start,
 Menue: build/configuration = Release,
 Menue: build/build all ,
 ( after this command, iemlib2.dll will be created in C:\Programs\pd\iemlib_R1.17\iemlib2 ).

6.a) edit makefile_win of iem_mp3
 change to directory C:\Programs\pd\iemlib_R1.17\iem_mp3\src
 and edit following lines of makefile_win and save
 ( VIS_CPP_PATH = ""  and  PD_INST_PATH = "" )

6.b) compile iem_mp3
 doubleclick iem_mp3.dsw, VCC++ will start,
 Menue: build/configuration = Release,
 Menue: build/build all ,
 ( after this command, iem_mp3.dll will be created in C:\Programs\pd\iemlib_R1.17\iem_mp3 ).

7.a) edit makefile_win of iem_t3_lib
 change to directory C:\Programs\pd\iemlib_R1.17\iem_t3_lib\src
 and edit following lines of makefile_win and save
 ( VIS_CPP_PATH = ""  and  PD_INST_PATH = "" )

7.b) compile iem_t3_lib
 doubleclick iem_t3_lib.dsw, VCC++ will start,
 Menue: build/configuration = Release,
 Menue: build/build all ,
 ( after this command, iem_t3_lib.dll will be created in C:\Programs\pd\iemlib_R1.17\iem_t3_lib ).

8.) install dynamic libraries
 copy from C:\Programs\pd\iemlib_R1.17\iemlib1\iemlib1.dll to C:\Programs\pd\extra\iemlib1\iemlib1.dll
 copy from C:\Programs\pd\iemlib_R1.17\iemlib2\iemlib2.dll to C:\Programs\pd\extra\iemlib2\iemlib2.dll
 copy from C:\Programs\pd\iemlib_R1.17\iem_mp3\iem_mp3.dll to C:\Programs\pd\extra\iem_mp3\iem_mp3.dll
 copy from C:\Programs\pd\iemlib_R1.17\iem_t3_lib\iem_t3_lib.dll to C:\Programs\pd\extra\iem_t3_lib\iem_t3_lib.dll





content of iemlib Release 1.17 from November 2006

============================ DSP~ ===============================

------------------------- filter~ -------------------------------
FIR~		 finite impuls response filter, with array-coefficients

maverage~	 moving average filter, (IIR + delay)

ap1~		 allpass 1.order
ap2~		 allpass 2.order
bpq2~		 bandpass 2.order with Q-inlet
bpw2~		 bandpass 2.order with bandwidth-inlet
bsq2~		 bandstop 2.order (notch) with Q-inlet
bsw2~		 bandstop 2.order (notch) with bandwidth-inlet
hp1~		 highpass 1.order
hp2~		 highpass 2.order
lp1~		 lowpass 1.order
lp2~		 lowpass 2.order
rbpq2~		 resonance-bandpass 2.order with Q-inlet
rbpw2~		 resonance-bandpass 2.order with bandwidth-inlet
hml_shelf~	 high-middle-low shelving-filter with freq- and gain-inlets
lp1_t~		 lowpass 1.order with time_constant inlet
para_bp2~	 parametrical bandpass 2. order with freq-, Q- and gain-inlet
hp2_butt~, hp3_butt~, hp4_butt~, hp5_butt~, hp6_butt~, hp7_butt~,
hp8_butt~, hp9_butt~, hp10_butt~
	highpass 2.3.4.5.6.7.8.9.10.order with butterworth characteristic
hp2_cheb~, hp3_cheb~, hp4_cheb~, hp5_cheb~, hp6_cheb~, hp7_cheb~,
hp8_cheb~, hp9_cheb~, hp10_cheb~
	highpass 2.3.4.5.6.7.8.9.10.order with chebyshev characteristic
hp2_bess~, hp3_bess~, hp4_bess~, hp5_bess~, hp6_bess~, hp7_bess~,
hp8_bess~, hp9_bess~, hp10_bess~
	highpass 2.3.4.5.6.7.8.9.10.order with bessel characteristic
hp2_crit~, hp3_crit~, hp4_crit~, hp5_crit~, hp6_crit~, hp7_crit~,
hp8_crit~, hp9_crit~, hp10_crit~
	highpass 2.3.4.5.6.7.8.9.10.order with critical damping
lp2_butt~, lp3_butt~, lp4_butt~, lp5_butt~, lp6_butt~, lp7_butt~,
lp8_butt~, lp9_butt~, lp10_butt~
	lowpass 2.3.4.5.6.7.8.9.10.order with butterworth characteristic
lp2_cheb~, lp3_cheb~, lp4_cheb~, lp5_cheb~, lp6_cheb~, lp7_cheb~,
lp8_cheb~, lp9_cheb~, lp10_cheb~
	lowpass 2.3.4.5.6.7.8.9.10.order with chebyshev characteristic
lp2_bess~, lp3_bess~, lp4_bess~, lp5_bess~, lp6_bess~, lp7_bess~,
lp8_bess~, lp9_bess~, lp10_bess~
	lowpass 2.3.4.5.6.7.8.9.10.order with bessel characteristic
lp2_crit~, lp3_crit~, lp4_crit~, lp5_crit~, lp6_crit~, lp7_crit~,
lp8_crit~, lp9_crit~, lp10_crit~
	lowpass 2.3.4.5.6.7.8.9.10.order with critical damping
vcf_hp2~, vcf_hp4~, vcf_hp6~, vcf_hp8~
	highpass 2.4.6.8.order with freq- and Q-signal-inlets
vcf_lp2~, vcf_lp4~, vcf_lp6~, vcf_lp8~
	lowpass 2.4.6.8.order with freq- and Q-signal-inlets
vcf_bp2~, vcf_bp4~, vcf_bp6~, vcf_bp8~
	bandpass 2.4.6.8.order with freq- and Q-signal-inlets
vcf_rbp2~, vcf_rbp4~, vcf_rbp6~, vcf_rbp8~
	resonance-bandpass 2.4.6.8.order with freq- and Q-signal-inlets

------------------------ arithmetic~ ----------------------------
addl~		 signal-addition with line~
divl~		 signal-divison with line~
mull~		 signal-multiplication with line~
subl~		 signal-subtraction with line~

------------------------- converter~ ----------------------------
prvu~		 peak and rms VU-meter interface
pvu~		 peak VU-meter interface
rvu~		 rms VU-meter interface
unsig~		 signal to float converter

------------------ t3~ - time-tagged-trigger --------------------
-- inputmessages allow a sample-accurate access to signalshape --
t3_sig~		 time tagged trigger sig~
t3_line~	 time tagged trigger line~

--------------------------- misc~ -------------------------------
fade~		 fade-in fade-out shaper (need line~)
iem_blocksize~	 blocksize of a window in samples
iem_samplerate~	 samplerate of a window in Hertz
int_fract~	 split signal-float to integer- and fractal-part
LFO_noise~	 downsampled 2-point interpolated white noise
mp3play~	 mp3 stereo player
peakenv~	 peak envelope shaper
pink~		 pink noise
round~		 round signal-float to nearest integer
sin_phase~	 output phase-difference of 2 sinewaves in samples

========================= control ==============================

------------- gui (included into millers pd) --------------------

bng		 bang, display and generate a bang-message
cnv		 canvas, colored background and text
hdl		 horizontal dial, for multiplex usage
hradio		 horizontal radiobutton, only float in/out
hsl		 horizontal slider
nbx		 numberbox, the second
tgl		 2 state toggle
vdl		 vertical dial, for multiplex usage
vradio		 vertical radiobutton, only float in/out
vsl		 vertical slider
vu		 vu-meter, display rms- + peak-level in dB

--------------------- float operating -------------------------
1p1z		 float-message-filter 1.order
db2v		 db to rms
dbtofad		 midi-db to fader-characteristic
fadtodb		 fader-characteristic to midi-db
fadtorms	 fader-characteristic to rms
rmstofad	 rms to fader-characteristic
round_zero	 round numbers near zero to zero
speedlim	 reduce speed of a numeric stream
split3		 part a numeric stream into 3 ways
split		 part a numeric stream into 2 ways (like moses)
transf_fader	 partial linear characteristic diagram (like table)
v2db		 rms to db
wrap		 wraparound

-------------------- symbol operating -------------------------
mergefilename	 merge a list of symbols together
splitfilename	 divide a symbol into 2 parts
stripfilename	 strip n characters of a symbol
unsymbol	 convert a symbol- to a anything-message

------------------- anything operating ------------------------
any		 store and recall any message (like f, or symbol)
iem_append	 append a message to any messages (obsolete: merge_any)
iem_prepend	 prepend a message to any messages (abbr. pp or prepend)

-------------------------- init -------------------------------
default		 replace initial-argument, if it is zero
dollarg		 receive parent initial-arguments (abbr. $n)
dsp		 control audio-engine, calculate dsp-performance (aka. dsp~)
float24		 store a 24-bit accurate float-number
init		 initialize a message via loadbang (abbr. ii)
once		 any message pass through only the first time
parentdollarzero receive parent unique number

------------------------- counter -----------------------------
exp_inc		 exponential increment counter (bang triggered)
for++		 incremental counter (triggered by internal metro)
modulo_counter	 endless loop counter (bang triggered)

-------------------------- misc -------------------------------
add2_comma	 add a comma-separated message to a messagebox
bpe		 break point envelope controller
f2note		 frequency to midi+cents+note
gate		 interruptible message connection (like spigot)
iem_i_route	 variation of route (abbr. iiroute)
iem_receive	 catch "sent" messages (receive-name-input) (abbr. iem_r)
iem_route	 improvement of route
iem_sel_any	 control a message-box with multiple content
iem_send	 send messages to named object (send-name-input)(ab. iem_s)
pre_inlet	 output an identifier-message and then the incoming message
prepend_ascii    output an identifier-message and then the incoming message
soundfile_info	 output header-info of a wav-file
toggle_mess	 control a message-box with multiple content (abbr. tm)

------------------- parameter handling ------------------------
iem_pbank_csv	 parameter memory manager (csv-format) (like textfile)
list2send	 array of send-objects
receive2list	 array of receive-objects

--------------- t3 - time-tagged-trigger  ---------------------
----------- a time-tag is prepended to each message -----------
----- so these objects allow a sample-accurate access to ------
---------- the signal-objects t3_sig~ and t3_line~ ------------
t3_bpe		 time tagged trigger break point envelope
t3_delay	 time tagged trigger delay
t3_metro	 time tagged trigger metronom
t3_timer	 time tagged trigger timer

-------------- obsolete ---------------------------------------
post_netreceive
pre_netsend