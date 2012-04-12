/*
 * Created by SharpDevelop.
 * User: Tebjan Halm
 * Date: 08.04.2012
 * Time: 21:53
 * 
 */
 
using System;
using NAudio.Wave;

namespace LibPDBinding
{
	/// <summary>
	/// Class for Buffer callback using LibPD
	/// </summary>
	public class LibPDWaveProvider : IWaveProvider
	{
		float[] FInBuffer;
		
		private WaveFormat waveFormat;
		public WaveFormat WaveFormat
		{
			get
			{
				return this.waveFormat;
			}
		}
		
		public LibPDWaveProvider() : this(44100, 2)
		{
		}
		
		public LibPDWaveProvider(int sampleRate, int channels)
		{
			this.SetWaveFormat(sampleRate, channels);
			FInBuffer = new float[8192];
		}
		
		public void SetWaveFormat(int sampleRate, int channels)
		{
			this.waveFormat = WaveFormat.CreateIeeeFloatWaveFormat(sampleRate, channels);
		}
		
		public unsafe int Read(byte[] buffer, int offset, int sampleCount)
		{
			//calc nr of PD frames
			var ticks = (sampleCount/4)/(64*waveFormat.Channels);
			
			fixed(float* inBuff = FInBuffer)
			{
			fixed(byte* outBuff = buffer)
			{
				LibPD.Process(ticks, ref inBuff[0], ref ((float*)outBuff)[0]);
			}
			}
			
			return sampleCount;
		}
	}
	
}
