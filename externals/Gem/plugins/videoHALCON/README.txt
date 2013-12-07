videoHALCON
===========

HALCON-backend for pix_video

HALCON is a commercial library that can do image-acquisition from so-called
"professional" hardware, available from MVTec.
You have to purchase their framework (there might be a cheap evaluation version)
in order to use it with Gem.

	http://www.mvtec.com/halcon/



installation instructions for linux
===================================
0. obvious prerequisites
you need a C++-compiler, Pd (inclusing headers) and the Gem-sources (including
the videoHALCON plugin)
you should have Gem running (preferrably self-compiled)

1. getting HALCON

get HALCON and install it onto your computer (and make sure to read their
installation instructions).
the directory you installed into shall henceforward be referred to as HALCONROOT
the architecture you chose (eg. "x86-linux2.4-gcc40") shall be called HALCONARCH

note: when running anything HALCONic, you will need to set HALCONROOT and
HALCONARCH as environment-variables; it's a good idea to do so right now:
e.g.
$ export HALCONROOT=${HOME}/lib/halcon
$ export HALCONARCH=x86-linux2.4-gcc40
$ ls -l ${HALCONROOT}/lib/${HALCONARCH}

if all went well, you should see a number of files from the halcon installation.
if not, adapt HALCONROOT and HALCONARCH to your system.


2. compiling videoHALCON

open a terminal and enter this directory
$ cd [Gem]/src/plugins/videoHALCON/

if you haven't done so yet, run autoreconf
$ autoreconf -fiv
(if you built Gem from source, you might have already done so)

now run configure
you will have to tell configure where to find your HALCON.
if you have set HALCONROOT and HALCONARCH correctly, then the HALCON-framework
should be detected automatically, so just run:
$ ./configure

if you haven't set the environment variables, you can specify them via
configure-flags, e.g.:
$ ./configure --with-halcon=${HOME}/lib/halcon --with-halconarch=x86-linux2.4-gcc40

you might also have to tell configure where to find the Pd-sources by specifying
the /path/to/pd with the "--with-pd=" flag.
for more information try:
$ ./configure --help


if all went well, you should see a line "checking for HALCON... yes" near the
end of configure's output.

now run the build:
$ make

you will get an error-message if something failed.

if you want, you can install the plugin into /usr/local/lib/pd/extra/Gem (or
wherever you specified with the "--libdir=" flag, by running
$ make install

you can also manually install the plugin, by copying the file
".libs/gem_videoHALCON.so" next to your Gem-binary.




3. using videoHALCON

start Pd/Gem
create an object [pix_video]
on the Pd-console, you should see (among other things) something like:
"[pix_video]: backend #0='halcon'        : halcon iidc gige falcon"
the backend-# will be different) depending on the number of video-backends found
on your system; it's only important that you find one backend named "halcon"

probably this won't work, with Gem complaining that it cannot find
"libhalconcpp.so" or similar.
in this case should tell the linker where to find the halcon-libraries, using
the LD_LIBRARY_PATH variable:
$ LD_LIBRARY_PATH=${HALCONROOT}/lib/${HALCONARCH} pd -lib Gem
should do the trick.

an alternative would be to install the files found in
${HALCONROOT}/lib/${HALCONARCH} into your system's library folder (e.g.
/usr/local/lib/) and/or add this folder to /etc/ld.so.conf

afaik, you really MUST set the HALCONROOT, in order to make halcon find your
license files.

once the plugin loaded correctly, you can start using it.
tell [pix_video] to open a device named like "<name>:<cameratype>:<device>"
(that's 3 parameters, separated by colons)
the exact meaning of these parameters can be obtained from the HALCON
documentation for your halcon-backend
http://www.mvtec.com/download/release/hacq-x86sse2-linux2.4-gcc40-9.0.html

examples:
[device File:../examples/data/(
this will use the HALCON's "File" (virtual framegrabber) driver, that will read
all images in the ../examples/data directory and display them.

[device GigEVision::0013d453dbc1(
this will use the "GiGE" backend, using the "default" camera-description
(meaning that it will be obtained from the camera; you could also specify an
XML-file here), and connecting to the GiGE camera with the MAC-address
00:13:d4:53:db:c1


which exact HALCON-backends are available depends on what you have installed.


possible issues
===============
due to lack of hardware (and HALCONs outdated iidc interface) i have not been
able to test anything but the "File" backend.
in theory however, all should work

some backends (e.g. "1394IIDC") require a camera-type containing colons; since
colons are already used for delimiting, this obivously won't work.
if you have idea's (and code) to solve this, i will be happily include them.


fmgasdr
IOhannes m zmölnig
Graz, 23.06.2010






