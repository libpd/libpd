building Gem using autoconf
===========================

autoconf is the preferred way to build Gem.

#0 preparation
 if you installed Gem from a released version, just go to the .../Gem directory
 if you grabbed a development snapshot of Gem, you have to generate the autotools
 first; just go to the .../Gem directory and do
   .../Gem$ ./autogen.sh


#1 configuration step
 run configure with the appropriate options
   .../Gem$ ./configure

 for a complete list of options try
   .../Gem$ ./configure --help=recursive

 a typical call would look like:
   .../Gem$ ./configure --with-pd=/usr/include/pd --prefix=/usr -C CXXFLAGS="-g -O2"


#2 build
 once configuration succeeded (it will not succeed if you miss crucial libraries
 like openGL), build Gem:
   .../Gem$ make

#3 finally install
   .../Gem$ make install

 this will install Gem e.g. into /usr/lib/pd/extra/Gem/

