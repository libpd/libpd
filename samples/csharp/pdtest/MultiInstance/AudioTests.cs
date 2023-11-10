using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using LibPDBinding.Managed;
using LibPDBinding.Managed.Data;
using LibPDBinding.Managed.Events;
using NUnit.Framework;

namespace LibPDBindingTest.MultiInstance
{
	[TestFixture]
	public class AudioTests
	{
		Pd _instance1;
		Pd _instance2;
		Patch _patch1;
		Patch _patch2;
		static readonly int _inputs = 2;
		static readonly int _outputs = 2;
		static readonly int _sampleRate = 44100;

		[SetUp]
		public void Init ()
		{
			_instance1 = new Pd (_inputs, _outputs, _sampleRate);
			_instance2 = new Pd (_inputs, _outputs, _sampleRate);
			_patch1 = _instance1.LoadPatch ("../../test_multi.pd");
			_patch2 = _instance2.LoadPatch ("../../test_multi.pd");
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
		public virtual void DecoupledAudioTest ()
		{
			int arraySize = _inputs * _instance1.BlockSize;
			float[] valueToSet1 = new float[arraySize];
			float[] valueToSet2 = new float[arraySize];
			float[] valueToGet1 = new float[arraySize];
			float[] valueToGet2 = new float[arraySize];
			for (int i = 0; i < arraySize; i++) {
				valueToSet1 [i] = 1f;
				valueToSet2 [i] = 0.5f;
			}
			_instance1.Start ();
			_instance1.Process (1, valueToSet1, valueToGet1);
			_instance1.Stop ();
			_instance2.Start ();
			_instance2.Process (1, valueToSet2, valueToGet2);
			_instance2.Stop ();
			for (int i = 0; i < arraySize; i++) {
				Assert.AreEqual (1f, valueToGet1 [i]);
			}
			for (int i = 0; i < arraySize; i++) {
				Assert.AreEqual (0.5f, valueToGet2 [i]);
			}
		}
	}
}
