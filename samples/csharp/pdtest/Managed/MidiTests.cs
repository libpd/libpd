using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using LibPDBinding.Managed;
using LibPDBinding.Managed.Events;
using NUnit.Framework;

namespace LibPDBindingTest.Managed
{
	[TestFixture]
	public class MidiTests
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
		public virtual void SendNoteOn ()
		{
			int channel = 1;
			int pitch = 64;
			int velocity = 32;
			int receivedChannel = 0;
			int receivedPitch = 0;
			int receivedVelocity = 0;
			_pd.Midi.NoteOn += delegate (object sender, NoteOnEventArgs args) {
				receivedChannel = args.Channel;
				receivedPitch = args.Pitch;
				receivedVelocity = args.Velocity;
			};
			_pd.Midi.SendNoteOn (channel, pitch, velocity);
			Assert.AreEqual (channel, receivedChannel);
			Assert.AreEqual (pitch, receivedPitch);
			Assert.AreEqual (velocity, receivedVelocity);            
		}

		[Test]
		public virtual void SendProgramChange ()
		{
			int channel = 1;
			int value = 32;
			int receivedChannel = 0;
			int receivedValue = 0;
			_pd.Midi.ProgramChange += delegate (object sender, ProgramChangeEventArgs args) {
				receivedChannel = args.Channel;
				receivedValue = args.Value;
			};
			_pd.Midi.SendProgramChange (channel, value);
			Assert.AreEqual (channel, receivedChannel);
			Assert.AreEqual (value, receivedValue);            
		}

		[Test]
		public virtual void SendControlChange ()
		{
			int channel = 1;
			int controller = 9;
			int value = 32;
			int receivedChannel = 0;
			int receivedController = 0;
			int receivedValue = 0;
			_pd.Midi.ControlChange += delegate (object sender, ControlChangeEventArgs args) {
				receivedChannel = args.Channel;
				receivedController = args.Controller;
				receivedValue = args.Value;
			};
			_pd.Midi.SendControlChange (channel, controller, value);
			Assert.AreEqual (channel, receivedChannel);
			Assert.AreEqual (controller, receivedController);
			Assert.AreEqual (value, receivedValue);            
		}
	}
}