using LibPDBinding.Managed;
using NUnit.Framework;

namespace LibPDBindingTest.Managed
{
	[TestFixture]
	public class PdTests
	{
		Pd _pd;
		static readonly int _inputs = 2;
		static readonly int _outputs = 3;
		static readonly int _sampleRate = 44100;

		[SetUp]
		public void Init ()
		{
			_pd = new Pd (_inputs, _outputs, _sampleRate);
		}

		[TearDown]
		public void Cleanup ()
		{
			_pd.Dispose ();
		}

		[Test]
		public virtual void InputCountTest ()
		{
			Assert.AreEqual (_inputs, _pd.Inputs);
		}

		[Test]
		public virtual void OutputCountTest ()
		{
			Assert.AreEqual (_outputs, _pd.Outputs);
		}

		[Test]
		public virtual void SampleRateTest ()
		{
			Assert.AreEqual (_sampleRate, _pd.SampleRate);
		}

		[Test]
		public virtual void BlockSizeTest ()
		{
			Assert.AreEqual (64, _pd.BlockSize);
		}

		[Test]
		public virtual void InitTest ()
		{
			Assert.IsFalse (_pd.IsComputing);
		}

		[Test]
		public virtual void StartTest ()
		{
			_pd.Start ();
			Assert.IsTrue (_pd.IsComputing);
		}

		[Test]
		public virtual void StopTest ()
		{
			_pd.Start ();
			_pd.Stop ();
			Assert.IsFalse (_pd.IsComputing);
		}
	}
}
