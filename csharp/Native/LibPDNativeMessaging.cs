/*
 * Created by SharpDevelop.
 * User: Tebjan Halm
 * Date: 11.04.2012
 * Time: 07:01
 * 
 * 
 */

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Runtime.InteropServices;
using System.Threading;
using System.Runtime.CompilerServices;

namespace LibPDBinding
{
	#region Delegates

	/// Return Type: void
	///recv: charr
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate void LibPDPrintHook([In] [MarshalAs(UnmanagedType.LPStr)] string recv);

	/// Return Type: void
	///recv: char*
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate void LibPDBangHook([In] [MarshalAs(UnmanagedType.LPStr)] string recv);

	/// Return Type: void
	///recv: char*
	///x: float
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate void LibPDFloatHook([In] [MarshalAs(UnmanagedType.LPStr)] string recv, float x);

	/// Return Type: void
	///recv: char*
	///sym: char*
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate void LibPDSymbolHook([In] [MarshalAs(UnmanagedType.LPStr)] string recv, [In] [MarshalAs(UnmanagedType.LPStr)] string sym);

	/// Return Type: void
	///recv: char*
	///argc: int
	///argv: char*
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	internal delegate void LibPDListStringHook([In] [MarshalAs(UnmanagedType.LPStr)] string recv, int argc, [In] [MarshalAsAttribute(UnmanagedType.LPStr)] string argv);

	/// <summary>
	/// List Event Delegate
	/// </summary>
	public delegate void LibPDListHook(string recv, object[] args);
	
	/// Return Type: void
	///recv: char*
	///msg: char*
	///argc: int
	///argv: char*
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	internal delegate void LibPDMessageStringHook([In] [MarshalAs(UnmanagedType.LPStr)] string recv, [In] [MarshalAs(UnmanagedType.LPStr)] string msg, int argc, [In] [MarshalAs(UnmanagedType.LPStr)] string argv);

	/// <summary>
	/// Message Event Delegate
	/// </summary>
	public delegate void LibPDMessageHook(string recv, string msg, object[] args);
	
	#endregion Delegates

	//the receiver part of libpd
	public static partial class LibPD
	{
		public static bool WriteMessageToDebug
		{
			[MethodImpl(MethodImplOptions.Synchronized)]
			get;
			[MethodImpl(MethodImplOptions.Synchronized)]
			set;
		}
		
		private static LibPDPrintHook PrintHook;
		private static LibPDBangHook BangHook;
		private static LibPDFloatHook FloatHook;
		private static LibPDSymbolHook SymbolHook;
		private static LibPDListStringHook ListHook;
		private static LibPDMessageStringHook MessageHook;
		
		private static Dictionary<string, IntPtr> Bindings = new Dictionary<string, IntPtr>();
		
		private static void SetupHooks()
		{
			PrintHook = new LibPDPrintHook(RaisePrintEvent);
			set_printhook(PrintHook);
			
			BangHook = new LibPDBangHook(RaiseBangEvent);
			set_banghook(BangHook);
			
			FloatHook = new LibPDFloatHook(RaiseFloatEvent);
			set_floathook(FloatHook);
			
			SymbolHook = new LibPDSymbolHook(RaiseSymbolEvent);
			set_symbolhook(SymbolHook);
			
			ListHook = new LibPDListStringHook(RaiseListEvent);
			set_liststrhook(ListHook);
			
			MessageHook = new LibPDMessageStringHook(RaiseMessageEvent);
			set_messagestrhook(MessageHook);
		}
		
		public static event LibPDPrintHook Print;
		public static event LibPDBangHook Bang;
		public static event LibPDFloatHook Float;
		public static event LibPDSymbolHook Symbol;
		public static event LibPDListHook List;
		public static event LibPDMessageHook Message;
		
		#region Rise events

		private static void RaisePrintEvent(string e)
        {
            // Event will be null if there are no subscribers
            if (Print != null)
            {
                // Use the () operator to raise the event.
                Print(e);
            }
        }
		
		private static void RaiseBangEvent(string recv)
		{
			if (Bang != null)
			{
				Bang(recv);
			}
		}
		
		private static void RaiseFloatEvent(string recv, float e)
		{
			if (Float != null)
			{
				Float(recv, e);
			}
		}
		
		private static void RaiseSymbolEvent(string recv, string e)
		{
			if (Symbol != null)
			{
				Symbol(recv, e);
			}
		}
		
		private static void RaiseMessageEvent(string recv, string msg, int argc, string argv)
		{
			if (Message != null)
			{
				var args = ParseArgsString(argv);
				
				if(args.Length != argc) Debug.WriteLine("Message string parsing got {3} objects but should have {4}: {0} {1} {2}", recv, msg, argv, args.Length, argc);
				
				Message(recv, msg, args);
			}
		}
		
		private static void RaiseListEvent(string recv, int argc, string argv)
		{
			if (List != null)
			{
				var args = ParseArgsString(argv);
				
				if(args.Length != argc) Debug.WriteLine("List string parsing got {2} objects but should have {3}: {0} {1}", recv, argv, args.Length, argc);
				
				List(recv, args);
			}
		}
		
