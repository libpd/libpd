extras for Gem
==============

this folder contains objectclasses that are shipped with Gem but are not linked
into Gem.

reasons why these objectclasses are not part of Gem proper are:

- license issues
  (Gem is GPL, objects that are not GPL would go in here)

- dependency issues
  (we try to keep the dependencies of Gem-core to the minimum of openGL and
   WindowManager (if at all); any objectclass that makes heavy use of 3rd party
   libraries would go in here)

- specialist objects
  (objectclasses that are highly specialized and of no common interest might go
   in here as well)



ADDING your own subdirectory to extra/ using autoconf:
- Gem/extra/Makefile.am
	add your projects to the SUBDIRS variable
	if your project has a "Makefile" that has all the usual autoconf
	targets, you are done :-)
	see Gem/extra/Makefile.am for an example
	.
	this applies to most projects!

- Gem/configure.ac
	if you provide a Makefile.am and want Gem's build process to create a
	Makefile from that, you have to add [extra/<project>/Makefile] to the
	AC_CONFIG_FILES in Gem/configure.ac;
	see Gem/configure.ac for an example
	.
	this applies to small projects (no additional dependencies) that are
	to be shipped with Gem

- Gem/extra/configure.ac
	if you provide your own autoconf system (configure.ac), add your project
	to the AC_CONFIG_SUBDIRS in Gem/extra/configure.ac
	see Gem/extra/configure.ac for an example
	.
	this applies to bigger projects (additional dependencies) or projects
	not to be shipped with Gem

