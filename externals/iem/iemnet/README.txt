iemnet - networking for Pd
==========================

this is a fork of martin peach's "net" library, that allows low-level
interaction with networks on OSI-layer 5 (transport layer).

for a list of features, see FEATURES.txt

Why fork?
=========

the original library is still actively maintained by martin peach.
however:
- forking allows me to experiment with new features/techniques more easily
- forking allows to remove all the legacy cruft (and not care about
  compatibility now)
- the development mode in the original library would involve the upstream author
  "signing-off" any changes (debatable; but i don't want to submit experimental
  code to their stable code base)

in practice one of the major drawbacks i see in upstream is, that (in the
multithreaded objects), for each message a separate thread is spawned. this
leads to excessive use of system ressources (detaching and joining threads takes
time), easy DoS (each thread uses one in a limited number of thread handles),
and abandons determinism (nobody guarantees that parallel threads are executed
"in order"; thus a message in a later-spawned thread might be delivered to the
socket earlier than older messages - effectively circumventing one of the
promises of TCP/IP: that all packets will reappear in order; users have already
reported this behaviour, which makes using those objects a bit unreliable)

on the long run compatibility with the upstream library is intended.
(though probably not for all the cruft that is in there)


Design:
=======

easy to maintain:
re-used code is bundled in a small "library" (currently only a single file
ienet.c), which is linked statically against the externals.
this library handles all the send/receive stuff (whether it uses threads or not
and if so how, is an implementation detail)
the lib doesn't know anything about the actual transport protocol. it only
interacts with a socket.

easy to run:
think speed, think reliability
all known implementations for pd are either slow or will freeze Pd when under
_heavy_ load. most do both.
iemnet wants to provide objects whih allow you to saturate the network
connection and still keep Pd reactive.
(sidenote: saturating even a 100MBit network with Pd might lead to audio
dropouts; this is not necessarily related to the network but rather to the
amount of data processed by Pd...)

easy to use:
probably not; but at least it has the same (basic) API as mrpeach/net so a
switch should be easy. "basic" means "not everything", so messages for special
workarounds in mrpeach/net (e.g. the block/unblock stuff) are not supported, as
well as debugging features ("dump") and features not related to networking (e.g.
the ability to read a file from harddisk)


Authors:
========
currently iemnet is developed by IOhannes m zmölnig

it (being a fork) is heavily based on code written by Martin Peach, who again
has used code by Olaf Matthes and Miller Puckette


LICENSE:
========
iemnet is published under the GPL.
see LICENSE.txt for more information
