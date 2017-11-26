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
		}

		public event EventHandler<NoteOnEventArgs> NoteOn;
		public event EventHandler<ProgramChangeEventArgs> ProgramChange;
		public event EventHandler<ControlChangeEventArgs> ControlChange;

		LibPDNoteOnHook NoteOnHook;
		LibPDProgramChangeHook ProgramChangeHook;
		LibPDControlChangeHook ControlChangeHook;

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

		void RaiseControlChangeEvent (int channel, int controller,  int value)
		{
			if (ControlChange != null) {
				ControlChange (this, new ControlChangeEventArgs (channel, controller, value));
			}
		}

		void SetupHooks ()
		{
			NoteOnHook = new LibPDNoteOnHook (RaiseNoteOnEvent);
			PInvoke.set_noteonhook (NoteOnHook);
			ProgramChangeHook = new LibPDProgramChangeHook(RaiseProgramChangeEvent);
			PInvoke.set_programchangehook (ProgramChangeHook);
			ControlChangeHook = new LibPDControlChangeHook(RaiseControlChangeEvent);
			PInvoke.set_controlchangehook (ControlChangeHook);
		}
	}
}