
This is a simple sample Universal iPad/iPhone app using PdLib to load and run a PD patch file.

The PdLib source and documentation is available at
http://gitorious.org/pdlib

The project is currently setup to use iOS 4.1 as its "Base SDK" and iOS 3.0 as its "iOS Deployment Target". 
That means the app will run on any device with iOS 3.0 or later. 
(These can be changed in project file's and Targets->PdTest01 GetInfo panels.)

PdLib is currently directly compiled into the app project. (See the pdlib folder)

It may make more sense to compile pdlib as a static library. Then link that into the app project.

One thing that is very important to get pdlib to compile is setting the correct compiler DEFINES. 
These can be done in the project build "Other C Flags" setting. 
   -DPD
   -DUSA_API_DUMMY
   -DHAVE_LIBDL
   -DHAVE_UNISTD_H
ALternatively you could just add #define's to the project precompiled header file. e.g. PdTest01_Prefix.pch

The PdAudio and PdBase classes provide the Objective C glue to pdlib. 

PdAudio initializes the iOS Audio Session.

A PdAudio object needs to created by your app. 
e.g.
   	pdAudio = [[PdAudio alloc] initWithSampleRate:44100.0 andTicksPerBuffer:64 andNumberOfInputChannels:2 andNumberOfOutputChannels:2];

PdBase provides an interface to PdLib through + class methods. 
These methods are analogous the PdLib Java API http://gitorious.org/pdlib/pages/Libpd

PdBase shouldn't be explicitly instantiated as an object. 
Just make sure you add the class to your app project by adding the source files.
Then call the class methods like C functions.
e.g.
	[PdBase openPatch: documentsPatchFilePath];
	[PdBase computeAudio:YES];

You then you also need to play [pdAudio play];

Tested systems:
- iPad (iOS 4.2b3)
- iPhone 3GS (iOS 4.0)
- iPod touch 2nd gen (iOS 4.1) - not working
- iPod touch 1st gen (iOS 3.0) - audio problems
- iPhone EDGE (iOS 3.0) - audio problmes.

Known problems:
- Audio playback problems with iOS SDK Simulator, iPod touch 1st gen, iPhone EDGE.
- No audio playback on iPod touch 2nd gen.

--------------- License ------------

This software is copyrighted by Miller Puckette, Reality Jockey, Peter Brinkmann 
and others.  
The following terms (the "Standard Improved BSD License") apply to all files 
associated with the software unless explicitly disclaimed in individual files:

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above  
   copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided
   with the distribution.
3. The name of the author may not be used to endorse or promote
   products derived from this software without specific prior 
   written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,   
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.
