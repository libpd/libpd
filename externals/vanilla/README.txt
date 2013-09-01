
These are just the objects included in Miller's Pd compiled as stand-alone
libraries based on their source files.  Its a quick and dirty hack to strip Pd
down to the bare essentials so that the namespace will be fully functional.

The lib_x_* files are generated using the included script, generate.sh.  They
should not be modified directly.  Ideally, these would be compiled as
individual objects.

The files named after the classes are GUI objects that originally had g_
prefixes on the file names.

This stuff is currently here as a proof of concept for turning Pd core into a
micro-language.  If you want to start modifying these, then we should discuss
how these should be maintained along with Miller's changes.

DO NOT CHANGE THIS CODE!
------------------------

This is not a place to fix bugs or add improvements.  This library is an exact
mirror of the code in Pd-vanilla, warts and all.  The aim is 100%
compatibility in a libdir form.  This way we can have libdirs for each version
number, and then choose to use old versions of this library for compatibilty
(i.e. vanilla-0.42.5, vanilla-0.41.4, vanilla-0.40.3).
