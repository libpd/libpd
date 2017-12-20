/*
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 * 
 * Copyright(c) 2016 Thomas Mayer<thomas@residuum.org>
 */
using System;
using NAudio.Utils;
using NAudio.Wave;
using LibPDBinding.Managed;
using LibPDBinding.Managed.Events;

namespace LibPdBindingNaudio
{
	class NewApiPdProvider : IWaveProvider, IDisposable
	{
		/// <summary>
		/// number of ticks for libPd to compute in a computation cycle.
		/// 
		/// lower values may lead to distortion because of switching between threads.
		/// </summary>
		static readonly int Ticks = 8;
		static readonly int SampleRate = 44100;
		static readonly int Channels = 2;

		/// <summary>
		/// use a CircularBuffer similar to BufferedWaveProvider.
		/// </summary>
		CircularBuffer _circularBuffer;
		int _minBuffer;
		Pd _pd;
		Patch _patch;
		float[] _pdBuffer;

		public NewApiPdProvider ()
		{
			SetUpPd ();
			SetUpBuffer ();
			RefillBuffer ();
		}

		~NewApiPdProvider ()
		{
			Dispose (false);
		}

		/// <summary>
		/// Sets up CircularBuffer for storage and float[] for getting data from libPd.
		/// </summary>
		void SetUpBuffer ()
		{
			int blocksize = _pd.BlockSize;
			_circularBuffer = new CircularBuffer (blocksize * Ticks * Channels * 4); // make the circular buffer large enough
			_pdBuffer = new float[Ticks * Channels * blocksize];
			_minBuffer = blocksize * Ticks * Channels * 2;
		}

		/// <summary>
		/// Sets up communication with libPd.
		/// </summary>
		void SetUpPd ()
		{
			// Init new Pd instance
			_pd = new Pd (0, 2, SampleRate);
			// Open Pd patch
			_patch = _pd.LoadPatch ("../../../pd/test.pd");
			// Subscribe to receiver
			_pd.Messaging.Float += Pd_Float;
			_pd.Messaging.Bind (CursorReceiver);
			// Start audio
			_pd.Start ();
		}

		/// <summary>
		/// An example for reading messages from LibPD
		/// </summary>
		void Pd_Float (object sender, FloatEventArgs e)
		{
			Console.WriteLine ("{0}, {1}", e.Receiver, e.Float.Value);
		}

		/// <summary>
		/// Let libPd compute data, while the CircularBuffer has less than _minBuffer bytes available.
		/// </summary>
		void RefillBuffer ()
		{
			while (_circularBuffer.Count < _minBuffer) {
				// Compute audio. Take care of the array sizes for audio in and out.z
				_pd.Process (Ticks, new float[0], _pdBuffer);
				_circularBuffer.Write (PcmFromFloat (_pdBuffer), 0, _pdBuffer.Length * 4);
			}
		}

		/// <summary>
		/// Convert float[] from libPd to byte[] for CircularBuffer.
		/// 
		/// This is surely optimizable
		/// </summary>
		byte[] PcmFromFloat (float[] pdOutput)
		{
			WaveBuffer wavebuffer = new WaveBuffer (pdOutput.Length * 4);
			for (var i = 0; i < pdOutput.Length; i++) {
				wavebuffer.FloatBuffer [i] = pdOutput [i];
			}
			return wavebuffer.ByteBuffer;
		}

		public int Read (byte[] buffer, int offset, int count)
		{
			int read = _circularBuffer.Read (buffer, offset, count);
			RefillBuffer ();
			return read;
		}

		public WaveFormat WaveFormat {
			get {
				// We have float wave format
				return WaveFormat.CreateIeeeFloatWaveFormat (SampleRate, Channels);
			}
		}

		public void Dispose ()
		{
			Dispose (true);
			GC.SuppressFinalize (this);
		}

		void Dispose (bool isDisposing)
		{
			// Only for illustration purposes, simply disposing of _patch and _pd is enough as with any sane implementation of IDisposable.
			// Unsubscribe from all message receivers
			_pd.Messaging.Unbind (CursorReceiver);
			_pd.Messaging.Float -= Pd_Float;
			// Stop audio
			_pd.Stop ();

			// Dispose of the IDisposables in correct order
			_patch.Dispose ();
			_pd.Dispose ();
		}

		public static string CursorReceiver = "cursor";
	}
}