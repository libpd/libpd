
The 'hcs' library is a random grabbag of objects that are experiments that
sometimes lead to full-fledged libraries.

====
TODO
====

- pddate compare object

- make ISOdate and ISOtime accept pddate format

- rename [folder_list] to [file/match] and make other file lib objects
  - make [file/match] handle lists of patterns
  - make [file/match] behave like [qlist]


- [cursor] object
  - based on [MouseState] and/or [gcanvas]
  - inlet to control the mouse cursor icon (pointer, X, text select, etc)


- mDNS objects!


- add pan objects to audio basics library


- create math library
  - standard constants like Pi, etc.
  - standard math functions


- create networking library
  - socket objects
  - multicast DNS objects


- create mapping library
  - move [hid] toolkit objects into mapping lib
  - sort thru Cyrille's ds_ objects
  - objects for smoothing sensor data


- create i/o library
  - add hid, comport, platform-specific hid, libusb, midi
  - convert [hid] to libhid
  - write [directinput]
  - [serial]
  	 - port numbering on darwin: number /dev/cu.* sequentially
  - perhaps specific objects for sensorboxs like the arduino, multio, etc


- create standard gui objects lib (what's the name?)
  - dsp
  - pan
  - amp

- create support lib for creating objects in Pd
  - *_argument objects
  - rename blocksize_in_ms to block_size_in_ms
  - how about "foundation" or "class" or "support"


ifeel.c
------------------------------------------------------------------------
- make default device be /dev/input/ifeel0 when there are no arguments

- add abort command ( http://moore.cx/dan/out/ifeel/
  http://inebriated-innovation.org/ifeel/ )
