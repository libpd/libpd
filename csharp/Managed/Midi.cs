using System;
using LibPDBinding.Managed.Events;
using LibPDBinding.Native;

namespace LibPDBinding.Managed
{
	public sealed class Midi : IDisposable
	{
		internal Midi ()
		{
			SetupHooks ();
		}

		~Midi ()
		{
			Dispose (false);
		}

		public void Dispose ()
		{
			Dispose (true);
			GC.SuppressFinalize (this);
		}

		private void Dispose (bool disposing)
		{			
			NoteOn = null;
			ProgramChange = null;
			ControlChange = null;
			Pitchbend = null;
			Aftertouch = null;
			PolyAftertouch = null;
			MidiByte = null;
		}

		public event EventHandler<NoteOnEventArgs> NoteOn;
		public event EventHandler<ProgramChangeEventArgs> ProgramChange;
		public event EventHandler<ControlChangeEventArgs> ControlChange;
		public event EventHandler<PitchbendEventArgs> Pitchbend;
		public event EventHandler<AftertouchEventArgs> Aftertouch;
		public event EventHandler<PolyAftertouchEventArgs> PolyAftertouch;
		public event EventHandler<MidiByteEventArgs> MidiByte;

		LibPDNoteOnHook NoteOnHook;
		LibPDProgramChangeHook ProgramChangeHook;
		LibPDControlChangeHook ControlChangeHook;
		LibPDPitchbendHook PitchbendHook;
		LibPDAftertouchHook AftertouchHook;
		LibPDPolyAftertouchHook PolyAftertouchHook;
		LibPDMidiByteHook MidiByteHook;

		public void SendNoteOn (int channel, int pitch, int velocity)
		{
			PInvoke.noteon (channel, pitch, velocity);
		}

		public void SendProgramChange (int channel, int value)
		{
			PInvoke.programchange (channel, value);
		}

		public void SendControlChange (int channel, int controller, int value)
		{
			PInvoke.controlchange (channel, controller, value);			
		}

		public void SendPitchbend (int channel, int value)
		{
			PInvoke.pitchbend (channel, value);			
		}

		public void SendAftertouch (int channel, int value)
		{
			PInvoke.aftertouch (channel, value);
		}

		public void SendPolyAftertouch (int channel, int pitch, int value)
		{
			PInvoke.polyaftertouch (channel, pitch, value);
		}

		public void SendMidiByte (int port, int value)
		{
			PInvoke.midibyte (port, value);
		}

		public void SendSysex (int port, int value)
		{
			PInvoke.sysex (port, value);
		}

		public void SendSysRealtime (int port, int value)
		{
			PInvoke.sysrealtime (port, value);
		}

		void RaiseNoteOnEvent (int channel, int pitch, int velocity)
		{
			if (NoteOn != null) {
				NoteOn (this, new NoteOnEventArgs (channel, pitch, velocity));
			}
		}

		void RaiseProgramChangeEvent (int channel, int value)
		{
			if (ProgramChange != null) {
				ProgramChange (this, new ProgramChangeEventArgs (channel, value));
			}
		}

		void RaiseControlChangeEvent (int channel, int controller, int value)
		{
			if (ControlChange != null) {
				ControlChange (this, new ControlChangeEventArgs (channel, controller, value));
			}
		}

		void RaisePitchbendEvent (int channel, int value)
		{
			if (Pitchbend != null) {
				Pitchbend (this, new PitchbendEventArgs (channel, value));
			}
		}

		void RaiseAftertouchEvent (int channel, int value)
		{
			if (Aftertouch != null) {
				Aftertouch (this, new AftertouchEventArgs (channel, value));
			}
		}

		void RaisePolyAftertouchEvent (int channel, int pitch, int value)
		{
			if (PolyAftertouch != null) {
				PolyAftertouch (this, new PolyAftertouchEventArgs (channel, pitch, value));
			}
		}

		void RaiseMidiByteEvent (int port, int midiByte)
		{
			if (MidiByte != null) {
				MidiByte (this, new MidiByteEventArgs (port, midiByte));
			}
		}

		void SetupHooks ()
		{
			NoteOnHook = new LibPDNoteOnHook (RaiseNoteOnEvent);
			PInvoke.set_noteonhook (NoteOnHook);
			ProgramChangeHook = new LibPDProgramChangeHook (RaiseProgramChangeEvent);
			PInvoke.set_programchangehook (ProgramChangeHook);
			ControlChangeHook = new LibPDControlChangeHook (RaiseControlChangeEvent);
			PInvoke.set_controlchangehook (ControlChangeHook);
			PitchbendHook = new LibPDPitchbendHook (RaisePitchbendEvent);
			PInvoke.set_pitchbendhook (RaisePitchbendEvent);
			AftertouchHook = new LibPDAftertouchHook (RaiseAftertouchEvent);
			PInvoke.set_aftertouchhook (RaiseAftertouchEvent);
			PolyAftertouchHook = new LibPDPolyAftertouchHook (RaisePolyAftertouchEvent);
			PInvoke.set_polyaftertouchhook (RaisePolyAftertouchEvent);
			MidiByteHook = new LibPDMidiByteHook (RaiseMidiByteEvent);
			PInvoke.set_midibytehook (RaiseMidiByteEvent);
		}
	}
}