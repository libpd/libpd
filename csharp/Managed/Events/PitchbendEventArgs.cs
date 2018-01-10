using System;

namespace LibPDBinding.Managed.Events
{
	public class PitchbendEventArgs:EventArgs
	{
		public int Channel { get; private set; }

		public int Value { get; private set; }

		public PitchbendEventArgs (int channel, int value)
		{
			Channel = channel;
			Value = value;
		}
	}	
}