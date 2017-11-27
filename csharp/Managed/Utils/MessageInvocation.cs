//
// Messaging.cs
//
// Author:
//       thomas <${AuthorEmail}>
//
// Copyright (c) 2016 thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
using System;
using System.Runtime.InteropServices;
using LibPDBinding.Managed.Data;
using LibPDBinding.Native;

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
			SendArgs (args);
			int finish = Native.Messaging.finish_list (receiver);
			if (finish != 0) {
				throw new PdProcessException (finish, "finish_list");				
			}
		}

		static void SendArgs (IAtom[] args)
		{
			int startMessage = Native.Messaging.start_message (args.Length);
			if (startMessage != 0) {
				throw new PdProcessException (startMessage, "start_message");
			}
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

