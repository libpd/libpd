using System;

namespace LibPDBinding.Managed.Events
{

	public class ProgramChangeEventArgs:EventArgs
	{
		public int Channel { get; private set; }

		public int Value { get; private set; }

		public ProgramChangeEventArgs (int channel, int value)
		{
			Channel = channel;
			Value = value;
		}
	}
}
