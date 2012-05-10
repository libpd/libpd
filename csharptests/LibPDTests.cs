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
		public virtual void aTestDollarZero()
		{
			Assert.AreEqual(1002, patch);
		}

		[Test]
		public virtual void testAudio()
		{
			float[] inBuffer = new float[256];
			float[] outBuffer = new float[768];
			for (int i = 0; i < 256; i++)
			{
				inBuffer[i] = i;
			}
			int err = LibPD.Process(2, inBuffer, outBuffer);
			Assert.AreEqual(0, err);
			for (int i = 0; i < 128; i++)
			{
				Assert.AreEqual(2 * i, outBuffer[3 * i], 0.0001);
				Assert.AreEqual(-6 * i, outBuffer[3 * i + 1], 0.0001);
				Assert.AreEqual(Math.Cos(2 * Math.PI * 440 / 44100 * i), outBuffer[3 * i + 2], 0.0001);
			}
			for (int i = 384; i < 768; i++)
			{
				Assert.AreEqual(0, outBuffer[i], 0);
			}
		}

		[Test]
		public virtual void testBlockSize()
		{
			Assert.AreEqual(64, LibPD.BlockSize);
		}
		
		[Test]
		public virtual void atestSubscription()
		{
			Assert.False(LibPD.Exists("baz"));
			Assert.False(LibPD.Subscribe(null));
			Assert.True(LibPD.Subscribe("baz"));
			Assert.True(LibPD.Exists("baz"));
			Assert.False(LibPD.Unsubscribe(null));
			Assert.False(LibPD.Unsubscribe(""));
			Assert.True(LibPD.Unsubscribe("baz"));
			Assert.False(LibPD.Exists("baz"));
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
			
			Assert.AreEqual(msgs.Length, i);
		}
		
		[Test]
		public virtual void atestReceive()
		{
			var receiver = "spam";
			var listArgs = new object[]{"hund", 1, "katze", 2.5, "maus", 3.1f};
			
			LibPD.Subscribe(receiver);
			
			var n = 0;
			LibPD.Bang += delegate(string recv) 
			{
				Assert.AreEqual(receiver, recv);
				n++;
			};
			
			LibPD.Float += delegate(string recv, float x) 
			{
				Assert.AreEqual(receiver, recv);
				Assert.AreEqual(42, x);
				n++;
			};
			
			LibPD.Symbol += delegate(string recv, string sym) 
			{
				Assert.AreEqual(receiver, recv);
				Assert.AreEqual("hund katze maus", sym);
				n++;
			};
			
			LibPD.List += delegate(string recv, object[] args) 
			{  
				Assert.AreEqual(receiver, recv);
				Assert.AreEqual(listArgs.Length, args.Length);
				
				for (int i = 0; i < args.Length; i++) 
				{
					Assert.AreEqual(listArgs[i], args[i]);
				}
				n++;
			};

			LibPD.SendBang(receiver);
			LibPD.SendFloat(receiver, 42);
			LibPD.SendSymbol(receiver, "hund katze maus");
			LibPD.SendList(receiver, listArgs);
			
			Assert.AreEqual(4, n);
		}
		
		[Test]
		public unsafe virtual void testArrayAccess()
		{
			int n = 128;
			Assert.AreEqual(n, LibPD.ArraySize("array1"));
			
			float[] u = new float[n];
			float[] v = new float[n];
			for (int i = 0; i < n; i++)
			{
				u[i] = i;
			}
			
			LibPD.WriteArray("array1", 0, u, n);
			LibPD.ReadArray(v, "array1", 0, n);
			for (int i = 0; i < n; i++)
			{
				Assert.AreEqual(u[i], v[i], 0);
			}
			
			fixed(float* vp = v)
			{
				LibPD.ReadArray(&vp[5], "array1", 50, 10);
				for (int i = 0; i < n; i++)
				{
					if (i < 5 || i >= 15)
					{
						Assert.AreEqual(u[i], v[i], 0);
					}
					else
					{
						Assert.AreEqual(u[i + 45], v[i], 0);
					}
				}
			}
			
			fixed(float* up = u)
			{
				LibPD.WriteArray("array1", 10, &up[25], 30);
			}
			
			LibPD.ReadArray(v, "array1", 0, n);
			for (int i = 0; i < n; i++)
			{
				if (i < 10 || i >= 40)
				{
					Assert.AreEqual(u[i], v[i], 0);
				}
				else
				{
					Assert.AreEqual(u[i + 15], v[i], 0);
				}
			}
		}

	}
}
