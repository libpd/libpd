using System;
using System.Collections.Generic;
using LibPDBinding.Managed.Data;
using LibPDBinding.Managed.Utils;

namespace LibPDBinding.Managed.Events
{
	public class MessageEventArgs : EventArgs
	{
		public string Receiver { get; private set; }

		public string Message { get; private set; }

		public IEnumerable<IAtom> List { get; private set; }

		public MessageEventArgs (string recv, string msg, int argc, IntPtr argv)
		{
			Receiver = recv;
			Message = msg;
			List = MessageInvocation.ConvertList (argc, argv);
		}
	}

}
