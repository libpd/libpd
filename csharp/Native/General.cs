using System;
using System.Runtime.InteropServices;

namespace LibPDBinding.Native
{
	static class General
	{
		[DllImport (Defines.DllName, EntryPoint = "libpd_init", CallingConvention = Defines.CallingConvention)]
		public static extern void libpd_init ();

		[DllImport (Defines.DllName, EntryPoint = "libpd_clear_search_path", CallingConvention = Defines.CallingConvention)]
		public static extern void clear_search_path ();

		[DllImport (Defines.DllName, EntryPoint = "libpd_add_to_search_path", CallingConvention = Defines.CallingConvention)]
		public static extern void add_to_search_path ([In] [MarshalAs (UnmanagedType.LPStr)] string sym);

		[DllImport (Defines.DllName, EntryPoint = "libpd_openfile", CallingConvention = Defines.CallingConvention)]
		public static extern IntPtr openfile ([In] [MarshalAs (UnmanagedType.LPStr)] string basename, [In] [MarshalAs (UnmanagedType.LPStr)] string dirname);

		[DllImport (Defines.DllName, EntryPoint = "libpd_closefile", CallingConvention = Defines.CallingConvention)]
		public static extern void closefile (IntPtr p);

		[DllImport (Defines.DllName, EntryPoint = "libpd_getdollarzero", CallingConvention = Defines.CallingConvention)]
		public static extern int getdollarzero (IntPtr p);

		[DllImport (Defines.DllName, EntryPoint = "libpd_exists", CallingConvention = Defines.CallingConvention)]
		public static extern int exists ([In] [MarshalAs (UnmanagedType.LPStr)] string sym);
	}
}