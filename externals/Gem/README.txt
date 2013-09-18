This is GEM - Graphics Environment for Multimedia
=================================================
You can get the current distribution from:
http://gem.iem.at
http://sourceforge.net/projects/pd-gem

=============================================
NEW:::
------

Gem is now supported by W32, linux and macOS-X
the IRIX version might work (but most probably will not): if you want to use Gem under IRIX,feel free to make it work and report failure and success to me, so i can incorporate any needed changes into the main Gem-trunk.

for installation instructions see below


NEW (gem-0.87):::
-----------------
(note: this NEWs only refer to the packaging, not to features of Gem)

As with GEM-0.87, i have broken the distribution into various packages
gem-<gemver>.tgz :: quite everything (except binaries and auxiliary libraries)
gem-doc-<gemver>.tgz :: example-patches, manual, ...
gem-bin-<gemver>.zip :: W32-binary (containing a single file "Gem.dll")
gem-bin-doc-<gemver>.zip :: W32-binary + documentation

GemLibs-<OS>-<libver>.tgz :: auxiliary libraries (used to be "AuxLibs")

1) the core Gem-packages:
the core Gem-packages all extract into the same directory gem-<gemver>/
there are install-scripts for windoze (and probably IRIX)
these will install the documentation...


2) the GemLib-package:
I don't know, whether it has much sense, to break the core Gem-package (doc/src/bin),
but i do know, that the GemLibs should be in a distinct package (just for the sake of downloading)

the GemLibs have their own version numbering, starting with 1.
You can get the newest GemLib from the place mentioned above


-------------------------------------
-------------------------------------

INSTALLATION:
=============

-------------------------------------
To install GEM on linux/OSX/...:
run
$ ./configure
$ make
$ make install

note that you might want to help Gem to find the Pd-headers, e.g. by doing
$ ./configure --with-pd=/usr/include/pd
try
$ ./configure --help to see more options

-------------------------------------
To install GEM on W32:
a) installer (preferred method)
  use the installer executable to install Gem into ...\pd\extra
  (to _build_ the installer you will have to have NSIS installed
  see build/win-nsis for details)

b) archive (do it by hand)

	1) unzip the GEM package

	2) put the subfolders of Gem-<version>\ into the "extra" folder of your
		Pd installation
	   e.g. if you installed Pd as "C:\Program Files\Pd-0.43-0" you should
		end up with:
		"C:\Program Files\Pd-0.43-0\extra\Gem"
		and eventually with
		"C:\Program Files\Pd-0.43-0\extra\pix_drum"
		"C:\Program Files\Pd-0.43-0\extra\pix_mano"
		...

	   there is no need to copy the README.txt found in Gem-<version>\ into
		"extra"

	3) please note that the archive comes with all plugins
		in most cases, you won't need all of them, and having plugins
		installed that you don't need, will considerably slow down load
		time of patches and might leed to undesired side-effects.
		therefore, if loading is too slow (or you experience weird
		things), it might be a good idea to disable plugins you don't
		need.
		disabling plugins is as simple as deleting them (or moving them
		into a subfolder)

	   plugins are files of the form: "gem_<type><NAME>.dll"
		e.g. "gem_filmQT.dll" is a plugin for reading films using the
		QuickTime framework.
		in order to use videoPYLON, videoHALCON and videoAVT, you need
		to install proprietary libraries yourself; if you haven't done
		so or don't own a device that can interact with those libraries,
		you can safely remove these plugins.
		if you have no clue what this is about, these plugins are most
		likely not for you (so remove them)



-------------------------------------
-------------------------------------

RUNNING:
========

just installing Gem is not enough !
you will have to tell pd that it should load that library !!
you cannot create any Gem-objects without having loaded the Gem-library into pd !!!

make sure you have the proper binary for you OS
  - windows: Gem.dll
  - macOS-X: Gem.pd_darwin
  - linux  : Gem.pd_linux
  - irix   : Gem.pd_irix
  - ...

when starting pd, tell it to load Gem with the "-lib" flag
if your Gem-binary lives in pd/extra/, you could just try "pd -lib Gem"
if your Gem-binary lives somewhere else use something like "pd -lib /path/to/my/Gem"
you could also use "pd -path /path/to/my -lib Gem"
after loading Gem you will see a bit of a welcome message on the konsole

NOTE: there *must not* be any file extension with the "-lib"-flag:: "pd -lib Gem.dll" will miserably fail
NOTE: please note the spelling: it is "Gem" and not "gem" nor "GEM"

if you have problems loading any library including Gem, have a look at the pd-documentation
if you have problems loading just Gem (but other libraries work), send me a bug-report (see below)

BUG-REPORTS:
============
please do not hesitate to report any crashes, weirdnesses or other issues, using
the bugtracker at sourceforge:
http://sourceforge.net/projects/pd-gem/
or the gem-dev mailinglist (subscription at http://lists.puredata.info)

if your mail only says "hey, it does not work !", it is an annoyance and no bug-report
please specify at least the following things:
	Operating-System (kernel-version,...)
	video-card, driver
	other hardware that is related to your problem (e.g.: camera)
	does your system work with similar applications (e.g.: capture-software, openGL (games, "glxgears")
	what is the output of pd when you start it with the "-verbose" flag (e.g.:"pd -verbose -lib Gem")

please do not use the puredata bugtracker for reporting gem-specific bugs.

----
have fun

zmoelnig@iem.at

