Pd externals by Iain Mott (iain.mott@bigpond.com)

The makefile for this collection is configured for Linux only - Linux  binaries included.

Each external has a demonstration Pd patch (*.pd) associated with it.

Source contents:

polygate~  	- switch between multiple signal inputs - variable fade rate both linear & equal power

shuffle  	- a no-repeat random generator - outputs numbers within a set range

pan~  		- equal power stereo panning

system 		- sends a system message to the console

ln~ 		- natural log + inverse

rec2pol~ 	- convert rectangular coordinates to polar eg. can be used to
		  convert sine & cosine rfft~ output to phase & magnitude
		
pol2rec~	- inverse of rec2pol~

getenv		- Sends value of an environment variable argument on bang. 
		  Use a 'set <NAME>' message to reset the variable name.


two other related patches:

polvoc.pd	- example of pol2rec~ etc

noisegate~.pd	- simple noisegate used in above


See also 'sqlsingle.tar.gz' archive for the 'sqlsingle' eternal. This object allows communication
and data retrieval from a PostgreSQL database in Pd.