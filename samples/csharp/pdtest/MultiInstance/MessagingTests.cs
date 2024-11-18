using System;
using NUnit.Framework;
using LibPDBinding.Managed;
using LibPDBinding.Managed.Events;
using LibPDBinding.Managed.Data;

namespace LibPDBindingTest.MultiInstance
{
	[TestFixture]
	public class MessagingTests
	{
		Pd _instance1;
		Pd _instance2;
		Patch _patch1;
		Patch _patch2;
		static readonly int _inputs = 2;
		static readonly int _outputs = 2;
		static readonly int _sampleRate = 44100;
		static readonly string _receiver = "spam";

		[SetUp]
		public void Init ()
		{
			_instance1 = new Pd (_inputs, _outputs, _sampleRate);
			_instance2 = new Pd (_inputs, _outputs, _sampleRate);
			_patch1 = _instance1.LoadPatch ("../../test_multi.pd");
			_patch2 = _instance2.LoadPatch ("../../test_multi.pd");
			_instance1.Messaging.Bind (_receiver);
			_instance2.Messaging.Bind (_receiver);
		}

		[TearDown]
		public void Cleanup ()
		{
			_instance1.Messaging.Unbind (_receiver);
			_instance2.Messaging.Unbind (_receiver);
			_patch1.Dispose ();
			_patch2.Dispose ();
			_instance1.Dispose ();
			_instance2.Dispose ();
		}

		[Test]
		public virtual void DataSendToInstance1ShouldBeReceivedAtInstance1 ()
		{
			float value1 = 0;
			_instance1.Messaging.Float += delegate(object sender, FloatEventArgs e) {
				if (e.Receiver == _receiver) {
					value1 = e.Float.Value;
				}
			};
			_instance1.Messaging.Send (_receiver, new Float (42));
			Assert.AreEqual (42, value1);
		}

		[Test]
		public virtual void DataSendToInstance2ShouldBeReceivedAtInstance2 ()
		{
			float value2 = 0;
			_instance2.Messaging.Float += delegate(object sender, FloatEventArgs e) {
				if (e.Receiver == _receiver) {
					value2 = e.Float.Value;
				}
			};
			_instance2.Messaging.Send (_receiver, new Float (23));
			Assert.AreEqual (23, value2);
		}

		[Test]
		public virtual void DataSendToInstance1ShouldNotBeReceivedAtInstance2 ()
		{
			float value2 = 0;
			_instance2.Messaging.Float += delegate(object sender, FloatEventArgs e) {
				if (e.Receiver == _receiver) {
					value2 = e.Float.Value;
				}
			};
			_instance1.Messaging.Send (_receiver, new Float (42));
			Assert.AreEqual (0, value2);
		}

		[Test]
		public virtual void DataSendToInstance2ShouldNotBeReceivedAtInstance1 ()
		{
			float value1 = 0;
			_instance1.Messaging.Float += delegate(object sender, FloatEventArgs e) {
				if (e.Receiver == _receiver) {
					value1 = e.Float.Value;
				}
			};
			_instance2.Messaging.Send (_receiver, new Float (23));
			Assert.AreEqual (0, value1);
        }

        [Test]
        public virtual void DataShouldBeReceivedAtTheCorrectInstanceOnlyOnce ()
        {
            float value1 = 0;
            float value2 = 0;
            _instance1.Messaging.Float += delegate (object sender, FloatEventArgs e) {
                if (e.Receiver == _receiver)
                {
                    value1 = e.Float.Value;
                }
            };
            _instance2.Messaging.Float += delegate (object sender, FloatEventArgs e) {
                if (e.Receiver == _receiver)
                {
                    value2 = e.Float.Value;
                }
            };
            _instance1.Messaging.Send (_receiver, new Float (42));
            _instance2.Messaging.Send (_receiver, new Float (23));
            Assert.AreEqual (42, value1);
            Assert.AreEqual (23, value2);
        }


        [Test]
        public virtual void DataShouldBeReceivedAtTheCorrectInstance ()
        {
            string sendSymbol = "bar";
            string receiveSymbol = "foo";
            _instance1.Messaging.Bind(sendSymbol);
            _instance2.Messaging.Bind(sendSymbol);
            float value1 = 0;
            float value2 = 0;
            _instance1.Messaging.Float += delegate (object sender, FloatEventArgs e) {
                if (e.Receiver == sendSymbol)
                {
                    value1 = e.Float.Value;
                }
            };
            _instance2.Messaging.Float += delegate (object sender, FloatEventArgs e) {
                if (e.Receiver == sendSymbol)
                {
                    value2 = e.Float.Value;
                }
            };
            _instance1.Messaging.Send (receiveSymbol, new Bang ());
            Assert.AreEqual (_patch1.DollarZero, value1);
			Assert.AreEqual (0, value2);
			value1 = 0;
			value2 = 0;
            _instance2.Messaging.Send (receiveSymbol, new Bang ());
            Assert.AreEqual (0, value1);
            Assert.AreEqual (_patch2.DollarZero, value2);
            _instance1.Messaging.Unbind (sendSymbol);
            _instance2.Messaging.Unbind (sendSymbol);
        }
    }
}