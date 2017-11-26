using System;

namespace LibPDBinding.Managed.Events
{
	public class PolyAftertouchEventArgs:EventArgs
	{
		public int Channel { get; private set; }

		public int Pitch { get; private set; }

		public int Value { get; private set; }

		public PolyAftertouchEventArgs (int channel, int pitch, int value)
		{
			Channel = channel;
			Pitch = pitch;
			Value = value;
		}
	}
}