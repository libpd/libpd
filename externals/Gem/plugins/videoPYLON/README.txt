videoPYLON
===========

PYLON-backend for pix_video

PYLON is a proprietary library that can do image-acquisition from 
a number of  GigE-cameras, available from Basler Vision Technologies.
You have to download pylon yourself.
At time o writing pylon is free as in beer, however it is not OpenSource.

	http://www.baslerweb.com/ (and search for "pylon")


installation instructions for linux
===================================
0. obvious prerequisites
you need a C++-compiler, Pd (inclusing headers) and the Gem-sources (including
the videoPYLON plugin)
you should have Gem running (preferrably self-compiled)

1. getting PYLON

get PYLON and install it onto your computer (and make sure to read their
installation instructions).
the directory you installed into shall henceforward be referred to as PYLON_ROOT

note: when running anything PYLONic, you will need to set PYLON_ROOT, GENICAM_ROOT_V1_1 as
environment-variables; 
you will also have to set your library path, so the dynamic linker will find the 
pylon libraries as needed.
for testing it makes also sense to add the pylon's bin-path to your PATH.
it's a good idea to do so right now:
e.g.
$ export PYLON_ROOT=${HOME}/lib/pylon
$ export GENICAM_ROOT_V1_1=${PYLON_ROOT}
$ export PATH=${PYLON_ROOT}/bin/:${PATH}
$ export LD_LIBRARY_PATH=${PYLON_ROOT}/lib
$ PylonViewerApp

if all went well, you should see a number of files from the pylon installation.
if not, adapt PYLON_ROOT to your system (and/or read PYLON's README again)

notes on LD_LIBRARY_PATH:
- if you already have set the LD_LIBRARY_PATH to something else before, make 
sure _add_ to it:
$ export LD_LIBRARY_PATH=${PYLON_ROOT}/lib:${LD_LIBRARY_PATH}
- on 64bit systems, you might need to set it to 
$ export LD_LIBRARY_PATH=${PYLON_ROOT}/lib64
instead

2. compiling videoPYLON

open a terminal and enter this directory
$ cd [Gem]/src/plugins/videoPYLON/

if you haven't done so yet, run autoreconf
$ autoreconf -fiv
(if you built Gem from source, you might have already done so)

now run configure
you will have to tell configure where to find your PYLON.
if you have set PYLON_ROOT, then the PYLON-framework
should be detected automatically, so just run:
$ ./configure

if you haven't set the environment variables, you can specify them via
configure-flags, e.g.:
$ ./configure --with-pylon=${HOME}/lib/pylon

you might also have to tell configure where to find the Pd-sources by specifying
the /path/to/pd with the "--with-pd=" flag.
for more information try:
$ ./configure --help


if all went well, you should see a line "checking for PYLON... yes" near the
end of configure's output.

now run the build:
$ make

you will get an error-message if something failed.

if you want, you can install the plugin into /usr/local/lib/pd/extra/Gem (or
wherever you specified with the "--libdir=" flag, by running
$ make install

you can also manually install the plugin, by copying the file
".libs/gem_videoPYLON.so" next to your Gem-binary.


3. using videoPYLON

start Pd/Gem
create an object [pix_video]
on the Pd-console, you should see (among other things) something like:
"[pix_video]: backend #0='pylon'        : pylon gige"
the backend-# will be different) depending on the number of video-backends found
on your system; it's only important that you find one backend named "pylon"

probably this won't work, with Gem complaining that it cannot find
"libpylonbase.so" or similar.
in this case should tell the linker where to find the pylon-libraries, using
the LD_LIBRARY_PATH variable:
$ LD_LIBRARY_PATH=${PYLON_ROOT}/lib pd -lib Gem
should do the trick. (see above, section 2)

once the plugin loaded correctly, you can start using it.
tell [pix_video] to open a device named like 
"Basler scA640-120gm#0030530F8E64#192.168.1.100:3956"
(that's a single long symbol with _spaces_ (and without quotes)!)
The name consists of vendor, modelname, MAC-address, IP-address and port, which
allows one to quite uniquely identify your camera.
Since such names are a bit awkward to use, you probably want to:
1: connect to your camera with something like "PylonViewerApp" and change
"user defined name" to something like "mycam"
2: enumerate all available devices (using the "enumerate" message to 
[pix_video]
3: access the device with it's short name "mycam".

you really have to run the "enumerate" in order to make [pix_video] aware of the 
user definee name!

examples:
[list Basler scA640-120gm#0030530F8E64#192.168.1.100:3956(
|
[symbol2list]
|
[device $1(
will search for a GigE-camera (scout scA640-120g) at the specified IP/MAC-address 
and connect.

[enumerate, device mycam(
will search for the cam named "mycam"

possible issues (FAQ)
=====================

Q1: my image quality is bad (white stripes)
A1: make sure you have a Gigabit-Ethernet card and enable "Jumbo Frames"
    you can do so by setting the MTU of your network card to 8000-9000
    e.g. use (as root):
    # ifconfig eth1 mtu 9500

Q2: i called the "resetDevice" command on the camera, and now it stopped working
    (it grabs some images and then stops)
A2: make sure you have "Jumbo Frames" also enabled on the _camera_!
    there is a parameter (in PylonViewerApp it is only visible in professional mode
    and it is somewhere in the "Transport" section) called either "PayloadSize" or
    "GevSCPSPacketSize" which you should set to the same value as your MTU.
    see the pylon documentation for more info

Q3: it doesn't work reliably (e.g. it works a bit, and then stops)
A3: make sure you have "Jumbo Frames" on both your network card and your camera

    






fmgasdr
IOhannes m zmölnig
Graz, 18.11.2010






