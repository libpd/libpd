using LibPDBinding.Managed.Data;
using LibPDBinding.Native;
using System;
using System.Runtime.InteropServices;

namespace LibPDBinding.Managed.Utils
{
	static class MessageInvocation
	{
		public static void SendMessage (string receiver, string message, params IAtom[] args)
		{
			SendArgs (args);
			int finish = Native.Messaging.finish_message (receiver, message);

			if (finish != 0) {
				throw new PdProcessException (finish, "finish_message");
			}
		}

		public static void SendBang (string receiver)
		{
			int finish = Native.Messaging.send_bang (receiver);
			if (finish != 0) {
				throw new PdProcessException (finish, "send_bang");
			}
			return;
		}

		public static void Send (string receiver, IAtom atom)
		{
			if (atom is Float) {
				int finish = Native.Messaging.send_float (receiver, (float)atom.Value);
				if (finish != 0) {
					throw new PdProcessException (finish, "send_float");
				}
				return;
			}
			if (atom is Symbol) {
				int finish = Native.Messaging.send_symbol (receiver, (string)atom.Value);
				if (finish != 0) {
					throw new PdProcessException (finish, "send_symbol");
				}
				return;
			}
		}

		public static void SendList (string receiver, IAtom[] args)
		{
			int startMessage = Native.Messaging.start_message (args.Length);
			if (startMessage != 0) {
				throw new PdProcessException (startMessage, "start_message");
			}
			SendArgs (args);
			int finish = Native.Messaging.finish_list (receiver);
			if (finish != 0) {
				throw new PdProcessException (finish, "finish_list");				
			}
		}

		static void SendArgs (IAtom[] args)
		{
			foreach (IAtom arg in args) {
				if (arg is Float) {
					Native.Messaging.add_float (((Float)arg).Value);
				} else if (arg is Symbol) {
					Native.Messaging.add_symbol (((Symbol)arg).Value);
				}
			}
		}

		public static IAtom[] ConvertList (int argc, IntPtr argv)
		{
			var args = new IAtom[argc];

			for (int i = 0; i < argc; i++) {
				if (i != 0)
					argv = Native.Messaging.next_atom (argv);

				if (Native.Messaging.atom_is_float (argv) != 0) {
					args [i] = new Float (Native.Messaging.atom_get_float (argv));
				} else if (Native.Messaging.atom_is_symbol (argv) != 0) {
					args [i] = new Symbol (Marshal.PtrToStringAnsi (Native.Messaging.atom_get_symbol (argv)));
				}
			}

			return args;
		}
	}
}

