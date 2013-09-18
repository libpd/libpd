videoAVT
========

backend for [pix_video] using Prosilica's "AVT GiGE SDK"

"AVT GiGE SDK" is a commercial library that can do image-acquisition from
GiGE-cameras.
"AVT" stands for "Allied Vision Technologies" and is the consortium defining the
GiGE-standard.

You have to get Prosilica's SDK from either
 - http://prosilica.com
 - http://www.alliedvisiontec.com/us/products/software.html
 - or whereever they have moved it now
in order to use it with Gem.

At the time of writing, the SDK runs on W32, linux, OSX and QNX


variable video-backends are available in Gem>=0.93


Installation on un*x
=====================
first you have to install the SDK from Prosilica: download and extract to
whereever you want; i will refer to this path as AVTSDKPATH.
$ tar xvzf AVT_GigE_SDK_1.24_Linux.tar.gz
$ cd AVT\ GigE\ SDK/
$ export AVTSDKPATH=$(pwd)
$

after installations you should have something like this:
$ cd ${AVTSDKPATH}
$ ls
bin-pc  documents  examples  inc-pc  lib-pc  README.txt
$

static building
---------------

static linking has the great advantage, that you are not depending on the dylib
anymore - you basically only need the binary produced by make, and can move it
between computers,...

now let's go back to the videoAVT directory (where this README.txt lives in).
you have to tell the build-process where to find includes and libraries
$ ./configure PKG_LIBPVAPI_CFLAGS="-I${AVTSDKPATH}/inc-pc/" PKG_LIBPVAPI_LIBS="-L${AVTSDKPATH}/lib-pc/x86/4.4/ -lPvAPI" 
$ make
$ make install
$

you might have to adjust the exact flags: "x86" is for x86-architectures, you
might want to x64, ppc, arm or whatelse, depending on your hardware.
also the  exact library version might be different (e.g. "4.4" only seems to
exists on x86 and x64); see what's there...

dynamic linking
---------------
dynamic linking has the advantage, that the resulting binary is smaller (because
the code from the SDK-library is kept somewhere else), and multiple binaries can
share the very same code. also it makes bugfixing easier, as only the dylib has
to be replaced.

now let's go back to the videoAVT directory (where this README.txt lives in).
you have to tell the build-process where to find includes and libraries
$ ./configure PKG_LIBPVAPI_CFLAGS="-I${AVTSDKPATH}/inc-pc/" PKG_LIBPVAPI_LIBS="-L${AVTSDKPATH}/bin-pc/x86 -lPvAPI"
$ make
$ make install
$

again, you might have to adjust the exact flags to fit your system.


loading
=======
the plugin is called gem_videoAVT.so (un*x) or gem_videoAVT.dll (w32).
it has to live in the same folder as the Gem-binary (Gem.pd_linux on linux,
Gem.pd_darwin on OSX, Gem.dll on w32) if you want to use it.
the "make install" should make sure of this, but it's always better to
double-check.

start "pd -lib Gem" and create a [pix_video] object.
on the Pd-console you should see something like that (among other things):
 [pix_video]: backend #0='avt'	: avt gige
the exact backend-# will depend on your system.
if no kline with backend 'avt' shows up, check both the Pd-console and the
stderr for hints, why the library couldn't be loaded.

if you have linked dynamically, then the dlinker might not find the libPvApi.so
libary
either add it's path to the LD_LIBRARY_PATH:
$ LD_LIBRARY_PATH=${AVTSDKPATH}/bin-pc/x86:${LD_LIBRARY_PATH} pd -lib Gem 
$
or install this the .so file somewhere the linker can find it (/usr/local/lib)
and/or make sure the linker looks for this path via ld.so.conf


using
=====
once [pix_video] has been initialized with the AVT backend, you can start using
it.
you can specify the device via the GiGE cam's IP-address, it's UniqueID,
SerialString or DisplayName.
The UniqueID, should be specified in hexadecimal.
If several cameras share the same SerialString or DisplayName, the 'first' one
found will be used.
the address can be specified both as an IP or a host-name.
e.g.
[device 192.168.1.34(
or
[device gige1.local(















TODO
====

testing
 i don't have access to a GiGE-camera; so the entire code is currently untested

colorspaces
 currently only Mono8, Rgb24, Bgr24, Rgba32 & Bgra32 are supported




