/*
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 * 
 * 
 * Created by SharpDevelop.
 * User: Tebjan Halm
 * Date: 03.05.2012
 * Time: 16:54
 * 
 * 
 */

using System;
using NUnit.Framework;

using LibPDBinding;

namespace LibPDBindingTest
{
	[TestFixture]
	public class LibPDTests
	{
		private static int patch;

		[SetUp]
		public static void loadPatch()
		{
			LibPD.OpenAudio(2, 3, 44100);
			patch = LibPD.OpenPatch(@"..\..\test_csharp.pd");
			LibPD.ComputeAudio(true);
		}

		[TearDown]
		public static void closePatch()
		{
			LibPD.Release();
		}
		
		[Test]
		public virtual void testDollarZero()
		{
			Assert.AreEqual(1004, patch);
		}

		[Test]
		public virtual void testAudio()
		{
			float[] @in = new float[256];
			float[] @out = new float[768];
			for (int i = 0; i < 256; i++)
			{
				@in[i] = i;
			}
			int err = LibPD.Process(2, @in, @out);
			Assert.AreEqual(0, err);
			for (int i = 0; i < 128; i++)
			{
				Assert.AreEqual(2 * i, @out[3 * i], 0.0001);
				Assert.AreEqual(-6 * i, @out[3 * i + 1], 0.0001);
				Assert.AreEqual(Math.Cos(2 * Math.PI * 440 / 44100 * i), @out[3 * i + 2], 0.0001);
			}
			for (int i = 384; i < 768; i++)
			{
				Assert.AreEqual(0, @out[i], 0);
			}
		}

		[Test]
		public virtual void testBlockSize()
		{
			Assert.AreEqual(64, LibPD.BlockSize);
		}

		[Test]
		public virtual void testPrint()
		{
			var msgs = new string[]
			{
				"print: bang\n",
				"print: 0\n",
				"print: 42\n",
				"print: symbol",
				" ",
				"don't panic",
				"\n",
			};
			
			var i = 0;
			LibPD.Print += delegate(string text) 
			{ 
				Assert.AreEqual(msgs[i++], text);
			};
			
			LibPD.SendBang("foo");
			LibPD.SendFloat("foo", 0);
			LibPD.SendFloat("foo", 42);
			LibPD.SendSymbol("foo", "don't panic");
		}

	}
}
