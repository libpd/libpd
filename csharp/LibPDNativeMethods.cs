/*
 * 
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 * 
 * 
 * Created by SharpDevelop.
 * User: Tebjan Halm
 * Date: 08.04.2012
 * Time: 20:18
 * 
 */

using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using LibPDBinding.Native;

namespace LibPDBinding
{

	/// <summary>
	/// LibPD provides basic C# bindings for pd. It follows the libpd Java bingings as good as possible.
	/// 
	/// Some random notes from Peter Brinkmann on the java bindings:
	/// 
	/// - This is a low-level library that aims to leave most design decisions to
	/// higher-level code. In particular, it will throw no exceptions (except for the
	/// methods for opening files, which may throw an <seealso cref="IOException"/> when appropriate). 
	/// At the same time, it is designed to be
	/// fairly robust in that it is thread-safe and does as much error checking as I
	/// find reasonable at this level. Client code is still responsible for proper
	/// dimensioning of buffers and such, though.
	/// 
	/// - The MIDI methods choose sanity over consistency with pd or the MIDI
	/// standard. To wit, channel numbers always start at 0, and pitch bend values
	/// are centered at 0, i.e., they range from -8192 to 8191.
	/// 
	/// - The basic idea is to turn pd into a library that essentially offers a
	/// rendering callback (process) mimicking the design of JACK, the JACK Audio
	/// Connection Kit.
	/// 
	/// - The release method is mostly there as a reminder that some sort of cleanup
	/// might be necessary; for the time being, it only releases the resources held
	/// by the print handler, closes all patches, and cancels all subscriptions.
	/// Shutting down pd itself wouldn't make sense because it might be needed in the
	/// future, at which point the native library may not be reloaded.
	/// 
	///  - I'm a little fuzzy on how/when to use sys_lock, sys_unlock, etc., and so I
	/// decided to handle all synchronization on the Java side. It appears that
	/// sys_lock is for top-level locking in scheduling routines only, and so
	/// Java-side sync conveys the same benefits without the risk of deadlocks.
	/// 
	/// 
	/// Author: Tebjan Halm (tebjan@vvvv.org)
	/// 
	/// </summary>
	[Obsolete("Use the new class based API instead and initialize LibPDBinding.Managed.Pd")]
	public static partial class LibPD
	{
		//only call this once
		static LibPD ()
		{
			Init ();
		}

		#region Environment
		static void Init ()
		{
			SetupHooks ();
			General.libpd_init ();
		}

		/// <summary>
		/// You almost never have to call this! The only case is when the libpdcsharp.dll 
		/// was unloaded and you load it again into your application.
		/// So be careful, it will also call Release() to clear all state.
		/// The first initialization is done automatically when using a LibPD method.
		/// </summary>
		[MethodImpl (MethodImplOptions.Synchronized)] 		
		[Obsolete("Use the new class based API instead and initialize a instance of LibPDBinding.Managed.Pd")]
		public static void ReInit ()
		{
			Release ();
			Init ();
		}
		
		//store open patches
		private static Dictionary<int, IntPtr> Patches = new Dictionary<int, IntPtr> ();


		/// <summary>
		/// clears the search path for pd externals
		/// </summary>
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static void ClearSearchPath ()
		{
			General.clear_search_path ();
		}


		/// <summary>
		/// adds a directory to the search paths
		/// </summary>
		/// <param name="sym">directory to add</param>
		[MethodImpl (MethodImplOptions.Synchronized)]
		[Obsolete("Use searchPaths parameter in LibPDBinding.Managed.Pd()")]
		public static void AddToSearchPath (string sym)
		{
			General.add_to_search_path (sym);
		}

		/// <summary>
		/// reads a patch from a file
		/// </summary>
		/// <param name="path">to the file </param>
		/// <returns> an integer handle that identifies this patch; this handle is the
		///         $0 value of the patch </returns>
		/// <exception cref="IOException">
		///             thrown if the file doesn't exist or can't be opened </exception>
		[MethodImpl (MethodImplOptions.Synchronized)]
		[Obsolete("Use LibPDBinding.Managed.Pd.LoadPatch()")]
		public static int OpenPatch (string filepath)
		{
			if (!File.Exists (filepath)) {
				throw new FileNotFoundException (filepath);
			}
			
			var ptr = General.openfile (Path.GetFileName (filepath), Path.GetDirectoryName (filepath));
			
			if (ptr == IntPtr.Zero) {
				throw new IOException ("unable to open patch " + filepath);
			}
			
			var handle = General.getdollarzero (ptr);
			Patches [handle] = ptr;
			return handle;
		}

