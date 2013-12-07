This library extends the performance of miller puckette's pure data (pd).

iem_spec2 is written by Thomas Musil from IEM Graz Austria
 and it is compatible to miller puckette's pd-0.37-3 to pd-0.39-2.
see also LICENCE.txt, GnuGPL.txt.

iem_spec2 contains 23 objects: 
all of them calculate only blocksize/2 + 1 samples of a signal vector;

spec2_abs~, spec2+~, spec2*~, spec2-~, spec2_dbtopow~, spec2_dbtorms~, 
spec2_powtodb~, spec2_rmstodb~, spec2_sqrt~, spec2_tabreceive~ 
do the same as objects without spec2_ prefix;

the other objects are: spec2_1p1z_freq~, spec2_1p1z_time~, spec2_+s~,
spec2_block_delay~, spec2_clip_max~, spec2_clip_min~,
spec2_matrix_bundle_stat~, spec2_*s~, spec2_shift~, spec2_stretch~,
spec2_sum~, spec2_tab_conv~, spec2_tabreceive_enable~.
	


