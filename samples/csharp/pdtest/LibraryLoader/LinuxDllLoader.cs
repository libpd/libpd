using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace LibPDBindingTest.LibraryLoader
{
	class LinuxDllLoader : IDllLoader
	{
		public IntPtr LoadLibrary (string fileName)
		{
			return dlopen (fileName, RTLD_LOCAL);
		}

		public bool FreeLibrary (IntPtr handle)
		{
			if (handle == IntPtr.Zero) {
				return false;
			}
			return dlclose (handle) != 0;
		}

		const int RTLD_NOW = 2;
		const int RTLD_LOCAL = 4;

		[DllImport ("libdl.so")]
		static extern IntPtr dlopen (string fileName, int flags);

		[DllImport ("libdl.so")]
		static extern int dlclose (IntPtr handle);
	}
}
