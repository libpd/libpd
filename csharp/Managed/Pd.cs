using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using LibPDBinding.Managed.Data;
using LibPDBinding.Managed.Events;
using LibPDBinding.Managed.Utils;
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
			get { return Audio.blocksize (); }
		}

		Messaging _messaging;

		/// <summary>
		/// Gets the messaging object.
		/// </summary>
		/// <value>The messaging object.</value>
		public Messaging Messaging {
			get {
				return _messaging;
			}
		}

		Midi _midi;

		/// <summary>
		/// Get the object for MIDI communication
		/// </summary>
		public Midi Midi {
			get { return _midi; }
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
			_messaging = new Messaging ();
			_midi = new Midi ();
			Inputs = inputChannels;
			Outputs = outputChannels;
			SampleRate = sampleRate;
			General.libpd_init ();
			foreach (string path in searchPaths ?? Enumerable.Empty<string>()) {
				General.add_to_search_path (path);
			}
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
			Messaging.Dispose ();
			Midi.Dispose ();
		}

		/// <summary>
		/// Starts audio computation.
		/// </summary>
		public void Start ()
		{
			Audio.init_audio (Inputs, Outputs, SampleRate);
			MessageInvocation.SendMessage ("pd", "dsp", new Float (1));
			IsComputing = true;
		}

		public bool Process (int ticks, short[] inBuffer, short[] outBuffer)
		{
			return Audio.process_short (ticks, inBuffer, outBuffer) == 0;
		}

		/// <summary>
		/// Reads samples from inBuffer and processes them with Pd.
		/// </summary>
		/// <param name="ticks">Number of ticks to process. To reduce overhead of function calls, raise this number. To lower latency, reduce this number.</param>
		/// <param name="inBuffer">Interleaved input buffer. Must be at least of size [InputChannels] * [Blocksize] * [ticks]</param>
		/// <param name="outBuffer">Output buffer for storing interleaved processed audio. Must be at least of size [OutputChannels] * [Blocksize] * [ticks]</param>
		/// <returns>[true] if audio processing was successful, [false] otherwise.</returns>
		public bool Process (int ticks, float[] inBuffer, float[] outBuffer)
		{
			return Audio.process_float (ticks, inBuffer, outBuffer) == 0;
		}

		public bool Process (int ticks, double[] inBuffer, double[] outBuffer)
		{
			return Audio.process_double (ticks, inBuffer, outBuffer) == 0;
		}

		/// <summary>
		/// Stops audio computation.
		/// </summary>
		public void Stop ()
		{
			MessageInvocation.SendMessage ("pd", "dsp", new Float (0));
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
			var ptr = General.openfile (Path.GetFileName (path), Path.GetDirectoryName (path));
			return new Patch (ptr);
		}

		public PdArray GetArray (string array)
		{
			return new PdArray (array);
		}
	}
}
