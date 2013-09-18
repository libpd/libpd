http://suita.chopin.edu.pl/~czaja/miXed/externs/cyclone.html

-------
cyclone
-------

Cyclone is a library of PureData classes, bringing some level of compatibility
between Max/MSP and Pd environments.  Although being itself in the early stage
of development, it is meant to eventually become part of a much larger
project, aiming at unification and standardization of computer musician's
tools.

In its current form, cyclone is mainly for people using both Max and Pd, and
thus wanting to develop cross-platform patches.  In this respect, cyclone has
much in common with Thomas Grill's flext, and flext-based externals.  See
Thomas' page.  While flext enables developing new cross-platform classes,
cyclone makes existing classes cross-platform.

Cyclone also comes handy, somewhat, in the task of importing Max/MSP patches
into Pd.  Do not expect miracles, though, it is usually not an easy task.

The entire cyclone library, which might be preloaded with either -lib cyclone
or -lib maxmode option, consists of:

 * the main hammer and sickle sub-libraries, containing Pd versions of,
   respectively, Max and MSP classes;

 * cyclone sub-library, taking care of loading hammer and sickle, and which
   itself contains: a small set of operators (!-, !/, ==~, !=~, <~, <=~, >~,
   >=~, !-~, !/~, %~, +=~); an importing mechanism provided by the cyclone
   class.

 * optional dummies sub-library, which contains a large set of dummy classes,
   serving as substitutions for Max/MSP classes not (yet) implemented in
   cyclone;

 * maxmode sub-library, which imposes strict compatibility mode, and loads all
   the other components, including dummies.

The two main sub-libraries might be loaded separately, by using -lib hammer
and/or -lib sickle options.  There is also a possibility of loading any single
class from hammer or sickle library dynamically (this feature is only
available in the linux snapshot).

Currently, the hammer part contains: accum, acos, active, anal, Append (more
info), asin, bangbang, bondo, Borax, Bucket, buddy, capture, cartopol, Clip,
coll, comment, cosh, counter, cycle, decide, Decode, drunk, flush, forward,
fromsymbol, funbuff, funnel, gate, grab, Histo, iter, match, maximum, mean,
midiflush, midiformat, midiparse, minimum, mousefilter, MouseState, mtr (more
info), next, offer, onebang, past, Peak, poltocar, prepend (more info), prob,
pv, seq (more info), sinh, speedlim, spell, split, spray, sprintf, substitute,
sustain, switch, Table (more info), tanh, thresh, TogEdge, tosymbol, Trough,
universal, urn, Uzi, xbendin, xbendout, xnotein, xnoteout, and zl.

The sickle part contains: abs~, acos~, acosh~, allpass~, asin~, asinh~, atan~,
atan2~, atanh~, average~, avg~, bitand~, bitnot~, bitor~, bitshift~, bitxor~,
buffir~, capture~, cartopol~, change~, click~, Clip~, comb~, cosh~, cosx~,
count~, curve~, cycle~, delay~, delta~, deltaclip~, edge~, frameaccum~,
framedelta~, index~, kink~, Line~, linedrive, log~, lookup~, lores~, matrix~
(more info), maximum~, minimum~, minmax~, mstosamps~, onepole~, peakamp~,
peek~, phasewrap~, pink~, play~, poke~, poltocar~, pong~, pow~, rampsmooth~,
rand~, record~, reson~, sah~, sampstoms~, Scope~, sinh~, sinx~, slide~,
Snapshot~, spike~, svf~, tanh~, tanx~, train~, trapezoid~, triangle~,
vectral~, wave~, and zerox~.

Cyclone comes without any documentation.  All the included patches were
created merely for testing.

Caveats:

* The binaries provided in this snapshot release are not expected to run
  inside of a pre-0.36 version of Pd, without prior recompiling.

* If a single -lib cyclone startup option is used, cyclone in turn loads its
  two main components: hammer and sickle.  If a single -lib maxmode startup
  option is used, all the remaining library components are going to be loaded:
  cyclone, hammer, sickle, and dummies.  In these cases, all the required
  libraries should be accessible by Pd.
