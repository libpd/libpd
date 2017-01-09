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
		public virtual void TestLoadBang ()
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
	}
}
