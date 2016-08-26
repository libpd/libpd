/*
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 * 
 * Copyright(c) 2016 Thomas Mayer<thomas@residuum.org>
 */
using System;
using LibPDBinding;
using NAudio.Utils;
using NAudio.Wave;

namespace LibPdBindingNaudio
{
    class PdProvider : IWaveProvider, IDisposable
    {
        /// <summary>
        /// number of ticks for libPd to compute in a computation cycle.
        /// 
        /// lower values may lead to distortion because of switching between threads.
        /// </summary>
        static readonly int Ticks = 8;
        static readonly int BlockSize = LibPD.BlockSize;
        static readonly int SampleRate = 44100;
        static readonly int Channels = 2;
        static readonly int BufferSize = Channels*Ticks*BlockSize;

        /// <summary>
        /// buffer for libPd to fill on computation.
        /// </summary>
        readonly float[] _buffer = new float[BufferSize];

        /// <summary>
        /// use a CircularBuffer similar to BufferedWaveProvider.
        /// </summary>
        CircularBuffer _circularBuffer;
        int _minBuffer;
        int _patchHandle;

        public PdProvider()
        {
            SetUpBuffer();
            SetUpPd();
            RefillBuffer();
        }

        ~PdProvider()
        {
            Dispose(false);
        }

        /// <summary>
        /// Sets up CircularBuffer for storage and float[] for getting data from libPd.
        /// </summary>
        void SetUpBuffer()
        {
            _circularBuffer = new CircularBuffer(BlockSize * Ticks * Channels * 4); // make the circular buffer large enough
            _minBuffer = BlockSize * Ticks * Channels * 2;
        }

        /// <summary>
        /// Sets up communication with libPd.
        /// </summary>
        void SetUpPd()
        {
            // Open Pd file
            _patchHandle = LibPD.OpenPatch("../../pd/test.pd");
            // Subscribe to receiver
            LibPD.Float += LibPd_Float;
            LibPD.Subscribe(CursorReceiver);
            // Set up audio format for Pd
            LibPD.OpenAudio(0, 2, SampleRate);
            // Start audio
            LibPD.ComputeAudio(true);
        }

        /// <summary>
        /// An example for reading messages from LibPD
        /// </summary>
        void LibPd_Float(string receiver, float value)
        {
            Console.WriteLine("{0}, {1}", receiver, value);
        }

        /// <summary>
        /// Let libPd compute data, while the CircularBuffer has less than _minBuffer bytes available.
        /// </summary>
        void RefillBuffer()
        {
            while (_circularBuffer.Count < _minBuffer)
            {
                // Compute audio. Take care of the array sizes for audio in and out.
                LibPD.Process(Ticks, new float[0], _buffer);
                _circularBuffer.Write(PcmFromFloat(_buffer), 0, _buffer.Length * 4);
            }
        }

        /// <summary>
        /// Convert float[] from libPd to byte[] for CircularBuffer.
        /// 
        /// This is surely optimizable
        /// </summary>
        byte[] PcmFromFloat(float[] pdOutput)
        {
            WaveBuffer wavebuffer = new WaveBuffer(pdOutput.Length * 4);
            for (var i = 0; i < pdOutput.Length; i++)
            {
                wavebuffer.FloatBuffer[i] = pdOutput[i];
            }
            return wavebuffer.ByteBuffer;
        }

        public int Read(byte[] buffer, int offset, int count)
        {
            int read = _circularBuffer.Read(buffer, offset, count);
            RefillBuffer();
            return read;
        }

        public WaveFormat WaveFormat
        {
            get
            {
                // We have float wave format
                return WaveFormat.CreateIeeeFloatWaveFormat(SampleRate, Channels);
            }
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        void Dispose(bool isDisposing)
        {
            // Unsubscribe from all message receivers
            LibPD.Unsubscribe(CursorReceiver);
            LibPD.Float -= LibPd_Float;
            // Disable audio
            LibPD.ComputeAudio(false);
            if (_patchHandle > 0)
            {
                // Close patch
                LibPD.ClosePatch(_patchHandle);
            }
            LibPD.Release();
        }

        public static string CursorReceiver = "cursor";
    }
}