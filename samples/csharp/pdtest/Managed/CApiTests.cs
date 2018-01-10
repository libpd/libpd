using System;
using LibPDBinding.Managed;
using NUnit.Framework;
using LibPDBinding.Managed.Events;

namespace LibPDBindingTest.Managed
{
	[TestFixture]
	public class CApiTests
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
		public virtual void LoadbangTest ()
		{
			_pd.Messaging.Bind ("foo");
			string value = "";
			_pd.Messaging.Print += delegate(object sender, PrintEventArgs e) {
				value += e.Symbol.Value;
			};
			using (Patch patch = _pd.LoadPatch (@"../../test_csharp.pd")) {
				;
			}
			Assert.AreEqual("print: hello\n", value);
		}

		[Test]
		public virtual void FloatFormattingTest ()
		{
			using (Patch patch = _pd.LoadPatch (@"../../test_float.pd")) {
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
					Assert.AreEqual (i, outBuffer [3 * i], 0.0001);
					Assert.AreEqual (i + 0.5, outBuffer [3 * i + 1], 0.0001);
					Assert.AreEqual (0, outBuffer [3 * i + 2], 0.0001);
				}
			}
		}
	}
}
