using LibPDBinding.Managed.Data;
using System;

namespace LibPDBinding.Managed.Events
{
	public class PrintEventArgs : EventArgs
	{
		public Symbol Symbol { get; private set; }

		public PrintEventArgs (string text)
		{
			Symbol = new Symbol (text);
		}
	}
}