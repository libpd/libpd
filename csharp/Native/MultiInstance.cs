using System;
using System.Runtime.InteropServices;

namespace LibPDBinding.Native
{
	static class MultiInstance
	{
		[DllImport (Defines.DllName, EntryPoint = "libpd_new_instance", CallingConvention = Defines.CallingConvention)]
		public static extern IntPtr new_instance ();

		[DllImport (Defines.DllName, EntryPoint = "libpd_set_instance", CallingConvention = Defines.CallingConvention)]
		public static extern void set_instance (IntPtr instance);

		[DllImport (Defines.DllName, EntryPoint = "libpd_free_instance", CallingConvention = Defines.CallingConvention)]
		public static extern void free_instance (IntPtr instance);

		[DllImport (Defines.DllName, EntryPoint = "libpd_this_instance", CallingConvention = Defines.CallingConvention)]
		public static extern IntPtr this_instance ();

		[DllImport (Defines.DllName, EntryPoint = "libpd_get_instance", CallingConvention = Defines.CallingConvention)]
		public static extern IntPtr get_instance (int index);

		[DllImport (Defines.DllName, EntryPoint = "libpd_num_instances", CallingConvention = Defines.CallingConvention)]
		public static extern int num_instances ();
	}
}