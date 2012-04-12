/*
 * Created by SharpDevelop.
 * User: Tebjan Halm
 * Date: 08.04.2012
 * Time: 20:18
 * 
 */

using System;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace LibPDBinding
{

	/// <summary>
	/// Static methods of libpd.dll
	/// </summary>
	public static partial class LibPD
	{
		//only call this once a lifetime
		static LibPD()
		{
			SetupHooks();
			SetupMidiHooks();
			libpd_init();
		}
		

				
		/// Return Type: void
		[DllImport("libpd.dll", EntryPoint="libpd_clear_search_path")]
		private static extern  void clear_search_path() ;
		
		/// <summary>
		/// clears the search path for pd externals
		/// </summary>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void ClearSearchPath()
		{
			clear_search_path();
		}

		
		/// Return Type: void
		///sym: char*
		[DllImport("libpd.dll", EntryPoint="libpd_add_to_search_path")]
		private static extern  void add_to_search_path([In] [MarshalAs(UnmanagedType.LPStr)] string sym) ;
		
		/// <summary>
		/// adds a directory to the search path
		/// </summary>
		/// <param name="sym">directory to add</param>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void AddToSearchPath(string sym)
		{
			add_to_search_path(sym);
		}
		
		
		/// <summary>
		/// same as "compute audio" checkbox in pd gui, or [;pd dsp 0/1(
		/// 
		/// Note: Maintaining a DSP state that's separate from the state of the audio
		/// rendering thread doesn't make much sense in libpd. In most applications,
		/// you probably just want to call {@code computeAudio(true)} at the
		/// beginning and then forget that this method exists.
		/// </summary>
		/// <param name="state"></param>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void ComputeAudio(bool state)
		{
			SendMessage("pd", "dsp", state ? 1 : 0);
		}

		/// Return Type: void*
		///basename: char*
		///dirname: char*
		[DllImport("libpd.dll", EntryPoint="libpd_openfile")]
		public static extern  IntPtr openfile([In] [MarshalAs(UnmanagedType.LPStr)] string basename, [In] [MarshalAs(UnmanagedType.LPStr)] string dirname) ;

		/// Return Type: void
		///p: void*
		[DllImport("libpd.dll", EntryPoint="libpd_closefile")]
		public static extern  void closefile(IntPtr p) ;
		
		/// Return Type: int
		///p: void*
		[DllImport("libpd.dll", EntryPoint="libpd_getdollarzero")]
		public static extern  int getdollarzero(IntPtr p) ;
				
		/// Return Type: int
		///sym: char*
		[DllImport("libpd.dll", EntryPoint="libpd_exists")]
		public static extern  int exists([In] [MarshalAs(UnmanagedType.LPStr)] string sym) ;

		
		
		#region AUDIO
		
		/// Return Type: int
		[DllImport("libpd.dll", EntryPoint="libpd_blocksize")]
		public static extern  int blocksize() ;
		
		
		[DllImport("libpd.dll", EntryPoint="libpd_init_audio")]
		private static extern  int init_audio(int inputChannels, int outputChannels, int sampleRate) ;
		
		/// <summary>
		/// sets up pd audio; must be called before process callback
		/// </summary>
		/// <param name="inputChannels"> </param>
		/// <param name="outputChannels"> </param>
		/// <param name="sampleRate"> </param>
		/// <returns> error code, 0 on success </returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int InitAudio(int inputChannels, int outputChannels, int sampleRate)
		{
			return init_audio(inputChannels, outputChannels, sampleRate);
		}

		/// Return Type: int
		///inBuffer: float*
		///outBuffer: float*
		[DllImport("libpd.dll", EntryPoint="libpd_process_raw")]
		public static extern  int process_raw(ref float inBuffer, ref float outBuffer) ;

		
		/// Return Type: int
		///ticks: int
		///inBuffer: short*
		///outBuffer: short*
		[DllImport("libpd.dll", EntryPoint="libpd_process_short")]
		public static extern  int process_short(int ticks, ref short inBuffer, ref short outBuffer) ;

		
		/// Return Type: int
		///ticks: int
		///inBuffer: float*
		///outBuffer: float*
		[DllImport("libpd.dll", EntryPoint="libpd_process_float")]
		private static extern  int process_float(int ticks, ref float inBuffer, ref float outBuffer) ;

		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int Process(int ticks, ref float inBuffer, ref float outBuffer)
		{
			return process_float(ticks, ref inBuffer, ref outBuffer);
		}
		
		/// Return Type: int
		///ticks: int
		///inBuffer: double*
		///outBuffer: double*
		[DllImport("libpd.dll", EntryPoint="libpd_process_double")]
		public static extern  int process_double(int ticks, ref double inBuffer, ref double outBuffer) ;
		
		#endregion AUDIO

		#region ARRAY
		
		/// Return Type: int
		///name: char*
		[DllImport("libpd.dll", EntryPoint="libpd_arraysize")]
		private static extern  int arraysize([In] [MarshalAs(UnmanagedType.LPStr)] string name) ;
		
		/// <summary>
		/// Get the size of an array
		/// </summary>
		/// <param name="name">Identifier of array</param>
		/// <returns>array size</returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int ArraySize(string name)
		{
			return arraysize(name);
		}
		

		[DllImport("libpd.dll", EntryPoint="libpd_read_array")]
		private static extern  int read_array(ref float dest, [In] [MarshalAs(UnmanagedType.LPStr)] string src, int offset, int n) ;

		/// <summary>
		/// Read from an PD array
		/// </summary>
		/// <param name="dest">Array to fill</param>
		/// <param name="src">Identifier of the PD array to read from</param>
		/// <param name="offset"></param>
		/// <param name="n"></param>
		/// <returns>0 for success</returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int ReadArray(float[] dest, string src, int offset, int n)
		{
			return read_array(ref dest[0], src, offset, n);
		}

		
		[DllImport("libpd.dll", EntryPoint="libpd_write_array")]
		public static extern  int write_array([In] [MarshalAs(UnmanagedType.LPStr)] string dest, int offset, ref float src, int n) ;
		
		/// <summary>
		/// Write to a PD array
		/// </summary>
		/// <param name="dest">Identifier of the PD array to write</param>
		/// <param name="src">Array to read from</param>
		/// <param name="offset"></param>
		/// <param name="n"></param>
		/// <returns>0 for success</returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int WriteArray(string dest, float[] src, int offset, int n)
		{
			return write_array(dest, offset, ref src[0], n);
		}

		#endregion ARRAY
	}
}
