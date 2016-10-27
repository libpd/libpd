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
using LibPDBinding;
using LibPDBindingTest.LibraryLoader;
using NUnit.Framework;

namespace LibPDBindingTest
{
	[TestFixture]
	public class LibPDTests
	{
		private static int SPatch;
		private static IntPtr SDllHandle;
		private static IDllLoader loader;

		[SetUp]
		public static void loadPatch ()
		{
			if (loader == null) {
				loader = IsLinux () ? (IDllLoader)new LinuxDllLoader () : (IDllLoader)new WindowsDllLoader ();
			}
			SDllHandle = loader.LoadLibrary ("libpdcsharp." + (IsLinux () ? "so" : "dll"));
			LibPD.ReInit ();

			LibPD.OpenAudio (2, 3, 44100);
			SPatch = LibPD.OpenPatch (@"../../test_csharp.pd");
			LibPD.ComputeAudio (true);
		}

		private static bool IsLinux ()
		{
			var p = (int)Environment.OSVersion.Platform;
			return (p == 4) || (p == 6) || (p == 128);
		}

		[TearDown]
		public static void closePatch ()
		{
			LibPD.Release ();

			while (loader.FreeLibrary (SDllHandle)) {
			}
		}

		[Test]
		public virtual void testDollarZero ()
		{
			Assert.AreEqual (1003, SPatch);
		}

		[Test]
		public virtual void testAudio ()
		{
			float[] inBuffer = new float[256];
			float[] outBuffer = new float[768];
			for (int i = 0; i < 256; i++) {
				inBuffer [i] = i;
			}
			int err = LibPD.Process (2, inBuffer, outBuffer);
			Assert.AreEqual (0, err);
			for (int i = 0; i < 128; i++) {
				Assert.AreEqual (2 * i, outBuffer [3 * i], 0.0001);
				Assert.AreEqual (-6 * i, outBuffer [3 * i + 1], 0.0001);
				Assert.AreEqual (Math.Cos (2 * Math.PI * 440 / 44100 * i), outBuffer [3 * i + 2], 0.0001);
			}
			for (int i = 384; i < 768; i++) {
				Assert.AreEqual (0, outBuffer [i], 0);
			}
		}

		[Test]
		public virtual void testBlockSize ()
		{
			Assert.AreEqual (64, LibPD.BlockSize);
		}

		[Test]
		public virtual void testSubscription ()
		{
			Assert.False (LibPD.Exists ("baz"));
			Assert.False (LibPD.Subscribe (null));
			Assert.True (LibPD.Subscribe ("baz"));
			Assert.True (LibPD.Exists ("baz"));
			Assert.False (LibPD.Unsubscribe (null));
			Assert.False (LibPD.Unsubscribe (""));
			Assert.True (LibPD.Unsubscribe ("baz"));
			Assert.False (LibPD.Exists ("baz"));
		}

		[Test]
		public virtual void testPrint ()
		{
			var msgs = new string[] {
				"print: bang\n",
				"print: 0\n",
				"print: 42\n",
				"print: symbol",
				" ",
				"don't panic",
				"\n",
			};
			
			var i = 0;
			
			LibPDPrint del = delegate(string text) { 
				Assert.AreEqual (msgs [i++], text);
			};
				
			LibPD.Print += del;
			
			LibPD.SendBang ("foo");
			LibPD.SendFloat ("foo", 0);
			LibPD.SendFloat ("foo", 42);
			LibPD.SendSymbol ("foo", "don't panic");
			
			Assert.AreEqual (msgs.Length, i);
			
			LibPD.Print -= del;
		}

		[Test]
		public virtual void testReceive ()
		{
			var receiver = "spam";
			var listArgs = new object[]{ "hund", 1, "katze", 2.5, "maus", 3.1f };
			var msgName = "testing";
			var msgArgs = new object[]{ "one", 1, "two", 2 };
			
			LibPD.Subscribe (receiver);
			
			var n = 0;
			
			LibPDBang delBang = delegate(string recv) {
				Assert.AreEqual (receiver, recv);
				n++;
			};
			
			LibPD.Bang += delBang;
			
			LibPDFloat delFloat = delegate(string recv, float x) {
				Assert.AreEqual (receiver, recv);
				Assert.AreEqual (42, x);
				n++;
			};
			
			LibPD.Float += delFloat;
			
			LibPDSymbol delSymbol = delegate(string recv, string sym) {
				Assert.AreEqual (receiver, recv);
				Assert.AreEqual ("hund katze maus", sym);
				n++;
			};
			
			LibPD.Symbol += delSymbol;
			
			LibPDList delList = delegate(string recv, object[] args) {  
				Assert.AreEqual (receiver, recv);
				Assert.AreEqual (listArgs.Length, args.Length);
				
				for (int i = 0; i < args.Length; i++) {
					Assert.AreEqual (listArgs [i], args [i]);
				}
				n++;
			};

			LibPD.List += delList;
			
			LibPDMessage delMessage = delegate(string recv, string msg, object[] args) {  
				
				Assert.AreEqual (receiver, recv);
				Assert.AreEqual (msgName, msg);
				Assert.AreEqual (msgArgs.Length, args.Length);
				
				for (int i = 0; i < args.Length; i++) {
					Assert.AreEqual (msgArgs [i], args [i]);
				}
				n++;
			};
			
			LibPD.Message += delMessage;
			
			LibPD.SendBang (receiver);
			LibPD.SendFloat (receiver, 42);
			LibPD.SendSymbol (receiver, "hund katze maus");
			LibPD.SendList (receiver, listArgs);
			LibPD.SendMessage (receiver, msgName, msgArgs);
			
			Assert.AreEqual (5, n);
			
			LibPD.Bang -= delBang;
			LibPD.Float -= delFloat;
			LibPD.Symbol -= delSymbol;
			LibPD.List -= delList;
			LibPD.Message -= delMessage;
		}

		[Test]
		public virtual unsafe void testArrayAccess ()
		{
			int n = 128;
			Assert.AreEqual (n, LibPD.ArraySize ("array1"));
			
			float[] u = new float[n];
			float[] v = new float[n];
			for (int i = 0; i < n; i++) {
				u [i] = i;
			}
			
			LibPD.WriteArray ("array1", 0, u, n);
			LibPD.ReadArray (v, "array1", 0, n);
			for (int i = 0; i < n; i++) {
				Assert.AreEqual (u [i], v [i], 0);
			}
			
			fixed(float* vp = v) {
				LibPD.ReadArray (&vp [5], "array1", 50, 10);
				for (int i = 0; i < n; i++) {
					if (i < 5 || i >= 15) {
						Assert.AreEqual (u [i], v [i], 0);
					} else {
						Assert.AreEqual (u [i + 45], v [i], 0);
					}
				}
			}
			
			fixed(float* up = u) {
				LibPD.WriteArray ("array1", 10, &up [25], 30);
			}
			
			LibPD.ReadArray (v, "array1", 0, n);
			for (int i = 0; i < n; i++) {
				if (i < 10 || i >= 40) {
					Assert.AreEqual (u [i], v [i], 0);
				} else {
					Assert.AreEqual (u [i + 15], v [i], 0);
				}
			}
		}

	}
}
