# LibPD for .NET and Mono

Thank you for using the NuGet package for LibPD for .NET and Mono.

The managed DLL needs a native shared library.

Currently included binaries for:
- Windows x86, amd64
- Linux x86, amd64, arm32, arm64

If your platform is not included in the list, you must build it 
yourself from source: https://github.com/libpd/libpd

Change the properties for the files after including:
- Set "Build Action" to "None", if not set to this
- Set "Copy to Output Directory" to "Copy if newer"

Bugtracker: https://github.com/libpd/libpd/issues
Questions: http://stackoverflow.com/questions/tagged/libpd
