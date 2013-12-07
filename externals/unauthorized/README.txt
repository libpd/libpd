
unauthorized is a library of GUI objects and a handful of objects for working
with streaming and mp3s.  The home page for the GPL version is here:

http://puredata.info/community/projects/software/unauthorized

unauthorized has split into two versions because of a change of
licensing. This version, maintained in the pure-data SVN, remains GPLv2
while the original author Yves Degoyon has split off a version with a
non-free license.

To get the new non-free version, download it from its new CVS repository:

 export CVSROOT=:pserver:anonymous at giss.tv:/home/cvs
 cvs co pidip
 cvs co unauthorized

original sources available from http://ydegoyon.free.fr/software.html

Installing
----------

there is no configure scripts, you need to install :

 * pd headers in /usr/include ( m_pd.h and g_canvas.h )
 * libmp3lame-dev for the mp3 externals
 * libspeex-dev for the speex external
 * sources of stk in /usr/src/stk for stk externals ( optional )

have fun!
sevy ( ydegoyon@gmail.com )
Copyright 2002 Yves Degoyon

Included Objects
-------------

audience~ : 2-dimensional audience simulation ( also called 2 dollars spatialization !!! ) 

beatify~ : modulate amplitude of a sound
    the idea was borrowed from musicscript,
    an excellent sound generation tool.
    ( http://musicscript.sourceforge.net, author : David Piott )

blinkenlight : a blinken lights films player ( but also a pixel grid )
    This object displays blinken lights movies, and lets you handle a
    grid of pixels.  It is also a "Telecran" !!  There is a movies
    archive @ http://www.blinkenlights.de/gallery/index.en.html

compressor~ : a compressor of audio signals

cooled : a micro sound editor
    This object displays a sound, lets you play a part of it and do
    some cut and paste operations.

disto~ : a kind of effect used in pop music, use it elsewhere
    the algorithm was taken from Digital Effects (DISTORT3), a guitar
    effects software for DOS which rocks, written by Alexey Smoli
    http://st.karelia.ru/~smlalx/

exciter : a bang-events sequencer
    the need for it sprung out of a talking with Nicolas Lhommet.

filterbank~ : filterbank outputs frequency response for a range of filters

formant~ is a formant synthesis generator external for pd

grid : 2-dimensional control object, ala "kaospad"

mp3amp~ is a MPEG I Layer III (mp3) icecast/shoutcast client for Pure Data
    mp3amp~ has been compiled for Linux using LAME 3.92.
    The newest version of LAME can be found at sourceforge.net

    PLEASE NOTE: This software may contain patented alogrithm (at
    least patented in some countries). It may be not allowed to
    sell/use products based on this source code in these
    countries. Check this out first!

    COPYRIGHT of MP3 music: Please note, that the duplicating of
    copyrighted music without explicit permission violates the rights
    of the owner.

    Using mp3amp~ external for Pure Data

    Open the help-mp3amp~.pd to understand how it works.  Open the
    help-graphic-mp3amp~.pd if you want to see the status of the
    incoming stream.

    BUGS :
    a/ certainly, not all bitrates will work,
    that's too tedious to test, 128, 256, 320 are ok.

    b/ cannot instantiate more than 10 decoders, if needed change
    MAX_DECODERS in the code

mp3cast~ is a MPEG I Layer III (mp3) streaming external for pd (by Miller 
Puckette) that connects to a SHOUTcast or IceCast server.

    Using mp3cast~ external for Pure Data

    Open the help-mp3cast~.pd to understand how it works.
    In this patch, you must send the messages to mp3cast~ 
    in the following order :

    1/ password ***** 
    2/ icecast | mp3cast
    3/ connect host port
    4/ pd dsp 1

    Parameters sent to mp3cast~ object :

    Sampling Rate (Hz): Possible values are 48000, 44100 and 32000. If
    Pd runs at a different sampling rate, LAME will resample the
    signal. Default value for mp3 sampling rate is Pd's sampling rate.

    Bitrate (kbit/s): Possible values are 32, 40, 48, 56, 64, 80, 96,
    112, 128, 160, 192, 224, 256 and 320. Default is 224.

    Mode: Possible values are 0 (stereo), 1 (joint stereo, the
    default), 2 (dual channel) and 3 (mono).

    Password: The default is 'pd', can be changed with a message
    "passwd yourpassword".

    Server: Use message "connect name_of_your_server.com port" to
    connect (same as with Pd's netsend). 'port' is the number
    specified in the server's config file.  Attention (for SHOUTcast
    users): The actual port number used is one higher!  Standard would
    be 8000 resulting in a socket at port 8001!!! Bare this in mind
    when configuring proxys or using mp3cast~ in connection with
    netsend / netreceive. For IceCast, the port number used is the
    same as specified.

    Outlet: The outlet outputs an int, 1 if connected to SHOUTcast
    server, 0 if not. This could be used to build an automatic
    reconnect mechanism.

    Other things: mp3cast~ prints the current status (connection,
    login, LAME status) to the pd window. To see the current settings,
    send it a message "print" and mp3 settings will be displayed.
    Note that changing any mp3 settings will require to disconnect and
    reconnect again!  This has to be done manually.

    Known problems: If you turn off audio processing when you are
    connected with the server, no data will be sent to it. This will
    make the server disconnect after a certain time ('no data' error
    in server log). mp3cast~ does not recongnise this and attempts to
    keep on streaming. To avoid this set 'AutoDumpSourceTime' in the
    servers config file to a fairly high value preventing the server
    from closing the socket to fast.

    ALLOWED QUALITY FACTOR :

    -q <arg>        <arg> = 0...9.  Default  -q 5 
                    -q 0:  Highest quality, very slow 
                    -q 9:  Poor quality, but fast 
    -h              Same as -q 2.   Recommended.
    -f              Same as -q 7.   Fast, ok quality

    ALLOWED SAMPLERATE/BITRATES

    MPEG-1   layer III sample frequencies (kHz):  32  48  44.1
    bitrates (kbps): 32 40 48 56 64 80 96 112 128 160 192 224 256 320

    MPEG-2   layer III sample frequencies (kHz):  16  24  22.05
    bitrates (kbps):  8 16 24 32 40 48 56 64 80 96 112 128 144 160

    MPEG-2.5 layer III sample frequencies (kHz):   8  12  11.025
    bitrates (kbps):  8 16 24 32 40 48 56 64 80 96 112 128 144 160

mp3live~ is a peer-to-peer mp3 streamer package consisting of three
    objects : mp3streamout~, mp3fileout~ and mp3streamin~.

    using mp3live~ external for Pure Data

    Open the mp3live~-help.pd to understand how it works.

    A note about MPEG encoding :

    ALLOWED QUALITY FACTOR :

    -q <arg>        <arg> = 0...9.  Default  -q 5 
                    -q 0:  Highest quality, very slow 
                    -q 9:  Poor quality, but fast 
    -h              Same as -q 2.   Recommended.
    -f              Same as -q 7.   Fast, ok quality

    ALLOWED BITRATES :

    bitrates (kbps): 32 40 48 56 64 80 96 112 128 160 192 224 256 320

    BUGS :

    1. You cannot create more than MAX_DECODERS mp3streamin~
    objects. The actual value is 100.

    2. Current version of lame ( 3.92 ) produces a lot of errors for
    quality < 5. Blame it on lame !!!!

    3. Mono is not supported. Some additional code should be added for
    mp3streamin~. Blame it on me !!!

    4. Resampling is not supported. Blame it on me !!!

mp3write~ is a MPEG I Layer III (mp3) file writer.

    using mp3write~ external for Pure Data

    Open the help-mp3write~.pd to understand how it works.
    In this patch, you must send the messages to mp3write~ 
    in the following order :

    1/ append|truncate if you wish to change file creation options ( default is append )
    2/ open /my/file 
    3/ start
    5/ pd dsp 1
    4/ stop : the tag is written at this stage

    Parameters sent to mp3write~ object :

    Sampling Rate (Hz): Possible values are 48000, 44100 and 32000. If
    Pd runs at a different sampling rate, LAME will resample the
    signal. Default value for mp3 sampling rate is Pd's sampling rate.

    Bitrate (kbit/s): Possible values are 32, 40, 48, 56, 64, 80, 96,
    112, 128, 160, 192, 224, 256 and 320. Default is 224.

    Mode: Possible values are 0 (stereo), 1 (joint stereo, the
    default), 2 (dual channel) and 3 (mono).

    Outlet: The outlet outputs an int, which the number of bytes
    written in this session.  this might be different from file size
    if you're using append mode.

    Known Problems : All combinations of samplerate, bitrate, quality
    factor will not be accepted.

    ALLOWED QUALITY FACTOR :

    -q <arg>        <arg> = 0...9.  Default  -q 5 
                    -q 0:  Highest quality, very slow 
                    -q 9:  Poor quality, but fast 
    -h              Same as -q 2.   Recommended.
    -f              Same as -q 7.   Fast, ok quality

    ALLOWED SAMPLERATE/BITRATES

    MPEG-1   layer III sample frequencies (kHz):  32  48  44.1
    bitrates (kbps): 32 40 48 56 64 80 96 112 128 160 192 224 256 320

    MPEG-2   layer III sample frequencies (kHz):  16  24  22.05
    bitrates (kbps):  8 16 24 32 40 48 56 64 80 96 112 128 144 160

    MPEG-2.5 layer III sample frequencies (kHz):   8  12  11.025
    bitrates (kbps):  8 16 24 32 40 48 56 64 80 96 112 128 144 160

    Furthermore, it seems that high quality factors will not work with
    this release of lame ( 3.92 ).  The same errors can be obtained
    with the command line : lame -q 1 file.wav outputs errors and
    mp3write can't do better.

pianoroll : a graphical sequencer controller

playlist : choose a file in 1 click with space, numeric, characters
     you can also send a seek message to select a file automatically.

probalizer : outputs integer values according to a drawn probability curve

samplebox~ : an opaque box to record and playback a sound ( with speed variations )

scratcher~ : records a sound and, then, let's you scratch it with your mouse.

scrolllist : displays and scrolls a text in  a patch window

sonogram~ : displays, plays back and lets you modify a recorded sonogram.

    The real and imaginery part of an fft~ is stored in a sonogram~
    and, then, you can apply modifications to it or do mouse-based
    graphic modifications.  The set of modifications provided for now
    consists of matrix operations but it will be improved soon.

speex~ is a voice quality streamer using Speex library consisting of
    two objects : speexin~ and speexout~.  A big thanx to Jean-Marc
    Valin, author of Speex who helped me fixing encoding/decoding
    problems.

    PLEASE NOTE: The speex codec is patent free unlike GSM codecs.
    that's the main reason why it's been choosen.  ( + it allows very
    low throughputs like 8kbits/s ).

spigot~ : a signal router.

vocoder~ : vocoder filter for PD inspired by xvox ( http://simon.morlat.free.fr )
i used xvox version 0.2.1, but you don't need to install it,
everything needed is bundled here.

wahwah~ : a kind of effect used in psychedelic music, use it elsewhere

    the algorithm was taken from Digital Effects, a guitar effects
    software for DOS which rocks, written by Alexey Smoli (
    http://st.karelia.ru/~smlalx/ )

