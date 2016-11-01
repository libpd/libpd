using System;
using LibPDBinding.Managed;
using NUnit.Framework;
using LibPDBinding.Managed.Events;
using LibPDBinding.Managed.Data;

namespace LibPDBindingTest.Managed
{
	[TestFixture]
	public class MessagingTests
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
			_pd.Start ();
		}

		[TearDown]
		public void Cleanup ()
		{
			_patch.Dispose ();
			_pd.Dispose ();
		}

		[Test]
		public virtual void SendFloatTest ()
		{
			string value = null;
			_pd.Messaging.Print += delegate (object sender, PrintEventArgs args) {
				value = args.Symbol.Value;
			};
			_pd.Messaging.Send ("foo", new Float (42));
			Assert.AreEqual ("print: 42\n", value);
		}

		[Test]
		public virtual void SendBangTest ()
		{
			string value = null;
			_pd.Messaging.Print += delegate (object sender, PrintEventArgs args) {
				value += args.Symbol.Value;
			};
			_pd.Messaging.Send ("foo", new Bang());
			Assert.AreEqual ("print: bang\n", value);
		}

		[Test]
		public virtual void SendSymbolTest(){
			string value = null;
			_pd.Messaging.Print += delegate (object sender, PrintEventArgs args) {
				value += args.Symbol.Value;
			};
			_pd.Messaging.Send ("foo", new Symbol ("bar"));
			Assert.AreEqual ("print: symbol bar\n", value);
		}

		[Test]
		public virtual void SendListTest(){
			string value = null;
			_pd.Messaging.Print += delegate (object sender, PrintEventArgs args) {
				value += args.Symbol.Value;
			};
			_pd.Messaging.Send ("foo", new Symbol("bar"), new Float(42));
			Assert.AreEqual ("print: list bar 42\n", value);
		}

		[Test]
		public virtual void ReceiveFloatTest(){
			float value = 0;
			string receiver = "spam";
			_pd.Messaging.Bind (receiver);
			_pd.Messaging.Float += delegate(object sender, FloatEventArgs e) {
				if (e.Receiver == receiver){
					value = e.Float.Value;
				}
			};
			_pd.Messaging.Send (receiver, new Float (42));
			_pd.Messaging.Unbind (receiver);
			Assert.AreEqual (42, value);
		}

		[Test]
		public virtual void ReceiveSymbolTest(){
			string value = null;
			string receiver = "spam";
			_pd.Messaging.Bind (receiver);
			_pd.Messaging.Symbol += delegate(object sender, SymbolEventArgs e) {
				if (e.Receiver == receiver){
					value = e.Symbol.Value;
				}
			};
			_pd.Messaging.Send (receiver, new Symbol ("foo"));
			_pd.Messaging.Unbind (receiver);
			Assert.AreEqual ("foo", value);
		}

		[Test]
		public virtual void ReceiveBangTest(){
			bool received = false;
			string receiver = "spam";
			_pd.Messaging.Bind (receiver);
			_pd.Messaging.Bang += delegate(object sender, BangEventArgs e) {
				if (e.Receiver == receiver){
					received = true;
				}
			};
			_pd.Messaging.Send (receiver, new Bang());
			_pd.Messaging.Unbind (receiver);
			Assert.IsTrue (received);
		}

		[Test]
		public virtual void UnbindTest(){
			int received = 0;
			string receiver = "spam";
			_pd.Messaging.Bind (receiver);
			_pd.Messaging.Bang += delegate(object sender, BangEventArgs e) {
				if (e.Receiver == receiver){
					received++;
				}
			};
			_pd.Messaging.Send (receiver, new Bang());
			_pd.Messaging.Send (receiver, new Bang());
			Assert.AreEqual (2, received);
			_pd.Messaging.Unbind (receiver);
			_pd.Messaging.Send (receiver, new Bang());
			Assert.AreEqual (2, received);
		}
	}
}
