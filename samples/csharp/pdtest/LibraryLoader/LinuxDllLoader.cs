/*
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 * 
 * Copyright(c) 2016 Thomas Mayer<thomas@residuum.org>
 */
using System;
using System.Runtime.InteropServices;

namespace LibPDBindingTest.LibraryLoader
{
	class LinuxDllLoader : IDllLoader
	{
		public IntPtr LoadLibrary (string fileName)
		{
			return dlopen (fileName, RTLD_NOW);
		}

		public bool FreeLibrary (IntPtr handle)
		{
			if (handle == IntPtr.Zero) {
				return false;
			}
			return dlclose (handle) != 0;
		}

		const int RTLD_NOW = 2;

		[DllImport ("libdl.so")]
		static extern IntPtr dlopen (string fileName, int flags);

		[DllImport ("libdl.so")]
		static extern int dlclose (IntPtr handle);
	}
}
