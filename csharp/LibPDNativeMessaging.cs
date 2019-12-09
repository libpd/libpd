/*
 * 
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 * 
 * 
 * Created by SharpDevelop.
 * User: Tebjan Halm
 * Date: 11.04.2012
 * Time: 07:01
 * 
 */

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using LibPDBinding.Native;

namespace LibPDBinding
{
	//the receiver part of libpd
	public static partial class LibPD
	{
		#region Events

		//private delegate field, holds the function pointer
		private static LibPDPrintHook PrintHook;
		private static LibPDBangHook BangHook;
		private static LibPDFloatHook FloatHook;
		private static LibPDSymbolHook SymbolHook;
		private static LibPDListHook ListHook;
		private static LibPDMessageHook MessageHook;
		
		//import hook set method

		private static void SetupHooks ()
		{
			//create the delegate with the method to call
			PrintHook = new LibPDPrintHook (RaisePrintEvent);
			Messaging.set_printhook (PrintHook);
			
			BangHook = new LibPDBangHook (RaiseBangEvent);
			Messaging.set_banghook (BangHook);

			FloatHook = new LibPDFloatHook (RaiseFloatEvent);
			Messaging.set_floathook (FloatHook);

			SymbolHook = new LibPDSymbolHook (RaiseSymbolEvent);
			Messaging.set_symbolhook (SymbolHook);
			
			ListHook = new LibPDListHook (RaiseListEvent);
			Messaging.set_listhook (ListHook);
			
			MessageHook = new LibPDMessageHook (RaiseMessageEvent);
			Messaging.set_messagehook (MessageHook);
			
		}

		/// <summary>
		/// Subscribe to this event in order to get PDs print messages.
		/// Note: Events may be raised by several threads, such as the GUI thread and 
		/// the audio thread. If a subscriber method calls operations that must be executed 
		/// in a particular thread, then the subscriber method is responsible for posting 
		/// those calls to the appropriate synchronization context.
		/// </summary>
		[Obsolete("Use LibPDBinding.Managed.Messaging.Print")]
		public static event LibPDPrint Print = delegate{};
		
		/// <summary>
		/// Subscribe to this event in order to get PDs bang messages.
		/// Note: Events may be raised by several threads, such as the GUI thread and 
		/// the audio thread. If a subscriber method calls operations that must be executed 
		/// in a particular thread, then the subscriber method is responsible for posting 
		/// those calls to the appropriate synchronization context.
		/// </summary>
		[Obsolete("Use LibPDBinding.Managed.Messaging.Bang")]
		public static event LibPDBang Bang = delegate{};
		
		/// <summary>
		/// Subscribe to this event in order to get PDs float messages.
		/// Note: Events may be raised by several threads, such as the GUI thread and 
		/// the audio thread. If a subscriber method calls operations that must be executed 
		/// in a particular thread, then the subscriber method is responsible for posting 
		/// those calls to the appropriate synchronization context.
		/// </summary>
		[Obsolete("Use LibPDBinding.Managed.Messaging.Float")]
		public static event LibPDFloat Float = delegate{};
		
		/// <summary>
		/// Subscribe to this event in order to get PDs symbol messages.
		/// Note: Events may be raised by several threads, such as the GUI thread and 
		/// the audio thread. If a subscriber method calls operations that must be executed 
		/// in a particular thread, then the subscriber method is responsible for posting 
		/// those calls to the appropriate synchronization context.
		/// </summary>
		[Obsolete("Use LibPDBinding.Managed.Messaging.Symbol")]
		public static event LibPDSymbol Symbol = delegate{};
		
		/// <summary>
		/// Subscribe to this event in order to get PDs list messages. Currently only
		/// float and symbol types are supported. Other types in the list such as pointers will be null.
		/// Note: Events may be raised by several threads, such as the GUI thread and 
		/// the audio thread. If a subscriber method calls operations that must be executed 
		/// in a particular thread, then the subscriber method is responsible for posting 
		/// those calls to the appropriate synchronization context.
		/// </summary>
		[Obsolete("Use LibPDBinding.Managed.Messaging.List")]
		public static event LibPDList List = delegate{};
		
