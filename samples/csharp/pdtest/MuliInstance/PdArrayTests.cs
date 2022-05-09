using System;
using LibPDBinding.Managed;
using NUnit.Framework;

namespace LibPDBindingTest.MultiInstance
{
    [TestFixture]
	public class PdArrayTests
	{
		Pd _instance1;
		Pd _instance2;
		Patch _patch1;
		Patch _patch2;
		static readonly int _inputs = 2;
		static readonly int _outputs = 2;
		static readonly int _sampleRate = 44100;
		PdArray _array1;
		PdArray _array2;

		[SetUp]
		public void Init ()
		{
			_instance1 = new Pd (_inputs, _outputs, _sampleRate);
			_instance2 = new Pd (_inputs, _outputs, _sampleRate);
			_patch1 = _instance1.LoadPatch ("../../test_multi.pd");
			_patch2 = _instance2.LoadPatch ("../../test_multi.pd");
			_array1 = _instance1.GetArray ("array1");
			_array2 = _instance2.GetArray ("array1");
		}

		[TearDown]
		public void Cleanup ()
		{
			_patch1.Dispose ();
			_patch2.Dispose ();
			_instance1.Dispose ();
			_instance2.Dispose ();
		}

		[Test]
		public virtual void SizeShouldBeDecoupled ()
		{
			int arraySize = 128;
			_array2.Resize (arraySize * 2);
			Assert.AreEqual (arraySize, _array1.Size);
			Assert.AreEqual (arraySize * 2, _array2.Size);
		}

		[Test]
		public virtual void WritingToInstance1ShouldOnlyWriteOnInstance1 ()
		{
			int arraySize = 128;
			float[] valueToSet = new float[arraySize];
			for (int i = 0; i < arraySize; i++) {
				valueToSet [i] = 1f;
			}
			_array1.Write (valueToSet, 0, arraySize);
			float[] readArray1 = _array1.Read (0, arraySize);
			float[] readArray2 = _array2.Read (0, arraySize);
			for (int i = 0; i < arraySize; i++) {
				Assert.AreEqual (1f, readArray1 [i]);
			}
			for (int i = 0; i < arraySize; i++) {
				Assert.AreEqual (0f, readArray2 [i]);
			}
		}

		[Test]
		public virtual void WritingToInstance2ShouldOnlyWriteOnInstance2 ()
		{
			int arraySize = 128;
			float[] valueToSet = new float[arraySize];
			for (int i = 0; i < arraySize; i++) {
				valueToSet [i] = 1f;
			}
			_array2.Write (valueToSet, 0, arraySize);
			float[] readArray1 = _array1.Read (0, arraySize);
			float[] readArray2 = _array2.Read (0, arraySize);
			for (int i = 0; i < arraySize; i++) {
				Assert.AreEqual (0f, readArray1 [i]);
			}
			for (int i = 0; i < arraySize; i++) {
				Assert.AreEqual (1f, readArray2 [i]);
			}
		}
	}
}

