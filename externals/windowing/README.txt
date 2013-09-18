Provides:
  hammimng~, hanning~, blackman~, cosine~, connes~, bartlett~,
  welch~, lanczos~, gaussian~, and kaiser~

Usage:
  Windows inlet signal on each DSP block and sends to outlet
  
  gaussian~ takes one optional argument (standard deviation)
            May be set by float in left inlet

  kaiser~ takes one option argument (alpha)
	    May be set by float in left inlet

  See windowing.pd

To build:
  Linux:   make windowing_linux
  Windows: nmake windowing_nt
