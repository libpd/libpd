LibPD for .NET and Mono
-----------------------

Thank you for using the NuGet package for LibPD for .NET and Mono.

The managed DLL needs a native shared library, that you must include
in your project. Depending on your platform, choose the correct one
from the folder packages/LibPDBinding/native/UnmanagedLibraries in
your solution or project root. For "Any CPU" build on 64 bit Windows
use the 32 bit version.

Add the correct shared library to your project, for Windows add 
libwinpthread-1.dll as well.

Change the properties for the files after including:
- Set "Build Action" to "None", if not set to this
- Set "Copy to Output Directory" to "Copy if newer"

Currently included binaries for:
- Windows x86, amd64
- Linux x86, amd64

For Windows x64 builds (not Any CPU on 64 bit systems) you must change
the version of the managed dll as well:
- Remove LibPDBinding.dll from your references
- Add LibPDBinding.dll from packages/LibPDBinding/lib/net40/x64

If your platform is not included in the list, you must build it 
yourself from source: https://github.com/libpd/libpd

Bugtracker: https://github.com/libpd/libpd/issues
Questions: http://stackoverflow.com/questions/tagged/libpd