		/// <summary>
		/// Subscribe to this event in order to get PDs typed messages. Currently only
		/// float and symbol types are supported. Other types in the list such as pointers will be null.
		/// Note: Events may be raised by several threads, such as the GUI thread and 
		/// the audio thread. If a subscriber method calls operations that must be executed 
		/// in a particular thread, then the subscriber method is responsible for posting 
		/// those calls to the appropriate synchronization context.
		/// </summary>
		[Obsolete("Use LibPDBinding.Managed.Messaging.Message")]
		public static event LibPDMessage Message = delegate{};

		private static void RaisePrintEvent (string e)
		{
			Print (e);
		}

		private static void RaiseBangEvent (string recv)
		{
			Bang (recv);
		}

		private static void RaiseFloatEvent (string recv, float e)
		{
			Float (recv, e);
		}

		private static void RaiseSymbolEvent (string recv, string e)
		{
			Symbol (recv, e);
		}

		private static void RaiseListEvent (string recv, int argc, IntPtr argv)
		{
			List (recv, ConvertList (argc, argv));
		}

		private static void RaiseMessageEvent (string recv, string msg, int argc, IntPtr argv)
		{
			Message (recv, msg, ConvertList (argc, argv));
		}

		private static object[] ConvertList (int argc, IntPtr argv)
		{
			var args = new object[argc];
			
			for (int i = 0; i < argc; i++) {
				if (i != 0)
					argv = Messaging.next_atom (argv);
				
				if (Messaging.atom_is_float (argv) != 0) {
					args [i] = Messaging.atom_get_float (argv);
				} else if (Messaging.atom_is_symbol (argv) != 0) {
					args [i] = Marshal.PtrToStringAnsi (Messaging.atom_get_symbol (argv));
				}
			}
			
			return args;
		}

		#endregion Events

		#region Message sending

		/// <summary>
		/// If set to true each sent message will be written to Debug output
		/// </summary>
		[Obsolete]
		public static bool WriteMessageToDebug {
			[MethodImpl(MethodImplOptions.Synchronized)]
			get {
				return SWriteMessageToDebug;
			}
			[MethodImpl(MethodImplOptions.Synchronized)]
			set {
				SWriteMessageToDebug = value;
			}
		}

		private static bool SWriteMessageToDebug;
		
		//binding-----------------------------------
		
		//store bindings
		private static Dictionary<string, IntPtr> Bindings = new Dictionary<string, IntPtr> ();

		/// <summary>
		/// subscribes to pd messages sent to the given symbol
		/// </summary>
		/// <param name="symbol"> </param>
		/// <returns> true on success </returns>
		[MethodImpl (MethodImplOptions.Synchronized)]
		[Obsolete("Use LibPDBinding.Managed.Messaging.Subscribe()")]
		public static bool Subscribe (string sym)
		{
			if (String.IsNullOrEmpty (sym))
				return false;
			if (Bindings.ContainsKey (sym))
				return true;

			var ptr = Messaging.bind (sym);

			if (ptr == IntPtr.Zero)
				return false;

			Bindings [sym] = ptr;
			return true;
		}

		/// <summary>
		/// unsubscribes from pd messages sent to the given symbol; will do nothing
		/// if there is no subscription to this symbol
		/// </summary>
		/// <param name="sym"> </param>
		/// <returns>true if unsubscribed</returns>
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static bool Unsubscribe (string sym)
		{
			if (String.IsNullOrEmpty (sym) || !Bindings.ContainsKey (sym))
				return false;
			Messaging.unbind (Bindings [sym]);
			return Bindings.Remove (sym);
		}

		
		//sending-----------------------------------------------------------

