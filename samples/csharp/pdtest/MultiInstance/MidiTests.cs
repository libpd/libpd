using System;
using NUnit.Framework;
using LibPDBinding.Managed;
using LibPDBinding.Managed.Events;

namespace LibPDBindingTest.MultiInstance
{
	[TestFixture]
	public class MidiTests
	{
		Pd _instance1;
		Pd _instance2;
		Patch _patch1;
		Patch _patch2;
		static readonly int _inputs = 2;
		static readonly int _outputs = 2;
		static readonly int _sampleRate = 44100;
		static readonly int _channel = 1;
		static readonly int _pitch = 64;
		static readonly int _velocity = 32;
		int _receivedChannel1 = 0;
		int _receivedPitch1 = 0;
		int _receivedVelocity1 = 0;
		int _receivedChannel2 = 0;
		int _receivedPitch2 = 0;
		int _receivedVelocity2 = 0;

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
		public virtual void DataSendToInstance1ShouldBeReceivedAtInstance1 ()
		{
			float value1 = 0;
			_instance1.Midi.NoteOn += delegate (object sender, NoteOnEventArgs args) {
				_receivedChannel1 = args.Channel;
				_receivedPitch1 = args.Pitch;
				_receivedVelocity1 = args.Velocity;
			};
			_instance1.Midi.SendNoteOn (_channel, _pitch, _velocity);
			Assert.AreEqual (_channel, _receivedChannel1);
			Assert.AreEqual (_pitch, _receivedPitch1);
			Assert.AreEqual (_velocity, _receivedVelocity1);
		}

		[Test]
		public virtual void DataSendToInstance2ShouldBeReceivedAtInstance2 ()
		{
			float value2 = 0;
			_instance2.Midi.NoteOn += delegate (object sender, NoteOnEventArgs args) {
				_receivedChannel2 = args.Channel;
				_receivedPitch2 = args.Pitch;
				_receivedVelocity2 = args.Velocity;
			};
			_instance2.Midi.SendNoteOn (_channel + 1, _pitch + 1, _velocity + 1);
			Assert.AreEqual (_channel + 1, _receivedChannel2);
			Assert.AreEqual (_pitch + 1, _receivedPitch2);
			Assert.AreEqual (_velocity + 1, _receivedVelocity2);   
		}

		[Test]
		public virtual void DataSendToInstance1ShouldNotBeReceivedAtInstance2 ()
		{
			float value2 = 0;
			_instance2.Midi.NoteOn += delegate (object sender, NoteOnEventArgs args) {
				_receivedChannel2 = args.Channel;
				_receivedPitch2 = args.Pitch;
				_receivedVelocity2 = args.Velocity;
			};
			_instance2.Midi.SendNoteOn (_channel + 1, _pitch + 1, _velocity + 1);
			Assert.AreEqual (_channel + 1, _receivedChannel2);
			Assert.AreEqual (_pitch + 1, _receivedPitch2);
			Assert.AreEqual (_velocity + 1, _receivedVelocity2);   
		}

		[Test]
		public virtual void DataSendToInstance2ShouldNotBeReceivedAtInstance1 ()
		{
			float value1 = 0;
			_instance1.Midi.NoteOn += delegate (object sender, NoteOnEventArgs args) {
				_receivedChannel1 = args.Channel;
				_receivedPitch1 = args.Pitch;
				_receivedVelocity1 = args.Velocity;
			};
			_instance2.Midi.SendNoteOn (_channel + 1, _pitch + 1, _velocity + 1);
			Assert.AreEqual (_channel + 1, _receivedChannel2);
			Assert.AreEqual (_pitch + 1, _receivedPitch2);
			Assert.AreEqual (_velocity + 1, _receivedVelocity2);   
		}

		[Test]
		public virtual void DataShouldBeReceivedAtTheCorrectInstance ()
		{
			_instance1.Midi.NoteOn += delegate (object sender, NoteOnEventArgs args) {
				_receivedChannel1 = args.Channel;
				_receivedPitch1 = args.Pitch;
				_receivedVelocity1 = args.Velocity;
			};
			_instance2.Midi.NoteOn += delegate (object sender, NoteOnEventArgs args) {
				_receivedChannel2 = args.Channel;
				_receivedPitch2 = args.Pitch;
				_receivedVelocity2 = args.Velocity;
			};
			_instance1.Midi.SendNoteOn (_channel, _pitch, _velocity);
			_instance2.Midi.SendNoteOn (_channel + 1, _pitch + 1, _velocity + 1);
			Assert.AreEqual (_channel, _receivedChannel1);
			Assert.AreEqual (_pitch, _receivedPitch1);
			Assert.AreEqual (_velocity, _receivedVelocity1);            
			Assert.AreEqual (_channel + 1, _receivedChannel2);
			Assert.AreEqual (_pitch + 1, _receivedPitch2);
			Assert.AreEqual (_velocity + 1, _receivedVelocity2);            
		}

	}
}
