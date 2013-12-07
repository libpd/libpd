this ist ext13, a collection of externals for pd
most of the code comes from other pd-object or externals
and is just modifyed. 
you can use, copy modify, distribute... blahblah
there`s no warranty for anything.


--------compile------------
ext13 is developed and tested in linux.
if you need it for another platform, you have to do the work to port it
to compile it to get a seperate file for each object (.pd_linux):

make clean
make


make dist

will make a tarball of a libdir which can be dropped into place anywhere in
Pd's path.

don`t forget to put the files in your pd-path or do a 
-lib /path/to/ext13
to be able to use it.


----------contact---------
     d13@klingt.org


---------thanks to--------
miller puckete and guenther geiger

