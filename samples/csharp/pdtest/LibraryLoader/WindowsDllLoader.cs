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
