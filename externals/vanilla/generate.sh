#!/bin/sh

echo this script appends, so first delete all files you want to update!

# put these at the top of the file
touch lib_d_fft.c
echo '#include "../../pd/src/d_fftroutine.c"' >> lib_d_fft.c
echo '#include "../../pd/src/d_fft_mayer.c"' >> lib_d_fft.c

for file in ../../pd/src/[dx]_*.c; do 
	 newfile=`echo $file | sed 's|.*/src/\([dx]_\)|lib_\1|'`
	 touch $newfile
	 /bin/echo -n '#include "' >> $newfile
	 /bin/echo -n $file >> $newfile
	 /bin/echo '"' >> $newfile
	 /bin/echo "void "`echo $newfile|sed 's|\(.*\)\.c|\1|'`"_setup(void)" >> $newfile 
	 /bin/echo "{" >> $newfile
	 /bin/echo $file | sed 's|.*src/\(.*\)\.c|    \1_setup();|' >> $newfile
	 /bin/echo "}" >> $newfile
done

# these files hold code for other classes, but no classes
rm lib_d_fftroutine.c lib_d_fft_mayer.c lib_d_resample.c
rm lib_d_fft_fftw.c lib_d_fft_fftsg.c lib_d_fftsg_h.c

# these files have been split out into separate files per object
rm lib_x_interface.c lib_x_qlist.c lib_x_gui.c
