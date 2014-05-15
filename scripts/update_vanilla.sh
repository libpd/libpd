#! /bin/bash
#
# a simple script to pull in the pd vanilla sources
#

WD=$(dirname $0)

VER=0.45-4

SRC=pd-$VER
DEST=../pure-data

###

cd $WD

# get latest source
curl -LO http://crca.ucsd.edu/~msp/Software/$SRC.src.tar.gz
tar -zxf $SRC.src.tar.gz

# remove uneeded makefiles, etc in src
find $SRC/src -name "makefile.*" -delete
find $SRC/src -name "Makefile.am" -delete
rm $SRC/src/notes.txt
rm $SRC/src/pd.ico
rm $SRC/src/pd.rc

# remove unneeded audio apis
rm $SRC/src/s_audio_alsa*
rm $SRC/src/s_audio_audiounit.c
rm $SRC/src/s_audio_esd.c
rm $SRC/src/s_audio_jack.c
rm $SRC/src/s_audio_mmio.c
rm $SRC/src/s_audio_oss.c
rm $SRC/src/s_audio_pa.c
rm $SRC/src/s_audio_paring.*

# remove unneeded midi apis
rm $SRC/src/s_midi_alsa.c
rm $SRC/src/s_midi_dummy.c
rm $SRC/src/s_midi_mmio.c
rm $SRC/src/s_midi_oss.c
rm $SRC/src/s_midi_pm.c
rm $SRC/src/s_midi.c

# remove uneeded fft library interfaces
rm $SRC/src/d_fft_fftsg.c
rm $SRC/src/d_fft_fftw.c

# remove some other stuff we don't need ...
rm $SRC/src/s_entry.c
rm $SRC/src/s_watchdog.c
rm $SRC/src/u_pdreceive.c
rm $SRC/src/u_pdsend.c

# make sure dest dirs exist
mkdir -p $DEST/src
mkdir -p $DEST/extra

# copy sources
cp -Rv $SRC/src/* $DEST/src
cp -Rv $SRC/extra/* $DEST/extra

# cleanup
rm -rf $SRC $SRC.src.tar.gz
