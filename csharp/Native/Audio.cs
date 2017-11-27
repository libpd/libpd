using System;
using System.Runtime.InteropServices;

namespace LibPDBinding.Native
{
	static class Audio
	{
		[DllImport (Defines.DllName, EntryPoint = "libpd_blocksize", CallingConvention = Defines.CallingConvention)]
		public static extern int blocksize ();

		[DllImport (Defines.DllName, EntryPoint = "libpd_init_audio", CallingConvention = Defines.CallingConvention)]
		public static extern int init_audio (int inputChannels, int outputChannels, int sampleRate);

		[DllImport (Defines.DllName, EntryPoint = "libpd_process_raw", CallingConvention = Defines.CallingConvention)]
		public static extern int process_raw ([In] float[] inBuffer, [Out] float[] outBuffer);

		[DllImport (Defines.DllName, EntryPoint = "libpd_process_raw", CallingConvention = Defines.CallingConvention)]
		public static extern unsafe  int process_raw (float* inBuffer, float* outBuffer);

		[DllImport (Defines.DllName, EntryPoint = "libpd_process_short", CallingConvention = Defines.CallingConvention)]
		public static extern int process_short (int ticks, [In] short[] inBuffer, [Out] short[] outBuffer);

		[DllImport (Defines.DllName, EntryPoint = "libpd_process_short", CallingConvention = Defines.CallingConvention)]
		public static extern unsafe  int process_short (int ticks, short* inBuffer, short* outBuffer);

		[DllImport (Defines.DllName, EntryPoint = "libpd_process_float", CallingConvention = Defines.CallingConvention)]
		public static extern int process_float (int ticks, [In] float[] inBuffer, [Out] float[] outBuffer);

		[DllImport (Defines.DllName, EntryPoint = "libpd_process_float", CallingConvention = Defines.CallingConvention)]
		public static extern unsafe  int process_float (int ticks, float* inBuffer, float* outBuffer);

		[DllImport (Defines.DllName, EntryPoint = "libpd_process_double", CallingConvention = Defines.CallingConvention)]
		public static extern int process_double (int ticks, [In] double[] inBuffer, [Out] double[] outBuffer);

		[DllImport (Defines.DllName, EntryPoint = "libpd_process_double", CallingConvention = Defines.CallingConvention)]
		public static extern unsafe int process_double (int ticks, double* inBuffer, double* outBuffer);

		[DllImport (Defines.DllName, EntryPoint = "libpd_arraysize", CallingConvention = Defines.CallingConvention)]
		public static extern int arraysize ([In] [MarshalAs (UnmanagedType.LPStr)] string name);

		[DllImport (Defines.DllName, EntryPoint = "libpd_read_array", CallingConvention = Defines.CallingConvention)]
		public static extern int read_array ([Out] float[] dest, [In] [MarshalAs (UnmanagedType.LPStr)] string src, int offset, int n);

		[DllImport (Defines.DllName, EntryPoint = "libpd_read_array", CallingConvention = Defines.CallingConvention)]
		public static extern unsafe  int read_array (float* dest, [In] [MarshalAs (UnmanagedType.LPStr)] string src, int offset, int n);

		[DllImport (Defines.DllName, EntryPoint = "libpd_write_array", CallingConvention = Defines.CallingConvention)]
		public static extern int write_array ([In] [MarshalAs (UnmanagedType.LPStr)] string dest, int offset, [In] float[] src, int n);

		[DllImport (Defines.DllName, EntryPoint = "libpd_write_array", CallingConvention = Defines.CallingConvention)]
		public static extern unsafe int write_array ([In] [MarshalAs (UnmanagedType.LPStr)] string dest, int offset, float* src, int n);
	}
}