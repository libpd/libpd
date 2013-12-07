
 Tcl for Pd
 ==========

This library allows to to write externals for Pd using the Tcl language.
It wraps quite closely the pd API (m_pd.h, plus some private functions)

Also a library of Tcl helper functions is provided. It is not mandatory to use
it (moreover: it requires Tcl 8.5, while the tclpd external alone requires only
Tcl 8.4), but it is a syntactic sugar and can simplify a lot the code.
To use it simply add 'package require TclpdLib' in your Tcl external.

Anyway, disregarding any approach chosen to develop Tcl externals, a general
knowledge of Pd internals (atoms, symbols, symbol table, inlets, objects) is
strongly required. (Pd-External-HOWTO is always a good reading)


 Compiling and installing
 ========================

To compile tclpd, simply type:

  make clean all

To compile it with debug enabled:

  make DEBUG=1 clean all

Requirements are pd >= 0.39, swig, c++ compiler.
To install tclpd, simply copy it to /usr/lib/pd/extra (or where you installed
pure-data).


 Writing GUI externals
 =====================

Pd is split into two processes: pd (the core) and pd-gui.
A pd external executes in the core. The same applies for a Tcl external loaded
by tclpd, because tclpd creates a Tcl interpreter for that, running in the
same process as pd.

On the gui side (pd-gui) there is another Tcl interpreter living in a separate
process, which communicates with pd using a network socket.
Communication happens in one way (pd to gui) with the sys_gui function, and in
the other way using ::pdsend. (needs to set up a receiver using pdbind, check
the examples).


 Data conversion between Tcl <=> Pd
 ==================================

In pd objects communicate using messages, and messages are made up of atoms.
An atom could be a float, a symbol, a list, and so on.
Tcl usually doesn't make distinction between strings and numbers. This means
that simply translating a message text into a string could lose information
about the atom type (to pd, symbol 456 is different from float 456, but if we
just convert it as a string "456" the type information is lost).

To maintain atom type infrmation, pd atoms are represented in Tcl as two
element lists, where the first element indicates the atom type.

Some examples of this conversion:

 Pd:  456
 Tcl: {float 456}

 Pd:  symbol foo
 Tcl: {symbol foo}

 Pd:  list cat dog 123 456 weee
 Tcl: {{symbol cat} {symbol dog} {float 123} {float 456} {symbol wee}}


 Examples
 ========

Some examples externals are provided, including their helpfile.


 Authors
 =======

Please refer to AUTHORS file found in tclpd package.


 License
 =======

Please refer to COPYING file found in tclpd package.