		#endregion Rise events
		
		#region Message sending
		
		//binding-----------------------------------
		
		/// <summary>
		/// subscribes to pd messages sent to the given symbol
		/// </summary>
		/// <param name="symbol"> </param>
		/// <returns> true on success </returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static bool Subscribe(string sym)
		{
			if(Bindings.ContainsKey(sym)) return true;
			
			var ptr = bind(sym);
			
			if(ptr == IntPtr.Zero) return false;
			
			Bindings[sym] = ptr;
			return true;
		}
		
		/// <summary>
		/// unsubscribes from pd messages sent to the given symbol; will do nothing
		/// if there is no subscription to this symbol
		/// </summary>
		/// <param name="sym"> </param>
		/// <returns>true if unsubscribed</returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static bool Unsubscribe(string sym)
		{
			if(!Bindings.ContainsKey(sym)) return false;
			unbind(Bindings[sym]);
			return Bindings.Remove(sym);
		}
		
		//sending-----------------------------------------------------------
		
		/// <summary>
		/// sends a bang to the object associated with the given symbol
		/// </summary>
		/// <param name="recv">
		///            symbol associated with receiver </param>
		/// <returns> error code, 0 on success </returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int SendBang(string recv)
		{
			return send_bang(recv);
		}

		/// <summary>
		/// sends a float to the object associated with the given symbol
		/// </summary>
		/// <param name="recv">
		///            symbol associated with receiver </param>
		/// <param name="x"> </param>
		/// <returns> error code, 0 on success </returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int SendFloat(string recv, float x)
		{
			return send_float(recv, x);
		}
		
		/// <summary>
		/// sends a symbol to the object associated with the given symbol
		/// </summary>
		/// <param name="recv">
		///            symbol associated with receiver </param>
		/// <param name="sym"> </param>
		/// <returns> error code, 0 on success </returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int SendSymbol(string recv, string sym)
		{
			return send_symbol(recv, sym);
		}
		
		/// <summary>
		/// Sends a message to an object in pd
		/// </summary>
		/// <param name="receiver"> </param>
		/// <param name="message"> </param>
		/// <param name="args">
		///            list of arguments of type Integer, Float, or String; no more
		///            than 32 arguments </param>
		/// <returns> error code, 0 on success </returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int SendMessage(string receiver, string message, params object[] args)
		{
			if(WriteMessageToDebug)
			{
				var s = String.Format("Message: {0} {1}", receiver, message);
				int err = ProcessArgsDebug(args, ref s);
				var ret = (err == 0) ? finish_message(receiver, message) : err;
				s += " Start: " + err;
				s += " End: " + ret;
				Debug.WriteLine(s);
				return ret;
			}
			else
			{
				int err = ProcessArgs(args);
				return (err == 0) ? finish_message(receiver, message) : err;
			}
		}
		
		/// <summary>
		/// Sends a list to an object in pd
		/// </summary>
		/// <param name="receiver"> </param>
		/// <param name="args">
		///            list of arguments of type Integer, Float, or String; no more
		///            than 32 arguments </param>
		/// <returns> error code, 0 on success </returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int SendList(string receiver, params object[] args)
		{
			if(WriteMessageToDebug)
			{
				var s = String.Format("List: {0}", receiver);
				int err = ProcessArgsDebug(args, ref s);
				var ret = (err == 0) ? finish_list(receiver) : err;
				s += " Start: " + err;
				s += " End: " + ret;
				Debug.WriteLine(s);
				return ret;
			}
			else
			{
				int err = ProcessArgs(args);
				return (err == 0) ? finish_list(receiver) : err;
			}
		}
		
		/// <summary>
		/// Get an object array from a space seperated message string threadsafe
		/// </summary>
		/// <param name="argsString"></param>
		/// <returns></returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static object[] ParseArgsString(string argsString)
		{
			return ParseArgsString(argsString);
		}
		
		/// <summary>
		/// Get an object array from a space seperated message string
		/// This method is not synchronized for performance reasons
		/// </summary>
		/// <param name="argsString"></param>
		/// <returns></returns>
		public static object[] ParseArgsStringUnsync(string argsString)
		{
			var args = argsString.Split(new char[]{' '}, StringSplitOptions.RemoveEmptyEntries);
			var ret = new object[args.Length];
			
			var previousCulture = Thread.CurrentThread.CurrentCulture;
            Thread.CurrentThread.CurrentCulture = CultureInfo.InvariantCulture;
            
            for(int i=0; i<args.Length; i++)
			{
            	var s = args[i];
				float f;
				if(float.TryParse(s, out f))
				{
					ret[i] = f;
				}
				else
				{
					ret[i] = s;
				}
			}

			Thread.CurrentThread.CurrentCulture = previousCulture;
			
			return ret;
		}
		
		#endregion Message sending
		
		#region private
		
