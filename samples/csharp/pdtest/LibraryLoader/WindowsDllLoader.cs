using System;
using System.Runtime.InteropServices;

namespace LibPDBindingTest.LibraryLoader
{
	class WindowsDllLoader : IDllLoader
	{

		[DllImport ("kernel32")]
		static extern IntPtr LoadLibrary (string lpLibFileName);

		[DllImport ("kernel32")]
		static extern bool FreeLibrary (IntPtr hModule);

		IntPtr IDllLoader.LoadLibrary (string fileName)
		{
			return LoadLibrary (fileName);
		}

		bool IDllLoader.FreeLibrary (IntPtr handle)
		{
			return FreeLibrary (handle);
		}
	}
}
