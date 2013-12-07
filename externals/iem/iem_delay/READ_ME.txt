This library extends the performance of miller puckette's pure data (pd).

iem_delay is written by Thomas Musil from IEM Graz Austria
 and it is compatible to miller puckette's pd-0.37-3 to pd-0.39-2.
see also LICENCE.txt, GnuGPL.txt.

iem_delay contains 3 objects: 
"block_delay~", "n_delay1p_line~" and "n_delay2p_line~"
block_delay~ delays signals to one blocksize.
n_delay1p_line~ is a delay with one signal inlet and n signal outlets
 which change its delay taps during a time period, without interpolation.
n_delay2p_line~ is a delay with one signal inlet and n signal outlets
 which change its delay taps during a time period, with a 2-point interpolation
 between the samples.
