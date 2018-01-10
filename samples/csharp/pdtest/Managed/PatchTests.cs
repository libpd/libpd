using LibPDBinding.Managed;
using NUnit.Framework;

namespace LibPDBindingTest.Managed
{
	[TestFixture]
	public class PatchTests
	{
		Pd _pd;
		static readonly int _inputs = 1;
		static readonly int _outputs = 2;
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
		public void NotNullOnExistingTest ()
		{
			using (Patch patch = _pd.LoadPatch (@"../../test_csharp.pd")) {
				Assert.NotNull (patch);
			}
		}

		[Test]
		public void NullOnNotExistingTest ()
		{
			Patch patch = _pd.LoadPatch (@"something_not_there.pd");
			Assert.Null (patch);
		}

		[Test]
		public void DollarZeroTest ()
		{
			using (Patch patch = _pd.LoadPatch (@"../../test_csharp.pd")) {
				Assert.True (patch.DollarZero > 1000);
			}
		}
	}
}
