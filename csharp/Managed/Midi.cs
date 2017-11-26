using System;
using LibPDBinding.Managed.Events;
using LibPDBinding.Native;
using System.Runtime.CompilerServices;

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
			PInvoke.noteon (channel, pitch, velocity);
		}

		/// <summary>
		/// Sends program change MIDI message.
		/// </summary>
		/// <param name="channel">Channel.</param>
		/// <param name="value">Value.</param>
		[MethodImpl (MethodImplOptions.Synchronized)]
		public void SendProgramChange (int channel, int value)
		{
			PInvoke.programchange (channel, value);
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
			PInvoke.controlchange (channel, controller, value);			
		}

		/// <summary>
		/// Sends pitchbend MIDI message.
		/// </summary>
		/// <param name="channel">Channel.</param>
		/// <param name="value">Value.</param>
		[MethodImpl (MethodImplOptions.Synchronized)]
		public void SendPitchbend (int channel, int value)
		{
			PInvoke.pitchbend (channel, value);			
		}

		/// <summary>
		/// Sends aftertouch MIDI message.
		/// </summary>
		/// <param name="channel">Channel.</param>
		/// <param name="value">Value.</param>
		[MethodImpl (MethodImplOptions.Synchronized)]
		public void SendAftertouch (int channel, int value)
		{
			PInvoke.aftertouch (channel, value);
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
			PInvoke.polyaftertouch (channel, pitch, value);
		}

		/// <summary>
		/// Sends raw midi byte.
		/// </summary>
		/// <param name="port">Port.</param>
		/// <param name="value">Byte.</param>
		[MethodImpl (MethodImplOptions.Synchronized)]
		public void SendMidiByte (int port, int value)
		{
			PInvoke.midibyte (port, value);
		}

		/// <summary>
		/// Sends MIDI system exclusive byte.
		/// </summary>
		/// <param name="port">Port.</param>
		/// <param name="value">Byte.</param>
		[MethodImpl (MethodImplOptions.Synchronized)]
		public void SendSysex (int port, int value)
		{
			PInvoke.sysex (port, value);
		}

		/// <summary>
		/// Sends MIDI system realtime byte.
		/// </summary>
		/// <param name="port">Port.</param>
		/// <param name="value">Value.</param>
		[MethodImpl (MethodImplOptions.Synchronized)]
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