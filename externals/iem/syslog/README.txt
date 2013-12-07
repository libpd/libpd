syslog for Pd
=============

allows to send syslog messages of a settable severity from with Pd.


installation
-----------
run:
$ ./configure
$ make

if the "./configure" script is missing (e.g. because you did a fresh checkout
from SVN, you have to run this _first_:
$ autoconf

finally, install it with:
$ make install
this will install everything needed to /usr/local/lib/pd/extra/syslog;
if you want another prefix, check 
$ ./configure --help
for your options

running
-------
create a [syslog] object and use it like [print]
