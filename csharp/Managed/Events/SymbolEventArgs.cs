using System;
using LibPDBinding.Managed.Data;

namespace LibPDBinding.Managed.Events
{
	public class SymbolEventArgs : EventArgs
	{
		public string Receiver { get; private set; }

		public Symbol Symbol { get; private set; }

		public SymbolEventArgs (string recv, string sym)
		{
			Receiver = recv;
			Symbol = new Symbol(sym);
		}
	}
}