		[DllImport("libpd.dll", EntryPoint="libpd_bind")]
		private static extern IntPtr bind([In] [MarshalAs(UnmanagedType.LPStr)] string sym) ;
		
		/// Return Type: void
		///p: void*
		[DllImport("libpd.dll", EntryPoint="libpd_unbind")]
		private static extern void unbind(IntPtr p) ;
		
		[DllImport("libpd.dll", EntryPoint="libpd_bang")]
		private static extern  int send_bang([In] [MarshalAs(UnmanagedType.LPStr)] string recv) ;
		
		[DllImport("libpd.dll", EntryPoint="libpd_float")]
		private static extern  int send_float([In] [MarshalAs(UnmanagedType.LPStr)] string recv, float x) ;
		
		[DllImport("libpd.dll", EntryPoint="libpd_symbol")]
		private static extern  int send_symbol([In] [MarshalAs(UnmanagedType.LPStr)] string recv, [In] [MarshalAs(UnmanagedType.LPStr)] string sym) ;
		
		/// Return Type: int
		///max_length: int
		[DllImport("libpd.dll", EntryPoint="libpd_start_message")]
		private static extern  int start_message(int max_length) ;
		
		/// Return Type: void
		///x: float
		[DllImport("libpd.dll", EntryPoint="libpd_add_float")]
		private static extern  void add_float(float x) ;

		
		/// Return Type: void
		///sym: char*
		[DllImport("libpd.dll", EntryPoint="libpd_add_symbol")]
		private static extern  void add_symbol([In] [MarshalAs(UnmanagedType.LPStr)] string sym) ;

		
		/// Return Type: int
		///recv: char*
		[DllImport("libpd.dll", EntryPoint="libpd_finish_list")]
		private static extern  int finish_list([In] [MarshalAs(UnmanagedType.LPStr)] string recv) ;

		
		/// Return Type: int
		///recv: char*
		///msg: char*
		[DllImport("libpd.dll", EntryPoint="libpd_finish_message")]
		private static extern  int finish_message([In] [MarshalAs(UnmanagedType.LPStr)] string recv, [In] [MarshalAs(UnmanagedType.LPStr)] string msg) ;

		
		//parse args helper
		private static int ProcessArgs(object[] args)
		{
			if (start_message(args.Length) != 0)
			{
				return -100;
			}
			foreach (object arg in args)
			{
				if (arg is int?)
				{
					add_float((int)((int?) arg));
				}
				else if (arg is float?)
				{
					add_float((float)((float?) arg));
				}
				else if (arg is double?)
				{
					add_float((float)((double?) arg));
				}
				else if (arg is string)
				{
					add_symbol((string) arg);
				}
				else
				{
					return -101; // illegal argument
				}
			}
			return 0;
		}
		
		//parse args helper with debug string
		private static int ProcessArgsDebug(object[] args, ref string debug)
		{
			if (start_message(args.Length) != 0)
			{
				return -100;
			}
			foreach (object arg in args)
			{
				if (arg is int?)
				{
					add_float((int)((int?) arg));
					debug += " i:" + arg.ToString();
				}
				else if (arg is float?)
				{
					add_float((float)((float?) arg));
					debug += " f:" + arg.ToString();
				}
				else if (arg is double?)
				{
					add_float((float)((double?) arg));
					debug += " d:" + arg.ToString();
				}
				else if (arg is string)
				{
					add_symbol((string) arg);
					debug += " s:" + arg.ToString();
				}
				else
				{
					debug += " illegal argument: " + arg.ToString();
					return -101; // illegal argument
				}
			}
			return 0;
		}
		
		/// Return Type: void
		///hook: t_libpd_printhook
		[DllImport("libpd.dll", EntryPoint="libpd_set_printhook")]
		private static extern  void set_printhook(LibPDPrintHook hook) ;
		
		/// Return Type: void
		///hook: t_libpd_banghook
		[DllImport("libpd.dll", EntryPoint="libpd_set_banghook")]
		private static extern  void set_banghook(LibPDBangHook hook) ;

		
		/// Return Type: void
		///hook: t_libpd_floathook
		[DllImport("libpd.dll", EntryPoint="libpd_set_floathook")]
		private static extern  void set_floathook(LibPDFloatHook hook) ;

		
		/// Return Type: void
		///hook: t_libpd_symbolhook
		[DllImport("libpd.dll", EntryPoint="libpd_set_symbolhook")]
		private static extern  void set_symbolhook(LibPDSymbolHook hook) ;

		
		/// Return Type: void
		///hook: t_libpd_listhook
		[DllImport("libpd.dll", EntryPoint="libpd_set_liststrhook")]
		private static extern  void set_liststrhook(LibPDListStringHook hook) ;

		
		/// Return Type: void
		///hook: t_libpd_messagehook
		[DllImport("libpd.dll", EntryPoint="libpd_set_messagestrhook")]
		private static extern  void set_messagestrhook(LibPDMessageStringHook hook) ;
		
		#endregion private
		
	}
}