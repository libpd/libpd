using System;
using System.Runtime.InteropServices;

namespace LibPDBinding.Native
{
	static class Messaging
	{
		[DllImport (Defines.DllName, EntryPoint = "libpd_set_printhook", CallingConvention = Defines.CallingConvention)]
		public static extern void set_printhook (LibPDPrintHook hook);

		[DllImport (Defines.DllName, EntryPoint = "libpd_set_banghook", CallingConvention = Defines.CallingConvention)]
		public static extern void set_banghook (LibPDBangHook hook);

		[DllImport (Defines.DllName, EntryPoint = "libpd_set_floathook", CallingConvention = Defines.CallingConvention)]
		public static extern void set_floathook (LibPDFloatHook hook);

		[DllImport (Defines.DllName, EntryPoint = "libpd_set_symbolhook", CallingConvention = Defines.CallingConvention)]
		public static extern void set_symbolhook (LibPDSymbolHook hook);

		[DllImport (Defines.DllName, EntryPoint = "libpd_set_listhook", CallingConvention = Defines.CallingConvention)]
		public static extern void set_listhook (LibPDListHook hook);

		[DllImport (Defines.DllName, EntryPoint = "libpd_set_messagehook", CallingConvention = Defines.CallingConvention)]
		public static extern void set_messagehook (LibPDMessageHook hook);

		[DllImport (Defines.DllName, EntryPoint = "libpd_is_float", CallingConvention = Defines.CallingConvention)]
		public static extern int atom_is_float (IntPtr a);

		[DllImport (Defines.DllName, EntryPoint = "libpd_is_symbol", CallingConvention = Defines.CallingConvention)]
		public static extern int atom_is_symbol (IntPtr a);

		[DllImport (Defines.DllName, EntryPoint = "libpd_get_float", CallingConvention = Defines.CallingConvention)]
		public static extern float atom_get_float (IntPtr a);

		[DllImport (Defines.DllName, EntryPoint = "libpd_get_symbol", CallingConvention = Defines.CallingConvention)]
		public static extern IntPtr atom_get_symbol (IntPtr a);

		[DllImport (Defines.DllName, EntryPoint = "libpd_next_atom", CallingConvention = Defines.CallingConvention)]
		public static extern IntPtr next_atom (IntPtr a);

		[DllImport (Defines.DllName, EntryPoint = "libpd_bind", CallingConvention = Defines.CallingConvention)]
		public static extern IntPtr bind ([In] [MarshalAs (UnmanagedType.LPStr)] string sym);

		[DllImport (Defines.DllName, EntryPoint = "libpd_unbind", CallingConvention = Defines.CallingConvention)]
		public static extern void unbind (IntPtr p);

		[DllImport (Defines.DllName, EntryPoint = "libpd_bang", CallingConvention = Defines.CallingConvention)]
		public static extern int send_bang ([In] [MarshalAs (UnmanagedType.LPStr)] string recv);

		[DllImport (Defines.DllName, EntryPoint = "libpd_float", CallingConvention = Defines.CallingConvention)]
		public static extern int send_float ([In] [MarshalAs (UnmanagedType.LPStr)] string recv, float x);

		[DllImport (Defines.DllName, EntryPoint = "libpd_symbol", CallingConvention = Defines.CallingConvention)]
		public static extern int send_symbol ([In] [MarshalAs (UnmanagedType.LPStr)] string recv, [In] [MarshalAs (UnmanagedType.LPStr)] string sym);

		[DllImport (Defines.DllName, EntryPoint = "libpd_start_message", CallingConvention = Defines.CallingConvention)]
		public static extern int start_message (int max_length);

		[DllImport (Defines.DllName, EntryPoint = "libpd_finish_message", CallingConvention = Defines.CallingConvention)]
		public static extern int finish_message ([In] [MarshalAs (UnmanagedType.LPStr)] string recv, [In] [MarshalAs (UnmanagedType.LPStr)] string msg);

		[DllImport (Defines.DllName, EntryPoint = "libpd_finish_list", CallingConvention = Defines.CallingConvention)]
		public static extern int finish_list ([In] [MarshalAs (UnmanagedType.LPStr)] string recv);

		[DllImport (Defines.DllName, EntryPoint = "libpd_add_float", CallingConvention = Defines.CallingConvention)]
		public static extern void add_float (float x);

		[DllImport (Defines.DllName, EntryPoint = "libpd_add_symbol", CallingConvention = Defines.CallingConvention)]
		public static extern void add_symbol ([In] [MarshalAs (UnmanagedType.LPStr)] string sym);
	}
}