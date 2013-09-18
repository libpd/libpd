------
libdir
------

This package provides support for the libdir format of libraries that is used
in Pd-extended.  It should work with any version of Pd 0.40 or newer.

The 'libdir' loader is a Pd loader which supports the libdir library
format.  The libdir library format aims to be a common library format
for Pd which works with objects written in any language, including
Pd. This library format was designed to be easy to create, install,
and use. It should work when installed into the global path
(i.e. pd/extra) or when copied locally into a project folder. It
should work with objects written in any supported language
(i.e. binaries, .pd, and the various loaders like pdlua and
tclpd). Also, starting with Pd 0.43 and Pd-extended 0.42, the Help
Browser dynamically builds itself based on the libraries that are
installed.

To install, copy the files for your platform into your "extra" folder and the
help patches to the "doc/5.reference" folder.  You will need to load the
libdir.dll/libdir.pd_linux/libdir.pd_darwin as a library before trying to load
any libdirs.  The libdirs have to be in the global classpath in order to be
found.

You can add a library two ways:
 - add this text to one of the fields in the "Startup" prefs:  libdir
 - load it on the command line with this:  -lib libdir

For more info on the structure of libdirs, see this webpage:

http://puredata.org/docs/developer/Libdir

