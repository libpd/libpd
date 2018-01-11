using System;
using LibPDBinding.Managed;
using NUnit.Framework;

namespace LibPDBindingTest.Managed
{
	[TestFixture]
	public class AudioTests
	{
		Pd _pd;
		Patch _patch;
		static readonly int _inputs = 2;
		static readonly int _outputs = 3;
		static readonly int _sampleRate = 44100;

		[SetUp]
		public void Init ()
		{
			_pd = new Pd (_inputs, _outputs, _sampleRate);
			_patch = _pd.LoadPatch ("../../test_csharp.pd");
		}

		[TearDown]
		public void Cleanup ()
		{
			_patch.Dispose ();
			_pd.Dispose ();
		}

		[Test]
		public virtual void AudioOutSizeTest ()
		{
			int blocksize = _pd.BlockSize;
			int ticks = 2;
			float[] inBuffer = new float[ticks * _inputs * blocksize];
			for (int i = 0; i < inBuffer.Length; i++) {
				inBuffer [i] = i;
			}
			float[] outBuffer = new float[ticks * _outputs * blocksize];
			_pd.Process (ticks, inBuffer, outBuffer);
			Assert.AreEqual (ticks * _outputs * blocksize, outBuffer.Length);
		}

		[Test]
		public virtual void AudioOffTest ()
		{
			int blocksize = _pd.BlockSize;
			int ticks = 2;
			float[] inBuffer = new float[ticks * _inputs * blocksize];
			for (int i = 0; i < inBuffer.Length; i++) {
				inBuffer [i] = i;
			}
			float[] outBuffer = new float[ticks * _outputs * blocksize];
			_pd.Process (ticks, inBuffer, outBuffer);
			for (int i = 0; i < outBuffer.Length; i++) {
				Assert.AreEqual (0, outBuffer [i]);
			}
		}

		[Test]
		public virtual void AudioOnTest ()
		{
			int blocksize = _pd.BlockSize;
			int ticks = 2;
			float[] inBuffer = new float[ticks * _inputs * blocksize];
			for (int i = 0; i < inBuffer.Length; i++) {
				inBuffer [i] = i;
			}
			_pd.Start ();
			float[] outBuffer = new float[ticks * _outputs * blocksize];
			_pd.Process (ticks, inBuffer, outBuffer);
			for (int i = 0; i < outBuffer.Length / 3; i++) {
				Assert.AreEqual (2 * i, outBuffer [3 * i], 0.0001);
				Assert.AreEqual (-6 * i, outBuffer [3 * i + 1], 0.0001);
				Assert.AreEqual (Math.Cos (2 * Math.PI * 440 / 44100 * i), outBuffer [3 * i + 2], 0.0001);
			}
		}

		[Test]
		public virtual void DoubleTest ()
		{
			int blocksize = _pd.BlockSize;
			int ticks = 2;
			double[] inBuffer = new double[ticks * _inputs * blocksize];
			for (int i = 0; i < inBuffer.Length; i++) {
				inBuffer [i] = i;
			}
			_pd.Start ();
			double[] outBuffer = new double[ticks * _outputs * blocksize];
			_pd.Process (ticks, inBuffer, outBuffer);
			for (int i = 0; i < outBuffer.Length / 3; i++) {
				Assert.AreEqual (2 * i, outBuffer [3 * i], 0.0001);
				Assert.AreEqual (-6 * i, outBuffer [3 * i + 1], 0.0001);
				Assert.AreEqual (Math.Cos (2 * Math.PI * 440 / 44100 * i), outBuffer [3 * i + 2], 0.0001);
			}
		}

		[Test]
		public virtual void ShortTest ()
		{
			int blocksize = _pd.BlockSize;
			int ticks = 2;
			short[] inBuffer = new short[ticks * _inputs * blocksize];
			for (int i = 0; i < inBuffer.Length; i++) {
				inBuffer [i] = (short)i;
			}
			_pd.Start ();
			short[] outBuffer = new short[ticks * _outputs * blocksize];
			_pd.Process (ticks, inBuffer, outBuffer);
			for (int i = 0; i < outBuffer.Length / 3; i++) {
				Assert.AreEqual ((short)(2 * i / (float)short.MaxValue), outBuffer [3 * i]);
				Assert.AreEqual ((short)(-6 * i / (float)short.MaxValue), outBuffer [3 * i + 1]);
				Assert.AreEqual ((short)(32767 * Math.Cos (2 * Math.PI * 440 / 44100 * i) / short.MaxValue), outBuffer [3 * i + 2]);
			}
		}
	}
}
