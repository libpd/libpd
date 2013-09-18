Version 0.25
copyright (c) 2002-2004 by Olaf Matthes

pdogg~ is a collection of Ogg/Vorbis externals for Pd (by Miller 
Puckette).

It includes:
- oggamp~   : streaming client
- oggcast~  : streamer (for Icecast2)
- oggread~  : reads files from disk
- oggwrite~ : writes files to disk

To use pdogg start Pd with '-lib path\to\pdogg' flag. 
On Win32 systems Pd 0.35 test 17 or later is necessary to get it working!

To compile pdogg~ you need the Ogg/Vorbis library from 
http://www.vorbis.com/ and under win additionally Pthreads-win32 from
http://sources.redhat.com/pthreads-win32/.
You have to modify the makefile to make it point to the place where the
libraries can be found on your system.


This software is published under LGPL terms.

This is software with ABSOLUTELY NO WARRANTY.
Use it at your OWN RISK. It's possible to damage e.g. hardware or your hearing
due to a bug or for other reasons. 

*****************************************************************************

pdogg~ uses the Ogg/Vorbis library to encode audio data.
The latest version of Ogg/Vorbis can be found at http://www.vorbis.com/

The original version was found at:
http://www.akustische-kunst.de/puredata/

Please report any bugs to olaf.matthes@gmx.de!

*****************************************************************************

oggamp~ Usage:

To run oggamp~ innormal mode, just use [oggamp~] or, to get the buffer status
displayed, use [oggamp~ 1].

Message "connect <host> <mountpoint> <port>" connects to an IceCast2 server.
Note that no response about succesfull connection is send by the server. All
messages in the Pd console window about connection status depend on the ability
to receive data from the server.
Use "connecturl <url>" to use url-like server adresses (like http://host:post/
stream.ogg).

Known bugs and other things:
- Pd halts for a moment when oggamp~ connects to the server. This results in a
  short audio drop out of sound currently played back.
- resampling not jet supported
- playback does not stop on a buffer underrun
- oggamp~ disconnects at end of stream, i.e. it is not possible to play back
  files streamed one after another without manual reconnect

*****************************************************************************

oggcast~ Usage:

Use message "vbr <samplerate> <channels> <quality>" to set the Vorbis
encoding parameters. Resampling is currently not supported, so 'samplerate' 
should be the one Pd is running at. 'channels' specyfies the number of channels 
to stream. This can be set to 2 (default) or 1 which means mono stream taking
the leftmost audio input only. 'quality' can be a value between 0.0 and 1.0 
giving the quality of the stream. 0.4 (default) results in a stream that's 
about 128kbps. 

Since Vorbis uses VBR encoding, bitrates vary depending on the type of audio
signal to be encoded. A pure sine [osc~] results in the smalest stream, com-
plex audio signals will increase this value significantly. To test the maximum 
bitrate that might occur for a quality setting use noise~ as signal input.

Use message "vorbis <samplerate> <channels> <maximum bitrate> <nominal bit-
rate> <minimal bitrate>" to set encoding quality on the basis of bitrates.
When setting all three bitrate parameters to the same value one gets a
constant bitrate stream. Values are in kbps!

Message "connect <host> <mountpoint> <port>" connects to the IceCast2 server.
Note that no response about succesfull connection is send by the server. All
messages in the Pd console window about connection status depend on the ability
to send data to the server.
The mountpoint should end with '.ogg' to indiocate to the player/client that 
it is an Ogg/Vorbis encoded stream.

Use "passwd <passwort>" to set your password (default is 'letmein') and 
"disconnect" to disconnect from the server. "print" prints out the current
Vorbis encoder settings.

To set the comment tags in the Ogg/Vorbis header (which can be displayed by
the receiving client) use message "<NAMEOFTAG> <comment>". Supported tags are:
TITLE, ARTIST, GENRE, PERFORMER, LOCATION, COPYRIGHT, CONTACT, DESCRIPTION and
DATE (which is automatically set to the date/time the broadcast started). To
get spaces use '=' or '_' instead. Note that under Win2k '=' sometimes get lost 
from the patch after saving!!!


Listening to it:

To listen to Ogg/Vorbis encoded livestreams many player need an extra plug-in.
Have a look at http://www.vorbis.com/ to find the appropiate plug-in for your
player.
To play back the stream just open lacation http://<server>:<port>/<mountpoint>.

Note that changing encoding parameters or header comments while oggcast~ is
streaming to the server might result in audible dropouts.
