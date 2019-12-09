//
// PdArrayTest.cs
//
// Author:
//       thomas <${AuthorEmail}>
//
// Copyright (c) 2016 thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

using LibPDBinding.Managed;
using NUnit.Framework;

namespace LibPDBindingTest.Managed
{
	[TestFixture]
	public class PdArrayTests
	{
		Pd _pd;
		Patch _patch;
		static readonly int _inputs = 1;
		static readonly int _outputs = 2;
		static readonly int _sampleRate = 44100;

		[SetUp]
		public void Init ()
		{
			_pd = new Pd (_inputs, _outputs, _sampleRate);
			_patch = _pd.LoadPatch (@"../../test_csharp.pd");
		}

		[TearDown]
		public void Cleanup ()
		{
			_patch.Dispose ();
			_pd.Dispose ();
		}

		[Test]
		public void ArrayLengthTest ()
		{
			PdArray array = _pd.GetArray ("array1");
			Assert.AreEqual (128, array.Size);
		}

		[Test]
		public void NonExistingTest ()
		{
			PdArray array = _pd.GetArray ("array2");
			Assert.AreEqual (-1, array.Size);
		}

		[Test]
		public void ResizeTest ()
		{
			PdArray array = _pd.GetArray ("array1");
			array.Resize (256);
			Assert.AreEqual (256, array.Size);
		}

		[Test]
		public void WriteTest ()
		{
			PdArray array = _pd.GetArray ("array1");
			int n = array.Size;
			float[] input = new float[n];
			for (int i = 0; i < n; i++) {
				input [i] = i;
			}
			array.Write (input, 0, n);
		}

		[Test]
		public void ReadTest ()
		{
			PdArray array = _pd.GetArray ("array1");
			int n = array.Size;
			float[] input = new float[n];
			for (int i = 0; i < n; i++) {
				input [i] = i;
			}
			array.Write (input, 0, n);
			float[] read = array.Read (0, n);
			for (int i = 0; i < n; i++) {
				Assert.AreEqual (i, read [i]);
			}
		}
	}
}

