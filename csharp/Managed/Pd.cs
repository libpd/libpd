using LibPDBinding.Managed.Data;
using LibPDBinding.Managed.Utils;
using LibPDBinding.Native;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;

namespace LibPDBinding.Managed
{
	/// <summary>
	/// Class for a Pd instance.
	///
	/// NB: If the C library is built without multi instance support, only one instance of Pd should be instanciated.
	/// </summary>
	public sealed class Pd : IDisposable
	{

		public static int NumberOfInstances {
			get {
				if (!_initialized) {
					return 0;
				}

				return MultiInstance.num_instances () - 1;
			}
		}

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
			Inputs = inputChannels;
			Outputs = outputChannels;
			SampleRate = sampleRate;
			if (!_initialized) {
				General.libpd_init ();
				_initialized = true;
			}
			_thisInstance = MultiInstance.new_instance ();
			Activate ();
			_messaging = new Messaging (this);
			_midi = new Midi (this);
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

		void Dispose (bool disposing)
		{
			if (disposing) {
				Stop ();
			}
			Messaging.Dispose ();
			Midi.Dispose ();
			MultiInstance.free_instance (_thisInstance);
			_currentInstance = IntPtr.Zero;
		}

		static bool _initialized;
		static IntPtr _currentInstance = IntPtr.Zero;
		readonly IntPtr _thisInstance;

		internal void Activate ()
		{
			if (_currentInstance == _thisInstance) {
				return;
			}
			MultiInstance.set_instance (_thisInstance);
			_currentInstance = _thisInstance;
		}

		/// <summary>
		/// Starts audio computation.
		/// </summary>
		[MethodImpl (MethodImplOptions.Synchronized)]
		public void Start ()
		{
			Activate ();
			Audio.init_audio (Inputs, Outputs, SampleRate);
			MessageInvocation.SendMessage ("pd", "dsp", new Float (1));
			IsComputing = true;
		}

		[MethodImpl (MethodImplOptions.Synchronized)]
		public bool Process (int ticks, short[] inBuffer, short[] outBuffer)
		{
			Activate ();
			return Audio.process_short (ticks, inBuffer, outBuffer) == 0;
		}

		/// <summary>
		/// Reads samples from inBuffer and processes them with Pd.
		/// </summary>
		/// <param name="ticks">Number of ticks to process. To reduce overhead of function calls, raise this number. To lower latency, reduce this number.</param>
		/// <param name="inBuffer">Interleaved input buffer. Must be at least of size [InputChannels] * [Blocksize] * [ticks]</param>
		/// <param name="outBuffer">Output buffer for storing interleaved processed audio. Must be at least of size [OutputChannels] * [Blocksize] * [ticks]</param>
		/// <returns>[true] if audio processing was successful, [false] otherwise.</returns>
		[MethodImpl (MethodImplOptions.Synchronized)]
		public bool Process (int ticks, float[] inBuffer, float[] outBuffer)
		{
			Activate ();
			return Audio.process_float (ticks, inBuffer, outBuffer) == 0;
		}

		[MethodImpl (MethodImplOptions.Synchronized)]
		public bool Process (int ticks, double[] inBuffer, double[] outBuffer)
		{
			Activate ();
			return Audio.process_double (ticks, inBuffer, outBuffer) == 0;
		}

		/// <summary>
		/// Stops audio computation.
		/// </summary>
		[MethodImpl (MethodImplOptions.Synchronized)]
		public void Stop ()
		{
			Activate ();
			MessageInvocation.SendMessage ("pd", "dsp", new Float (0));
			IsComputing = false;
		}

		/// <summary>
		/// Loads a Pd patch from the specified file path.
		/// </summary>
		/// <param name="path">Path to the Pd file.</param>
		/// <returns>New Patch, if loading is successful, else null.</returns>
		[MethodImpl (MethodImplOptions.Synchronized)]
		public Patch LoadPatch (string path)
		{
			Activate ();
			if (path.StartsWith (".")) {
				string currentDirectory = AppDomain.CurrentDomain.BaseDirectory;
				path = Path.Combine (currentDirectory, path);
			}
			if (!File.Exists (path)) {
				return null;
			}
			var ptr = General.openfile (Path.GetFileName (path), Path.GetDirectoryName (path));
			return new Patch (ptr, this);
		}

		/// <summary>
		/// Gets a Pd array with the specified name.
		/// </summary>
		/// <param name="array">Name of the array.</param>
		/// <returns>New PdArray.</returns>
		[MethodImpl (MethodImplOptions.Synchronized)]
		public PdArray GetArray (string array)
		{
			Activate ();
			return new PdArray (this, array);
		}
	}
}
