
aenv~ is a asymptotic ADSR envelope generator; The output value approaches the
target values as asymptotes.



partconv~ is an external that implements partitioned fast convolution,
suitable for convolving input signals with long impulse responses for reverbs,
etc.  This release offers much improved performance, as well as independence
of the partition size and your patch's blocksize.  It includes Altivec code
from Chris Clepper.  There's also some SSE 1 code that produces almost correct
results but doesn't seem to improve performance.  If you are familiar with SSE
and want to have a go at writing an SSE version, please do!
partconv~ requires FFTW3 (http://fftw.org)



pvoc~ is a phase vocoder based on Pd's 09.pvoc.pd example patch. Advantages over the abstraction include (reportedly) faster execution, instantaneous response to input, and adjustable phase locking. It requires FFTW3. 
bensaylor's Home 



susloop~: sample player with various loop methods (ping-pong, ... ) think
tracker. svf~  This is a signal-controlled port of Steve Harris' state
variable filter LADSPA plugin.



svf~: a signal-controlled port of Steve Harris' state variable filter
LADSPA plugin (http://plugin.org.uk).



zhzhx~: Turns the input signal into a staticky, distorted mess. Comes with tone
control. 



Benjamin R. Saylor <bensaylor@fastmail.fm>
