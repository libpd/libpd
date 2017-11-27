using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using LibPDBinding.Managed.Data;
using LibPDBinding.Managed.Events;
using LibPDBinding.Managed.Utils;

namespace LibPDBinding.Managed
{
	/// <summary>
	/// Messaging in Pd.
	/// </summary>
	public sealed class Messaging : IDisposable
	{
		internal Messaging ()
		{
			SetupHooks ();
		}

		~Messaging ()
		{
			Dispose (false);
		}

		public void Dispose ()
		{
			Dispose (true);
			GC.SuppressFinalize (this);
		}

		void Dispose (bool disposing)
		{
			foreach (IntPtr pointer in _bindings.Values) {
				Native.Messaging.unbind (pointer);
			}
			Print = null;
			Bang = null;
			Float = null;
			Symbol = null;
			List = null;
			Message = null;
		}

		readonly Dictionary<string, IntPtr> _bindings = new Dictionary<string, IntPtr> ();

		/// <summary>
		/// Send a general message to the specified receiver with a range of atoms.
		/// </summary>
		/// <param name="receiver">Receiver.</param>
		/// <param name="message">Message.</param>
		/// <param name="atoms">Atoms.</param>
		[MethodImpl (MethodImplOptions.Synchronized)]
		public void Send (string receiver, string message, params IAtom[] atoms)
		{
			MessageInvocation.SendMessage (receiver, message, atoms);
		}

		/// <summary>
		/// Send a message to the specified receiver with a range of atoms.
		/// </summary>
		/// <param name="receiver">Receiver.</param>
		/// <param name="atoms">Atoms.</param>
		[MethodImpl (MethodImplOptions.Synchronized)]
		public void Send (string receiver, params IAtom[] atoms)
		{
			if (atoms.Length == 1) {
				MessageInvocation.Send (receiver, atoms [0]);
				return;
			}
			MessageInvocation.SendList (receiver, atoms);
		}

		/// <summary>
		/// Send a bang message to the specified receiver with a range of atoms.
		/// </summary>
		/// <param name="receiver">Receiver.</param>
		/// <param name="bang">Bang.</param>
		[MethodImpl (MethodImplOptions.Synchronized)]
		public void Send (string receiver, Bang bang)
		{
			MessageInvocation.SendBang (receiver);
		}

		/// <summary>
		/// Binds to get messages from the specified receiver.
		/// </summary>
		/// <param name="receiver">Receiver.</param>
		[MethodImpl (MethodImplOptions.Synchronized)]
		public void Bind (string receiver)
		{
			if (_bindings.ContainsKey (receiver)) {
				return;
			}
			IntPtr pointer = Native.Messaging.bind (receiver);
			_bindings.Add (receiver, pointer);
		}

		/// <summary>
		/// Unbinds from the specified receiver.
		/// </summary>
		/// <param name="receiver">Receiver.</param>
		[MethodImpl (MethodImplOptions.Synchronized)]
		public void Unbind (string receiver)
		{
			IntPtr pointer;
			if (!_bindings.TryGetValue (receiver, out pointer)) {
				return;
			}
			Native.Messaging.unbind (pointer);
			_bindings.Remove (receiver);
		}

		void RaisePrintEvent (string text)
		{
			if (Print != null) {
				Print (this, new PrintEventArgs (text));
			}
		}

		void RaiseBangEvent (string recv)
		{
			if (Bang != null) {
				Bang (this, new BangEventArgs (recv));
			}
		}

		void RaiseFloatEvent (string recv, float x)
		{
			if (Float != null) {
				Float (this, new FloatEventArgs (recv, x));
			}
		}

		void RaiseSymbolEvent (string recv, string sym)
		{
			if (Symbol != null) {
				Symbol (this, new SymbolEventArgs (recv, sym));
			}
		}

		void RaiseListEvent (string recv, int argc, IntPtr argv)
		{
			if (List != null) {
				List (this, new ListEventArgs (recv, argc, argv));
			}
		}

		void RaiseMessageEvent (string recv, string msg, int argc, IntPtr argv)
		{
			if (Message != null) {
				Message (this, new MessageEventArgs (recv, msg, argc, argv));
			}
		}

		/// <summary>
		/// Occurs when print is called in Pd.
		/// </summary>
		public event EventHandler<PrintEventArgs> Print;
		/// <summary>
		/// Occurs when a Bang message is received on a subscribed receiver.
		/// </summary>
		public event EventHandler<BangEventArgs> Bang;
		/// <summary>
		/// Occurs when a Float message is received on a subscribed receiver.
		/// </summary>
		public event EventHandler<FloatEventArgs> Float;
		/// <summary>
		/// Occurs when a Symbol message is received on a subscribed receiver.
		/// </summary>
		public event EventHandler<SymbolEventArgs> Symbol;
		/// <summary>
		/// Occurs when a List message is received on a subscribed receiver.
		/// </summary>
		public event EventHandler<ListEventArgs> List;
		/// <summary>
		/// Occurs when a general message is received on a subscribed receiver.
		/// </summary>
		public event EventHandler<MessageEventArgs> Message;

		LibPDPrintHook PrintHook;
		LibPDBangHook BangHook;
		LibPDFloatHook FloatHook;
		LibPDSymbolHook SymbolHook;
		LibPDListHook ListHook;
		LibPDMessageHook MessageHook;

		void SetupHooks ()
		{
			PrintHook = new LibPDPrintHook (RaisePrintEvent);
			Native.Messaging.set_printhook (PrintHook);

			BangHook = new LibPDBangHook (RaiseBangEvent);
			Native.Messaging.set_banghook (BangHook);

			FloatHook = new LibPDFloatHook (RaiseFloatEvent);
			Native.Messaging.set_floathook (FloatHook);

			SymbolHook = new LibPDSymbolHook (RaiseSymbolEvent);
			Native.Messaging.set_symbolhook (SymbolHook);

			ListHook = new LibPDListHook (RaiseListEvent);
			Native.Messaging.set_listhook (ListHook);

			MessageHook = new LibPDMessageHook (RaiseMessageEvent);
			Native.Messaging.set_messagehook (MessageHook);
		}
	}
}