		/// <summary>
		/// sends a bang to the object associated with the given symbol
		/// </summary>
		/// <param name="recv">
		///            symbol associated with receiver </param>
		/// <returns> error code, 0 on success </returns>
		[Obsolete("Use LibPDBinding.Managed.Messaging.Send()")]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static int SendBang (string recv)
		{
			return Messaging.send_bang (recv);
		}


		/// <summary>
		/// sends a float to the object associated with the given symbol
		/// </summary>
		/// <param name="recv">
		///            symbol associated with receiver </param>
		/// <param name="x"> </param>
		/// <returns> error code, 0 on success </returns>
		[Obsolete("Use LibPDBinding.Managed.Messaging.Send()")]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static int SendFloat (string recv, float x)
		{
			return Messaging.send_float (recv, x);
		}

		/// <summary>
		/// sends a symbol to the object associated with the given symbol
		/// </summary>
		/// <param name="recv">
		///            symbol associated with receiver </param>
		/// <param name="sym"> </param>
		/// <returns> error code, 0 on success </returns>
		[Obsolete("Use LibPDBinding.Managed.Messaging.Send()")]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static int SendSymbol (string recv, string sym)
		{
			return Messaging.send_symbol (recv, sym);
		}


		/// <summary>
		/// Sends a message to an object in pd
		/// </summary>
		/// <param name="receiver"> </param>
		/// <param name="message"> </param>
		/// <param name="args"> list of arguments of type Integer, Float, or String </param>
		/// <returns> error code, 0 on success </returns>
		[Obsolete("Use LibPDBinding.Managed.Messaging.Send()")]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static int SendMessage (string receiver, string message, params object[] args)
		{
			var s = "";
			int err = ProcessArgs (args, ref s);
			var ret = (err == 0) ? Messaging.finish_message (receiver, message) : err;
			
			if (SWriteMessageToDebug) {
				s = String.Format ("Message: {0} {1}", receiver, message) + s;
				s += " Start: " + err;
				s += " End: " + ret;
				Debug.WriteLine (s);
			}
		
			return ret;
		}


		/// <summary>
		/// Sends a list to an object in pd
		/// </summary>
		/// <param name="receiver"> </param>
		/// <param name="args"> list of arguments of type Integer, Float, or String </param>
		/// <returns> error code, 0 on success </returns>
		[Obsolete("Use LibPDBinding.Managed.Messaging.Send()")]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static int SendList (string receiver, params object[] args)
		{
			string s = "";
			int err = ProcessArgs (args, ref s);
							
			var ret = (err == 0) ? Messaging.finish_list (receiver) : err;
				
			
			if (SWriteMessageToDebug) {
				s = String.Format ("List: {0}", receiver) + s;
				s += " Start: " + err;
				s += " End: " + ret;
				Debug.WriteLine (s);
			}
			
			return ret;
		}

		//parse args helper with debug string
		private static int ProcessArgs (object[] args, ref string debug)
		{
			if (Messaging.start_message (args.Length) != 0) {
				return -100;
			}
			foreach (object arg in args) {
				if (arg is int?) {
					Messaging.add_float ((int)((int?)arg));
					if (SWriteMessageToDebug)
						debug += " i:" + arg.ToString ();
				} else if (arg is float?) {
					Messaging.add_float ((float)((float?)arg));
					if (SWriteMessageToDebug)
						debug += " f:" + arg.ToString ();
				} else if (arg is double?) {
					Messaging.add_float ((float)((double?)arg));
					if (SWriteMessageToDebug)
						debug += " d:" + arg.ToString ();
				} else if (arg is string) {
					Messaging.add_symbol ((string)arg);
					if (SWriteMessageToDebug)
						debug += " s:" + arg.ToString ();
				} else {
					if (SWriteMessageToDebug)
						debug += " illegal argument: " + arg.ToString ();
					return -101; // illegal argument
				}
			}
			return 0;
		}

		#endregion Message sending
		
	}
}
