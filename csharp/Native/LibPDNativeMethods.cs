/*
 * Created by SharpDevelop.
 * User: Tebjan Halm
 * Date: 08.04.2012
 * Time: 20:18
 * 
 */

using System;
using System.Collections.Generic;
using System.IO;
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
		private static Dictionary<int, IntPtr> Patches = new Dictionary<int, IntPtr>();
		
		//only call this once a lifetime
		static LibPD()
		{
			SetupHooks();
			SetupMidiHooks();
			libpd_init();
		}
		
		#region Environment
		
		/// Init PD
		[DllImport("libpd.dll", EntryPoint="libpd_init")]
		private static extern  void libpd_init() ;

				
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
		/// adds a directory to the search paths
		/// </summary>
		/// <param name="sym">directory to add</param>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void AddToSearchPath(string sym)
		{
			add_to_search_path(sym);
		}
		
		/// Return Type: void*
		///basename: char*
		///dirname: char*
		[DllImport("libpd.dll", EntryPoint="libpd_openfile")]
		private static extern  IntPtr openfile([In] [MarshalAs(UnmanagedType.LPStr)] string basename, [In] [MarshalAs(UnmanagedType.LPStr)] string dirname) ;
		
		/// <summary>
		/// reads a patch from a file
		/// </summary>
		/// <param name="path">to the file </param>
		/// <returns> an integer handle that identifies this patch; this handle is the
		///         $0 value of the patch </returns>
		/// <exception cref="IOException">
		///             thrown if the file doesn't exist or can't be opened </exception>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int OpenPatch(string filepath)
		{
			if(!File.Exists(filepath))
			{
				throw new FileNotFoundException(filepath);
			}
			
			var ptr =  openfile(Path.GetFileName(filepath), Path.GetDirectoryName(filepath));
			
			if(ptr == IntPtr.Zero)
			{
				throw new IOException("unable to open patch " + filepath);
			}
			
			var handle = getdollarzero(ptr);
			
			Patches[handle] = ptr;
			
			return handle;
		}

		/// Return Type: void
		///p: void*
		[DllImport("libpd.dll", EntryPoint="libpd_closefile")]
		private static extern  void closefile(IntPtr p) ;
		
		/// <summary>
		/// closes a patch; will do nothing if the handle is invalid
		/// </summary>
		/// <param name="p">$0 of the patch, as returned by OpenPatch</param>
		/// <returns>true if file was found and closed</returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static bool ClosePatch(int p)
		{
			if(!Patches.ContainsKey(p)) return false;
			
			var ptr = Patches[p];
			closefile(ptr);
			return Patches.Remove(p);
			
		}
		
		/// Return Type: int
		///p: void*
		[DllImport("libpd.dll", EntryPoint="libpd_getdollarzero")]
		private static extern  int getdollarzero(IntPtr p) ;

				
		/// Return Type: int
		///sym: char*
		[DllImport("libpd.dll", EntryPoint="libpd_exists")]
		private static extern  int exists([In] [MarshalAs(UnmanagedType.LPStr)] string sym) ;

		/// <summary>
		/// checks whether a symbol represents a pd object
		/// </summary>
		/// <param name="s">String representing pd symbol </param>
		/// <returns> true if and only if the symbol given by s is associated with
		///         something in pd </returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static bool Exists(string sym)
		{
			return exists(sym) != 0;
		}
		
		/// <summary>
		/// releases resources held by native bindings (PdReceiver object and
		/// subscriptions); otherwise, the state of pd will remain unaffected
		/// 
		/// Note: It would be nice to free pd's I/O buffers here, but sys_close_audio
		/// doesn't seem to do that, and so we'll just skip this for now.
		/// </summary>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void Release()
		{
			foreach (var ptr in Bindings.Values)
			{
				unbind(ptr);
			}
			
			Bindings.Clear();
			foreach (var ptr in Patches.Values)
			{
				closefile(ptr);
			}
			Patches.Clear();
		}

		#endregion Environment
		
		#region Audio
		
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
		
		/// Return Type: int
		[DllImport("libpd.dll", EntryPoint="libpd_blocksize")]
		public static extern  int blocksize() ;
		
		/// <summary>
		/// default pd block size, DEFDACBLKSIZE (currently 64) (aka number
		/// of samples per tick per channel)
		/// </summary>
		public static int BlockSize
		{
			[MethodImpl(MethodImplOptions.Synchronized)]
			get
			{
				return blocksize();
			}
		}
		
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
		public static int OpenAudio(int inputChannels, int outputChannels, int sampleRate)
		{
			return init_audio(inputChannels, outputChannels, sampleRate);
		}

		/// Return Type: int
		///inBuffer: float*
		///outBuffer: float*
		[DllImport("libpd.dll", EntryPoint="libpd_process_raw")]
		private static extern  int process_raw(ref float inBuffer, ref float outBuffer) ;

		/// <summary>
		/// raw process callback, processes one pd tick, writes raw data to buffers
		/// without interlacing
		/// </summary>
		/// <param name="inBuffer">
		///            must be an array of the right size, never null; use inBuffer =
		///            new short[0] if no input is desired </param>
		/// <param name="outBuffer">
		///            must be an array of size outBufferSize from openAudio call </param>
		/// <returns> error code, 0 on success </returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int ProcessRaw(ref float inBuffer, ref float outBuffer)
		{
			return process_raw(ref inBuffer, ref outBuffer);
		}

		
		/// Return Type: int
		///ticks: int
		///inBuffer: short*
		///outBuffer: short*
		[DllImport("libpd.dll", EntryPoint="libpd_process_short")]
		private static extern  int process_short(int ticks, ref short inBuffer, ref short outBuffer) ;
		
		/// <summary>
		/// main process callback, reads samples from inBuffer and writes samples to
		/// outBuffer, using arrays of type float
		/// </summary>
		/// <param name="ticks">
		///            the number of Pd ticks (i.e., blocks of 64 frames) to compute </param>
		/// <param name="inBuffer">
		///            must be an array of the right size, never null; use inBuffer =
		///            new short[0] if no input is desired </param>
		/// <param name="outBuffer">
		///            must be an array of size outBufferSize from openAudio call </param>
		/// <returns> error code, 0 on success </returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int Process(int ticks, ref short inBuffer, ref short outBuffer)
		{
			return process_short(ticks, ref inBuffer, ref outBuffer);
		}

		
		/// Return Type: int
		///ticks: int
		///inBuffer: float*
		///outBuffer: float*
		[DllImport("libpd.dll", EntryPoint="libpd_process_float")]
		private static extern  int process_float(int ticks, ref float inBuffer, ref float outBuffer) ;


		/// <summary>
		/// main process callback, reads samples from inBuffer and writes samples to
		/// outBuffer, using arrays of type float
		/// </summary>
		/// <param name="ticks">
		///            the number of Pd ticks (i.e., blocks of 64 frames) to compute </param>
		/// <param name="inBuffer">
		///            must be an array of the right size, never null; use inBuffer =
		///            new short[0] if no input is desired </param>
		/// <param name="outBuffer">
		///            must be an array of size outBufferSize from openAudio call </param>
		/// <returns> error code, 0 on success </returns>
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
		private static extern  int process_double(int ticks, ref double inBuffer, ref double outBuffer) ;
		
		/// <summary>
		/// main process callback, reads samples from inBuffer and writes samples to
		/// outBuffer, using arrays of type float
		/// </summary>
		/// <param name="ticks">
		///            the number of Pd ticks (i.e., blocks of 64 frames) to compute </param>
		/// <param name="inBuffer">
		///            must be an array of the right size, never null; use inBuffer =
		///            new short[0] if no input is desired </param>
		/// <param name="outBuffer">
		///            must be an array of size outBufferSize from openAudio call </param>
		/// <returns> error code, 0 on success </returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int Process(int ticks, ref double inBuffer, ref double outBuffer)
		{
			return process_double(ticks, ref inBuffer, ref outBuffer);
		}
		
		#endregion Audio

		#region Array
		
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
		/// read values from an array in Pd
		/// </summary>
		/// <param name="destination"> float array to write to </param>
		/// <param name="destOffset">  index at which to start writing </param>
		/// <param name="source">      array in Pd to read from </param>
		/// <param name="srcOffset">   index at which to start reading </param>
		/// <param name="n">           number of values to read </param>
		/// <returns>            0 on success, or a negative error code on failure </returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int ReadArray(float[] destination, int destOffset, string source, int srcOffset, int n)
		{
			if (destOffset < 0 || destOffset + n > destination.Length)
			{
				return -2;
			}
			return read_array(ref destination[destOffset], source, srcOffset, n);
		}
		
		[DllImport("libpd.dll", EntryPoint="libpd_write_array")]
		private static extern  int write_array([In] [MarshalAs(UnmanagedType.LPStr)] string dest, int offset, ref float src, int n) ;
		

		/// <summary>
		/// write values to an array in Pd
		/// </summary>
		/// <param name="destination"> name of the array in Pd to write to </param>
		/// <param name="destOffset">  index at which to start writing </param>
		/// <param name="source">      float array to read from </param>
		/// <param name="srcOffset">   index at which to start reading </param>
		/// <param name="n">           number of values to write </param>
		/// <returns>            0 on success, or a negative error code on failure </returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int WriteArray(string destination, int destOffset, float[] source, int srcOffset, int n)
		{
			if (srcOffset < 0 || srcOffset + n > source.Length)
			{
				return -2;
			}
			return write_array(destination, destOffset, ref source[srcOffset], n);
		}
		

		#endregion Array
	}
}
