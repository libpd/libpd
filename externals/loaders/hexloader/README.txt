---------
hexloader
---------
(c) copyleft 2006-2007 IOhannes m zmölnig, IEM


the problem
-----------
when dealing with abstractions and single-file externals, we have a 1-to-1 
correspondence between an objectclass and a file.
examples:
  the [expr] object matches an external "./expr.pd_linux"
  the [zexy/nop~] object matches an abstraction "./zexy/nop~.pd"
general:
  the object [<name>] matches a file "<name>.$EXT"
when dealing with externals, the name of the objectclass is also to be found 
in the setup function within the external
example:
  to load the "expr" external, Pd will look for a function 
  "expr_setup()" within the file "./expr.pd_linux"
general:
  the external "<name>" has to provide a setup-function 
  "void <name>_setup(void)"

this works fine, as long as there are no special characters involved:
according to the above scheme, when loading the external "expr~"
Pd should actually be looking for "expr~_setup()" within the file 
"./expr~.pd_linux"; 
unfortunately, the C-language (wherein most externals are written) does not 
allow function-names to use any characters other than "a-zA-Z0-9_".
therefore the name "expr~_setup()" is invalid;
therefore, Pd handles the case of the trailing "~" in a special way: it 
actually expands the "~" to "_tilde" and looks for "expr_tilde_setup()"
general:
  the object [<name>~] corresponds to an external "<name>~.pd_linux" which 
  offers a setupfunction "<name>_tilde_setup()"
unfortunately this doesn't work well for a larger number of characters 
forbidden in C-functionames.
example:
 how should the setupfunction for an objectclass [~<~] be named?

additionally, several filesystems have problems with certain characters.
for instance, you cannot use the character '>' on FAT32 filesystems.
but how do you then create a file for the objectclass [>~]?


a solution
----------
one solution is to just forbid objects with weird (non alphanumerical) names.
pretty frustrating

another solution
----------------
use libraries with valid names that hold objects with weird names

a better solution
-----------------
another solution is to translate the forbidden characters into allowed ones.
for instance, every ASCII-character can be called by it's name (e.g. 'a') as
well as by it's numeric value "97" (or in hex: "0x61")
the here-proposed solution is, to replace every illegal character by it's 
hexadecimal representation (in lowercase).
examples:
  [>] corresponds to the file "0x3e.pd"
  [>~] corresponds to the file "0x3e0x7e.pd" or to "0x3e~.pd"
  [a>b] corresponds to the file "a0x3eb.pd"

the same system can be applied to the setup-functions:
examples:
  [a>b] has a setupfunction "a0x3eb_setup()"
CAVEAT:
C also forbids to start function names with numeric values.
therefore, this is a nono:
  [>]'s setupfunction would be "0x3e_setup()" (ILLEGAL)
we can simply fix this by putting the "setup()" in front:
 [>] has a setupfunction "setup_0x3e()"

implementation
--------------
the "hexloader" external adds the possibility to Pd use the proposed
alternative naming scheme.
just load the "hexloader" lib, and the next time you try to create the
(yet unknown) object X, Pd will also look for several alternatives
example:
  [>~]
	">~.pd_linux"			(filename ILLEGAL on SOME filesystems)
		>~_setup() 		(ILLEGAL functioname)
		setup_>~		(ILLEGAL functioname)
		0x3e0x7e_setup()	(ILLEGAL functioname)
		setup_0x3e0x7e()
	"0x3e~.pd_linux"
		0x3e~_setup()		(ILLEGAL functioname)
		setup_0x3e~()
		0x3e0x7e_setup()	(ILLEGAL functioname)
		setup_0x3e0x7e()
	"0x3e0x7e.pd_linux"
		0x3e0x7e_setup()	(ILLEGAL functioname)
		setup_0x3e0x7e()	
	">~.pd"				(filename ILLEGAL on SOME filesystems)
	"0x3e~.pd"
	"0x3e0x7e.pd"
the hexloader will try one alternative after another until it finds one
that actually works (there is no problem in querying the ILLEGAL names,
it is just impossible to create such filenames/setupfunctions)


handling of "/"
---------------
a special problem is the "/" character, as this might be a special character
in the name of an object and the path-delimiter
example:
	"a/b" might be a file "b" in the directory "a" or just "a/b"
solution
--------
hexloader treats the "/" in a special way, as it tries all possibilities
example:
  [a/b}
	a/b	(file "b" in directory "a")
		b_setup()
		setup_b()
	a0x2fb	(file "a/b")
		a0x2fb_setup()
		setup_a0x2fb()


CAVEATS
=======
obviously the abstraction "0x3e.pd" can be addressed as both [>~] and [0x3e]
this basically means, we have now an n-to-1 correspondence!
in the case of externals this could be fixed by only using the "setup_*"
naming scheme (instead of Pd's standard "*_setup()")
for practical reasons this has been dumped (too many devs created the wrong
setupfunction and couldn't tell why the externals would not load)
