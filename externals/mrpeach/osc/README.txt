OpenSoundControl (OSC) for Pd
=============================

a collection of Pd objectclasses for OSC-messages.
these objects only convert between Pd-messages and OSC-messages (binary format),
so you will need a separate set of objects that implement the transport
(OSI-Layer 4), for instance [udpsend]/[udpreceive] for sending OSC over UDP.

Author: Martin Peach

objectclasses

- [packOSC]
	convert a Pd-message to an OSC (binary) message
	(useful if you want to transmit OSC over UDP or other protocols that
	have the concept of variable length packets)

- [unpackOSC]
	convert an OSC (binary) message to a Pd-message
	(useful if you want to transmit OSC over UDP or other protocols that
	have the concept of variable length packets)

- [routeOSC]
	route OSC-like Pd-messages according to the first element in the path

- [pipelist]
	delay lists 
	(useful if you want to respect timestamps)

- [packOSCstream]
	convert a Pd-message to an OSC (binary) message suitable for streaming transport
	(useful if you want to transmit OSC over TCP/IP or a serial line)
	
- [unpackOSCstream]
	convert an OSC (binary) message suitable for streaming transport to a Pd-message
	(useful if you want to transmit OSC over TCP/IP or a serial line)


