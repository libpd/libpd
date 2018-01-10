using System;

namespace LibPDBinding.Managed.Events
{
	public class NoteOnEventArgs : EventArgs
	{
		public int Channel { get; private set; }

		public int Pitch { get; private set; }

		public int Velocity { get; private set; }

		public NoteOnEventArgs (int channel, int pitch, int velocity)
		{
			Channel = channel;
			Pitch = pitch;
			Velocity = velocity;
		}
	}

}