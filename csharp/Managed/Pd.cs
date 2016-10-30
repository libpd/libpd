using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using LibPDBinding.Native;

namespace LibPDBinding.Managed
{
	/// <summary>
	/// Class for a Pd instance. 
	/// 
	/// NB: Currently only one instance of Pd should be instanciated, but multi-process LibPd is on the way.
	/// </summary>
	public sealed class Pd : IDisposable
	{
		/// <summary>
		/// Gets the number of input channels.
		/// </summary>
		public int Inputs { get; private set; }

		/// <summary>
		/// Gets the number of output channels.
		/// </summary>
		public int Outputs { get; private set; }

		/// <summary>
		/// Gets the sample rate.
		/// </summary>
		public int SampleRate { get; private set; }

		/// <summary>
		/// Gets the block size.
		/// </summary>
		public int BlockSize {
			get { return PInvoke.blocksize (); }
		}

		/// <summary>
		/// Returns [true] when audio computation is enabled, and [false] when audio computation is disabled.
		/// </summary>
		public bool IsComputing { get; private set; }

		/// <summary>
		/// Initializes a new instance of Pd
		/// </summary>
		/// <param name="inputChannels">Number of input channels.</param>
		/// <param name="outputChannels">Number of output channels.</param>
		/// <param name="sampleRate">Sample rate for project.</param>
		public Pd (int inputChannels, int outputChannels, int sampleRate) : this (inputChannels, outputChannels, sampleRate, null)
		{
		}

		/// <summary>
		/// Initializes a new instance of Pd
		/// </summary>
		/// <param name="inputChannels">Number of input channels.</param>
		/// <param name="outputChannels">Number of output channels.</param>
		/// <param name="sampleRate">Sample rate for project.</param>
		/// <param name="searchPaths">Paths for Pd to search for externals.</param>
		public Pd (int inputChannels, int outputChannels, int sampleRate, IEnumerable<string> searchPaths)
		{
			Inputs = inputChannels;
			Outputs = outputChannels;
			SampleRate = sampleRate;
			//SetupHooks();
			PInvoke.libpd_init ();
			foreach (string path in searchPaths ?? Enumerable.Empty<string>()) {
				PInvoke.add_to_search_path (path);
			}
		}

		private void SetupHooks ()
		{
			//PrintHook = new LibPDPrintHook(RaisePrintEvent);
			//PInvoke.set_printhook(PrintHook);

			//BangHook = new LibPDBangHook(RaiseBangEvent);
			//PInvoke.set_banghook(BangHook);

			//FloatHook = new LibPDFloatHook(RaiseFloatEvent);
			//PInvoke.set_floathook(FloatHook);

			//SymbolHook = new LibPDSymbolHook(RaiseSymbolEvent);
			//PInvoke.set_symbolhook(SymbolHook);

			//ListHook = new LibPDListHook(RaiseListEvent);
			//PInvoke.set_listhook(ListHook);

			//MessageHook = new LibPDMessageHook(RaiseMessageEvent);
			//PInvoke.set_messagehook(MessageHook);
		}

		~Pd ()
		{
			Dispose (false);
		}

		public void Dispose ()
		{
			Dispose (true);
			GC.SuppressFinalize (this);
		}

		private void Dispose (bool disposing)
		{
			if (disposing) {
				Stop ();
			}
		}

		/// <summary>
		/// Starts audio computation.
		/// </summary>
		public void Start ()
		{
			PInvoke.init_audio (Inputs, Outputs, SampleRate);
			Messaging.SendMessage ("pd", "dsp", 1);
			IsComputing = true;
		}

		public short[] Process (int ticks, short[] inBuffer)
		{
			short[] outBuffer = new short[Outputs * ticks * BlockSize];
			PInvoke.process_short (ticks, inBuffer, outBuffer);
			return outBuffer;
		}

		/// <summary>
		/// Reads samples from inBuffer and processes them with Pd.
		/// </summary>
		/// <param name="ticks">Number of ticks to process. To reduce overhead of function calls, raise this number. To lower latency, reduce this number.</param>
		/// <param name="inBuffer">Interleaved input buffer. Must be at least of size [InputChannels] * [Blocksize] * [ticks]</param>
		/// <returns>Interleaved processed audio samples.</returns>
		public float[] Process (int ticks, float[] inBuffer)
		{
			float[] outBuffer = new float[Outputs * ticks * BlockSize];
			PInvoke.process_float (ticks, inBuffer, outBuffer);
			return outBuffer;
		}

		public double[] Process (int ticks, double[] inBuffer)
		{
			double[] outBuffer = new double[Outputs * ticks * BlockSize];
			PInvoke.process_double (ticks, inBuffer, outBuffer);
			return outBuffer;
		}

		/// <summary>
		/// Stops audio computation.
		/// </summary>
		public void Stop ()
		{
			Messaging.SendMessage ("pd", "dsp", 0);
			IsComputing = false;
		}

		/// <summary>
		/// Loads a Pd patch from the specified file path.
		/// </summary>
		/// <param name="path">Path to the Pd file.</param>
		/// <returns>New Patch, if loading is successful, else null.</returns>
		public Patch LoadPatch (string path)
		{
			if (!File.Exists (path)) {
				return null;
			}
			var ptr = PInvoke.openfile (Path.GetFileName (path), Path.GetDirectoryName (path));
			return new Patch (ptr);
		}

		public PdArray GetArray (string array)
		{
			return new PdArray (array);
		}
	}
}
