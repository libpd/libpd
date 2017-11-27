using System;
using System.Runtime.CompilerServices;
using LibPDBinding.Managed.Events;

namespace LibPDBinding.Managed
{
	/// <summary>
	/// MIDI in Pd.
	/// </summary>
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

		void Dispose (bool disposing)
		{			
			NoteOn = null;
			ProgramChange = null;
			ControlChange = null;
			Pitchbend = null;
			Aftertouch = null;
			PolyAftertouch = null;
			MidiByte = null;
		}

		/// <summary>
		/// Occurs when MIDI note on message is sent from Pd.
		/// </summary>
		public event EventHandler<NoteOnEventArgs> NoteOn;

		/// <summary>
		/// Occurs when MIDI program change is sent from Pd.
		/// </summary>
		public event EventHandler<ProgramChangeEventArgs> ProgramChange;

		/// <summary>
		/// Occurs when MIDI control change is sent from Pd.
		/// </summary>
		public event EventHandler<ControlChangeEventArgs> ControlChange;

		/// <summary>
		/// Occurs when MIDI pitchbend is sent from Pd.
		/// </summary>
		public event EventHandler<PitchbendEventArgs> Pitchbend;

		/// <summary>
		/// Occurs when MIDI aftertouch is sent from Pd.
		/// </summary>
		public event EventHandler<AftertouchEventArgs> Aftertouch;

		/// <summary>
		/// Occurs when MIDI poly aftertouch is sent from Pd.
		/// </summary>
		public event EventHandler<PolyAftertouchEventArgs> PolyAftertouch;

		/// <summary>
		/// Occurs when raw MIDI byte is sent from Pd.
		/// </summary>
		public event EventHandler<MidiByteEventArgs> MidiByte;

		LibPDNoteOnHook NoteOnHook;
		LibPDProgramChangeHook ProgramChangeHook;
		LibPDControlChangeHook ControlChangeHook;
		LibPDPitchbendHook PitchbendHook;
		LibPDAftertouchHook AftertouchHook;
		LibPDPolyAftertouchHook PolyAftertouchHook;
		LibPDMidiByteHook MidiByteHook;

		/// <summary>
		/// Sends note on MIDI message.
		/// </summary>
		/// <param name="channel">Channel.</param>
		/// <param name="pitch">Pitch.</param>
		/// <param name="velocity">Velocity.</param>
		[MethodImpl (MethodImplOptions.Synchronized)]
		public void SendNoteOn (int channel, int pitch, int velocity)
		{
			Native.Midi.noteon (channel, pitch, velocity);
		}

		/// <summary>
		/// Sends program change MIDI message.
		/// </summary>
		/// <param name="channel">Channel.</param>
		/// <param name="value">Value.</param>
		[MethodImpl (MethodImplOptions.Synchronized)]
		public void SendProgramChange (int channel, int value)
		{
			Native.Midi.programchange (channel, value);
		}

		/// <summary>
		/// Sends control change MIDI message.
		/// </summary>
		/// <param name="channel">Channel.</param>
		/// <param name="controller">Controller.</param>
		/// <param name="value">Value.</param>
		[MethodImpl (MethodImplOptions.Synchronized)]
		public void SendControlChange (int channel, int controller, int value)
		{
			Native.Midi.controlchange (channel, controller, value);			
		}

		/// <summary>
		/// Sends pitchbend MIDI message.
		/// </summary>
		/// <param name="channel">Channel.</param>
		/// <param name="value">Value.</param>
		[MethodImpl (MethodImplOptions.Synchronized)]
		public void SendPitchbend (int channel, int value)
		{
			Native.Midi.pitchbend (channel, value);			
		}

		/// <summary>
		/// Sends aftertouch MIDI message.
		/// </summary>
		/// <param name="channel">Channel.</param>
		/// <param name="value">Value.</param>
		[MethodImpl (MethodImplOptions.Synchronized)]
		public void SendAftertouch (int channel, int value)
		{
			Native.Midi.aftertouch (channel, value);
		}

		/// <summary>
		/// Sends poly aftertouch MIDI message.
		/// </summary>
		/// <param name="channel">Channel.</param>
		/// <param name="pitch">Pitch.</param>
		/// <param name="value">Value.</param>
		[MethodImpl (MethodImplOptions.Synchronized)]
		public void SendPolyAftertouch (int channel, int pitch, int value)
		{
			Native.Midi.polyaftertouch (channel, pitch, value);
		}

		/// <summary>
		/// Sends raw midi byte.
		/// </summary>
		/// <param name="port">Port.</param>
		/// <param name="value">Byte.</param>
		[MethodImpl (MethodImplOptions.Synchronized)]
		public void SendMidiByte (int port, int value)
		{
			Native.Midi.midibyte (port, value);
		}

		/// <summary>
		/// Sends MIDI system exclusive byte.
		/// </summary>
		/// <param name="port">Port.</param>
		/// <param name="value">Byte.</param>
		[MethodImpl (MethodImplOptions.Synchronized)]
		public void SendSysex (int port, int value)
		{
			Native.Midi.sysex (port, value);
		}

		/// <summary>
		/// Sends MIDI system realtime byte.
		/// </summary>
		/// <param name="port">Port.</param>
		/// <param name="value">Value.</param>
		[MethodImpl (MethodImplOptions.Synchronized)]
		public void SendSysRealtime (int port, int value)
		{
			Native.Midi.sysrealtime (port, value);
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
			Native.Midi.set_noteonhook (NoteOnHook);
			ProgramChangeHook = new LibPDProgramChangeHook (RaiseProgramChangeEvent);
			Native.Midi.set_programchangehook (ProgramChangeHook);
			ControlChangeHook = new LibPDControlChangeHook (RaiseControlChangeEvent);
			Native.Midi.set_controlchangehook (ControlChangeHook);
			PitchbendHook = new LibPDPitchbendHook (RaisePitchbendEvent);
			Native.Midi.set_pitchbendhook (PitchbendHook);
			AftertouchHook = new LibPDAftertouchHook (RaiseAftertouchEvent);
			Native.Midi.set_aftertouchhook (AftertouchHook);
			PolyAftertouchHook = new LibPDPolyAftertouchHook (RaisePolyAftertouchEvent);
			Native.Midi.set_polyaftertouchhook (PolyAftertouchHook);
			MidiByteHook = new LibPDMidiByteHook (RaiseMidiByteEvent);
			Native.Midi.set_midibytehook (MidiByteHook);
		}
	}
}