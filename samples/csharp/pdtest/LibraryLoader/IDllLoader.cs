/*
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 * 
 * Copyright(c) 2016 Thomas Mayer<thomas@residuum.org>
 */
using System;

namespace LibPDBindingTest.LibraryLoader
{
	interface IDllLoader
	{
		IntPtr LoadLibrary (string fileName);

		bool FreeLibrary (IntPtr handle);
	}
}