		/// <summary>
		/// closes a patch; will do nothing if the handle is invalid
		/// </summary>
		/// <param name="p">$0 of the patch, as returned by OpenPatch</param>
		/// <returns>true if file was found and closed</returns>,		
		[Obsolete("Use LibPDBinding.Managed.Patch.Dispose()")]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static bool ClosePatch (int p)
		{
			if (!Patches.ContainsKey (p))
				return false;
			var ptr = Patches [p];
			General.closefile (ptr);
			return Patches.Remove (p);
		}


		/// <summary>
		/// checks whether a symbol represents a pd object
		/// </summary>
		/// <param name="s">String representing pd symbol </param>
		/// <returns> true if and only if the symbol given by s is associated with
		///         something in pd </returns>
		[Obsolete]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static bool Exists (string sym)
		{
			return General.exists (sym) != 0;
		}

		/// <summary>
		/// releases resources held by native bindings (PdReceiver object and
		/// subscriptions); otherwise, the state of pd will remain unaffected
		/// 
		/// Note: It would be nice to free pd's I/O buffers here, but sys_close_audio
		/// doesn't seem to do that, and so we'll just skip this for now.
		/// </summary>
		[Obsolete("Use LibPDBinding.Managed.Pd.Dispose()")]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static void Release ()
		{
			ComputeAudio (false);
			
			foreach (var ptr in Bindings.Values) {
				Messaging.unbind (ptr);
			}
			Bindings.Clear ();
			
			foreach (var ptr in Patches.Values) {
				General.closefile (ptr);
			}
			Patches.Clear ();
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
		[Obsolete("Use LibPDBinding.Managed.Pd.Start() and LibPDBinding.Managed.Pd.Stop()")]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static void ComputeAudio (bool state)
		{
			SendMessage ("pd", "dsp", state ? 1 : 0);
		}

		/// <summary>
		/// default pd block size, DEFDACBLKSIZE (currently 64) (aka number
		/// of samples per tick per channel)
		/// </summary>
		[Obsolete("Use LibPDBinding.Managed.Pd.BlockSize")]
		public static int BlockSize {
			[MethodImpl(MethodImplOptions.Synchronized)]
			get {
				return Audio.blocksize ();
			}
		}

		/// <summary>
		/// sets up pd audio; must be called before process callback
		/// </summary>
		/// <param name="inputChannels"> </param>
		/// <param name="outputChannels"> </param>
		/// <param name="sampleRate"> </param>
		/// <returns> error code, 0 on success </returns>
		[Obsolete("Use parameters inputChannels, outputChannels. and sampleRate in LibPDBinding.Managed.Pd()")]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static int OpenAudio (int inputChannels, int outputChannels, int sampleRate)
		{
			return Audio.init_audio (inputChannels, outputChannels, sampleRate);
		}

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
		[Obsolete]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static int ProcessRaw (float[] inBuffer, float[] outBuffer)
		{
			return Audio.process_raw (inBuffer, outBuffer);
		}

		/// <summary>
		/// raw process callback, processes one pd tick, writes raw data to buffers
		/// without interlacing. use this method if you have a pointer to the local memory or raw byte arrays in the right format.
		/// you need to pin the memory yourself with a fixed{} code block. it also allows an offset using
		/// pointer arithmetic like: &myMemory[offset]
		/// </summary>
		/// <param name="inBuffer">
		///            pointer to an array of the right size, never null; use inBuffer =
		///            new short[0] if no input is desired </param>
		/// <param name="outBuffer">
		///            pointer to an array of size outBufferSize from openAudio call </param>
		/// <returns> error code, 0 on success </returns>
		[Obsolete]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static unsafe int ProcessRaw (float* inBuffer, float* outBuffer)
		{
			return Audio.process_raw (inBuffer, outBuffer);
		}

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
		[Obsolete("Use LibPDBinding.Managed.Pd.Process()")]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static int Process (int ticks, short[] inBuffer, short[] outBuffer)
		{
			return Audio.process_short (ticks, inBuffer, outBuffer);
		}

		/// <summary>
		/// main process callback, reads samples from inBuffer and writes samples to
		/// outBuffer, using arrays of type float. use this method if you have a pointer to the local memory or raw byte arrays in the right format.
		/// you need to pin the memory yourself with a fixed{} code block. it also allows an offset using
		/// pointer arithmetic like: &myMemory[offset]
		/// </summary>
		/// <param name="ticks">
		///            the number of Pd ticks (i.e., blocks of 64 frames) to compute </param>
		/// <param name="inBuffer">
		///            pointer to an array of the right size, never null; use inBuffer =
		///            new short[0] if no input is desired </param>
		/// <param name="outBuffer">
		///            pointer to an array of size outBufferSize from openAudio call </param>
		/// <returns> error code, 0 on success </returns>
		[Obsolete("Use LibPDBinding.Managed.Pd.Process()")]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static unsafe int Process (int ticks, short* inBuffer, short* outBuffer)
		{
			return Audio.process_short (ticks, inBuffer, outBuffer);
		}


		/// <summary>
		/// main process callback, reads samples from inBuffer and writes samples to
		/// outBuffer, using arrays of type float
		/// </summary>
		/// <param name="ticks">
		///            the number of Pd ticks (i.e., blocks of 64 frames) to compute </param>
		/// <param name="inBuffer">
		///            must be an array of the right size, never null; use inBuffer =
		///            new float[0] if no input is desired </param>
		/// <param name="outBuffer">
		///            must be an array of size outBufferSize from openAudio call </param>
		/// <returns> error code, 0 on success </returns>
		[Obsolete("Use LibPDBinding.Managed.Pd.Process()")]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static int Process (int ticks, float[] inBuffer, float[] outBuffer)
		{
			return Audio.process_float (ticks, inBuffer, outBuffer);
		}

		/// <summary>
		/// main process callback, reads samples from inBuffer and writes samples to
		/// outBuffer, using arrays of type float. use this method if you have a pointer to the local memory or raw byte arrays in the right format.
		/// you need to pin the memory yourself with a fixed{} code block. it also allows an offset using
		/// pointer arithmetic like: &myMemory[offset]
		/// </summary>
		/// <param name="ticks">
		///            the number of Pd ticks (i.e., blocks of 64 frames) to compute </param>
		/// <param name="inBuffer">
		///            pointer to an array of the right size, never null; use inBuffer =
		///            new float[0] if no input is desired </param>
		/// <param name="outBuffer">
		///            pointer to an array of size outBufferSize from openAudio call </param>
		/// <returns> error code, 0 on success </returns>
		[Obsolete("Use LibPDBinding.Managed.Pd.Process()")]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static unsafe int Process (int ticks, float* inBuffer, float* outBuffer)
		{
			return Audio.process_float (ticks, inBuffer, outBuffer);
		}

		/// <summary>
		/// main process callback, reads samples from inBuffer and writes samples to
		/// outBuffer, using arrays of type float
		/// </summary>
		/// <param name="ticks">
		///            the number of Pd ticks (i.e., blocks of 64 frames) to compute </param>
		/// <param name="inBuffer">
		///            must be an array of the right size, never null; use inBuffer =
		///            new double[0] if no input is desired </param>
		/// <param name="outBuffer">
		///            must be an array of size outBufferSize from openAudio call </param>
		/// <returns> error code, 0 on success </returns>
		[Obsolete("Use LibPDBinding.Managed.Pd.Process()")]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static int Process (int ticks, double[] inBuffer, double[] outBuffer)
		{
			return Audio.process_double (ticks, inBuffer, outBuffer);
		}

		/// <summary>
		/// main process callback, reads samples from inBuffer and writes samples to
		/// outBuffer, using arrays of type double. use this method if you have a pointer to the local memory or raw byte arrays in the right format.
		/// you need to pin the memory yourself with a fixed{} code block. it also allows an offset using
		/// pointer arithmetic like: &myMemory[offset]
		/// </summary>
		/// <param name="ticks">
		///            the number of Pd ticks (i.e., blocks of 64 frames) to compute </param>
		/// <param name="inBuffer">
		///            pointer to an array of the right size, never null; use inBuffer =
		///            new double[0] if no input is desired </param>
		/// <param name="outBuffer">
		///            pointer an array of size outBufferSize from openAudio call </param>
		/// <returns> error code, 0 on success </returns>
		[Obsolete("Use LibPDBinding.Managed.Pd.Process()")]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static unsafe int Process (int ticks, double* inBuffer, double* outBuffer)
		{
			return Audio.process_double (ticks, inBuffer, outBuffer);
		}

		#endregion Audio

		#region Array

		/// <summary>
		/// Get the size of an array
		/// </summary>
		/// <param name="name">Identifier of array</param>
		/// <returns>array size</returns>
		[Obsolete("Use LibPDBinding.Managed.PdArray.Size")]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static int ArraySize (string name)
		{
			return Audio.arraysize (name);
		}


		/// <summary>
		/// read values from an array in Pd. if you need an offset use the pointer method and use pointer arithmetic.
		/// </summary>
		/// <param name="destination"> float array to write to </param>
		/// <param name="source">      array in Pd to read from </param>
		/// <param name="srcOffset">   index at which to start reading </param>
		/// <param name="n">           number of values to read </param>
		/// <returns>            0 on success, or a negative error code on failure </returns>
		[Obsolete("Use LibPDBinding.Managed.PdArray.Read()")]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static int ReadArray (float[] destination, string source, int srcOffset, int n)
		{
			if (n > destination.Length) {
				return -2;
			}
			
			return Audio.read_array (destination, source, srcOffset, n);
		}


		/// <summary>
		/// read values from an array in Pd. use this method if you have a pointer to the local memory.
		/// you need to pin the memory yourself with a fixed{} code block. it also allows an offset using
		/// pointer arithmetic like: &myDestinationMemory[offset]
		/// </summary>
		/// <param name="destination"> pointer to float array to write to </param>
		/// <param name="source">      array in Pd to read from </param>
		/// <param name="srcOffset">   index at which to start reading </param>
		/// <param name="n">           number of values to read </param>
		/// <returns>            0 on success, or a negative error code on failure </returns>
		[Obsolete("Use LibPDBinding.Managed.PdArray.Read()")]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static unsafe int ReadArray (float* destination, string source, int srcOffset, int n)
		{
			return Audio.read_array (destination, source, srcOffset, n);
		}

		/// <summary>
		/// write values to an array in Pd. if you need an offset use the pointer method and use pointer arithmetic.
		/// </summary>
		/// <param name="destination"> name of the array in Pd to write to </param>
		/// <param name="destOffset">  index at which to start writing </param>
		/// <param name="source">      float array to read from </param>
		/// <param name="n">           number of values to write </param>
		/// <returns>            0 on success, or a negative error code on failure </returns>
		[Obsolete("Use LibPDBinding.Managed.PdArray.Write()")]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static int WriteArray (string destination, int destOffset, float[] source, int n)
		{
			if (n > source.Length) {
				return -2;
			}
			
			return Audio.write_array (destination, destOffset, source, n);
		}

		/// <summary>
		/// write values to an array in Pd. use this method if you have a pointer to the local memory.
		/// you need to pin the memory yourself with a fixed{} code block. it also allows an offset using
		/// pointer arithmetic like: &mySourceMemory[offset]
		/// </summary>
		/// <param name="destination"> name of the array in Pd to write to </param>
		/// <param name="destOffset">  index at which to start writing </param>
		/// <param name="source"> pointer to a float array to read from </param>
		/// <param name="n">         number of values to write </param>
		/// <returns>            0 on success, or a negative error code on failure </returns>
		[Obsolete("Use LibPDBinding.Managed.PdArray.Write()")]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static unsafe int WriteArray (string destination, int destOffset, float* source, int n)
		{
			return Audio.write_array (destination, destOffset, source, n);
		}

		#endregion Array
	}
}
