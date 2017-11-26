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
		public virtual void NoteOnTest ()
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
		public virtual void ProgramChangeTest ()
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
		public virtual void ControlChangeTest ()
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

		[Test]
		public virtual void PitchbendTest ()
		{
			int channel = 1;
			int value = -512;
			int receivedChannel = 0;
			int receivedValue = 0;
			_pd.Midi.Pitchbend += delegate (object sender, PitchbendEventArgs args) {
				receivedChannel = args.Channel;
				receivedValue = args.Value;
			};
			_pd.Midi.SendPitchbend (channel, value);
			Assert.AreEqual (channel, receivedChannel);
			Assert.AreEqual (value, receivedValue);            
		}

		[Test]
		public virtual void AftertouchTest ()
		{
			int channel = 1;
			int value = 32;
			int receivedChannel = 0;
			int receivedValue = 0;
			_pd.Midi.Aftertouch += delegate (object sender, AftertouchEventArgs args) {
				receivedChannel = args.Channel;
				receivedValue = args.Value;
			};
			_pd.Midi.SendAftertouch (channel, value);
			Assert.AreEqual (channel, receivedChannel);
			Assert.AreEqual (value, receivedValue);            
		}

		[Test]
		public virtual void PolyAftertouchTest ()
		{
			int channel = 1;
			int pitch = 56;
			int value = 32;
			int receivedChannel = 0;
			int receivedPitch = 0;
			int receivedValue = 0;
			_pd.Midi.PolyAftertouch += delegate (object sender, PolyAftertouchEventArgs args) {
				receivedChannel = args.Channel;
				receivedPitch = args.Pitch;
				receivedValue = args.Value;
			};
			_pd.Midi.SendPolyAftertouch (channel, pitch, value);
			Assert.AreEqual (channel, receivedChannel);
			Assert.AreEqual (pitch, receivedPitch);
			Assert.AreEqual (value, receivedValue);            
		}

		[Test]
		public virtual void MidiByteTest ()
		{
			int port = 1;
			int value = 32;
			int receivedPort = 0;
			int receivedValue = 0;
			_pd.Midi.MidiByte += delegate (object sender, MidiByteEventArgs args) {
				receivedPort = args.Port;
				receivedValue = args.Value;
			};
			_pd.Midi.SendMidiByte (port, value);
			Assert.AreEqual (port, receivedPort);
			Assert.AreEqual (value, receivedValue);            
		}
	}
}