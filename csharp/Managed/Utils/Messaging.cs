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
using LibPDBinding.Native;
using LibPDBinding.Managed;

namespace LibPDBinding
{
	public static class Messaging
	{
		public static void SendMessage (string receiver, string message, params object[] args)
		{

			SendArgs (args);
			int finish = PInvoke.finish_message (receiver, message);

			if (finish != 0) {
				throw new PdProcessException (finish, "finish_message");
			}
		}

		private static void SendArgs (object[] args)
		{
			int startMessage = PInvoke.start_message (args.Length);
			if (startMessage != 0) {
				throw new PdProcessException (startMessage, "start_message");
			}
			foreach (object arg in args) {
				if (arg is int?) {
					PInvoke.add_float ((int)((int?)arg));
				} else if (arg is float?) {
					PInvoke.add_float ((float)((float?)arg));
				} else if (arg is double?) {
					PInvoke.add_float ((float)((double?)arg));
				} else if (arg is string) {
					PInvoke.add_symbol ((string)arg);
				} else {
					throw new ArgumentOutOfRangeException ("args[]", arg, "Argument is of wrong type.");
				}
			}
		}
	}
}

