using System;

namespace LibPDBinding.Managed.Events
{
	public class BangEventArgs : EventArgs
	{
		public string Receiver { get; private set; }

		public BangEventArgs (string text)
		{
			Receiver = text;
		}
	}

}
