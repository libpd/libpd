using System;
using System.Collections.Generic;
using LibPDBinding.Managed.Data;
using LibPDBinding.Managed.Utils;

namespace LibPDBinding.Managed.Events
{
	public class ListEventArgs : EventArgs
	{
		public string Receiver { get; private set; }

		public IEnumerable<IAtom> List { get; private set; }

		public ListEventArgs (string recv, int argc, IntPtr argv)
		{
			Receiver = recv;
			List = MessageInvocation.ConvertList (argc, argv);
		}
	}

}
