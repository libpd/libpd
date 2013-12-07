    -----------------------------------------
    - SMLib : Signal processing for Mapping -
    -----------------------------------------

							v0.12 date 20021125

1. Introduction
---------------

SMLib is an external objects library for PD (pure data). It complements PD
with a set of objects for vector processing, vector analysis, vector
synthesis, number stream analysis, number stream filters.

I was missing objects that could do dsp-like operations on control signals.
PD is event-driven by nature (except the dsp-objects and [delay], [metro],
...). A slider that does not change does not transmit anything.  Event-driven
events can be sampled by eg. using a [float] with a [metro] on its main input,
and a slider on its right inlet. SMLib has objects designed to process such
streams.  Eg. [lavg] is a leaky integrator, it smoothes abrupt changes. Leaky
processes are interesting in mapping because our attention is also leaky.
Eg. [hp] is a high-pass filter (just like [hp~]), and it only passes changes,
and decays to zero when there are no changes. Likewise our attention shifts
towards remarkable changes in our environment.  You can detect 'events' in
float streams with [threshold].  A vector is a list of floats.

These objects are suitable for gesture and high-level music analysis
prototyping.

This library is written by Johannes Taelman (johannes.taelman@rug.ac.be). The
code is free+open (GNU GPL license).  I have only tested/compiled the code on
win32, but porting to linux or macOs should be easy.

2. List of objects
------------------

processing stream of floats (context) (float output)

  lavg        leaky average
  lmax        leaky maximum
  lmin        leaky minimum
  lrange      leaky range
  lstd        leaky standard deviation
  decimator   passes 1 in n input values
  threshold   detection with hysteresis
  hip         first order high-pass filter
  bp          second order (resonant) high-pass filter

analyzing stream of floats (vector output)

  hist        histogram
  lhist       leaky histogram, clips samples
  lhisti      leaky histogram, ignore samples
                outside bins
  itov        bin index to value (for the histograms
  prevl       previous floats in a list
  deltas      difference between last float and
                previous floats

immediate vector analysis (float output)

  vsum        sum of vector elements
  vcog        center of gravity
  vmax        maximum and its location
  vmin        minimum and its location
  vrms        root mean square
  vstd        standard deviation

vector processors (vector output)

  vv+         vector addition
  vv-         vector substraction
  vvconv      vector convolution
  vclip       clip elements
  vfmod       floating point modulo
  vpow        power
  vthreshold  detections with hysteresises

unit conversions on vectors

  vftom       frequency to midi
  vmtof       midi to frequency
  vdbtorms    dB to rms
  vrmstodb    rms to dB

vector synthesis (vector output)

  linspace    linearly spaced vector

vector stream processing (vector output) (context)

  vlavg       leaky averages
  vlmax       leaky maxima
  vlmin       leaky minima
  vlrange     leaky ranges
  vdelta      difference between successive vectors


3. Installation
---------------

Requires PD on the win32 platform (for now). Put SMLib.dll in a directory in
your PD path. Put the help patches in pd\doc\5.reference.  

Start PD with "-lib SMLib"

4. Getting started
------------------

Start PD. 

Create a new patch.

Create an object [SMLib]

Right-click on this object after creation. Choose help.

This gives you the main SMLib patch with all objects and a brief explanation
of them. Choose help for the objects to get details.  Have a look at the
example patches for design ideas.

5. References
-------------

PD headquarters:
http://crca.ucsd.edu/~msp/software.html
http://iem.kug.ac.at/pd/
http://www.puredata.info
