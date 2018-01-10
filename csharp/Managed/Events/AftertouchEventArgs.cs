using System;

namespace LibPDBinding.Managed.Events
{
	public class AftertouchEventArgs:EventArgs
	{
		public int Channel { get; private set; }

		public int Value { get; private set; }

		public AftertouchEventArgs (int channel, int value)
		{
			Channel = channel;
			Value = value;
		}
	}
}
