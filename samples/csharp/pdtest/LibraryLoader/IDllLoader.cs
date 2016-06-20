using System;

namespace LibPDBindingTest.LibraryLoader
{
	interface IDllLoader
	{
		IntPtr LoadLibrary (string fileName);

		bool FreeLibrary (IntPtr handle);
	}
}
