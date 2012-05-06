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
using System.Globalization;
using System.Runtime.InteropServices;
using System.Threading;
using System.Runtime.CompilerServices;

namespace LibPDBinding
{
	#region delegates
	
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	internal delegate void LibPDPrintHook([In] [MarshalAs(UnmanagedType.LPStr)] string text);
	
	public delegate void LibPDPrint(string text);
	
	#endregion delegates
	
	//the receiver part of libpd
	public static partial class LibPD
	{

		#region Events

		//private delegate field, holds the function pointer
		private static LibPDPrintHook PrintHook;

		//import hook set method
		[DllImport("libpdcsharp.dll", EntryPoint="libpd_set_printhook")]
		private static extern void set_printhook(LibPDPrintHook hook);
		
		private static void SetupHooks()
		{
			//create the delegate with the method to call
			PrintHook = new LibPDPrintHook(RaisePrintEvent);
			
			//assign the delegate as PDs printhook
			set_printhook(PrintHook);
		}

		/// <summary>
		/// Subscribe to this event in order to get PDs print messages.
		/// Note: Events can be raised by any thread. In multithreading 
		/// scenarios you need to synchronize the calls in the subscriber method.
		/// </summary>
		public static event LibPDPrint Print = delegate{};

		private static void RaisePrintEvent(string e)
		{
			Print(e);
		}

		#endregion Events
		
		#region Message sending
		
		/// <summary>
		/// If set to true each sent message will be written to Debug output
		/// </summary>
		public static bool WriteMessageToDebug
		{
			[MethodImpl(MethodImplOptions.Synchronized)]
			get
			{
				return SWriteMessageToDebug;
			}
			[MethodImpl(MethodImplOptions.Synchronized)]
			set
			{
				SWriteMessageToDebug = value;
			}
		}
		
		private static bool SWriteMessageToDebug;
		
		//sending-----------------------------------------------------------
				
		[DllImport("libpdcsharp.dll", EntryPoint="libpd_bang")]
		private static extern  int send_bang([In] [MarshalAs(UnmanagedType.LPStr)] string recv) ;
		
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

				
		[DllImport("libpdcsharp.dll", EntryPoint="libpd_float")]
		private static extern  int send_float([In] [MarshalAs(UnmanagedType.LPStr)] string recv, float x) ;
		
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
		
				
		[DllImport("libpdcsharp.dll", EntryPoint="libpd_symbol")]
		private static extern  int send_symbol([In] [MarshalAs(UnmanagedType.LPStr)] string recv, [In] [MarshalAs(UnmanagedType.LPStr)] string sym) ;
		
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
		
		
		[DllImport("libpdcsharp.dll", EntryPoint="libpd_start_message")]
		private static extern  int start_message(int max_length) ;

		[DllImport("libpdcsharp.dll", EntryPoint="libpd_finish_message")]
		private static extern  int finish_message([In] [MarshalAs(UnmanagedType.LPStr)] string recv, [In] [MarshalAs(UnmanagedType.LPStr)] string msg) ;
		
		/// <summary>
		/// Sends a message to an object in pd
		/// </summary>
		/// <param name="receiver"> </param>
		/// <param name="message"> </param>
		/// <param name="args"> list of arguments of type Integer, Float, or String </param>
		/// <returns> error code, 0 on success </returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int SendMessage(string receiver, string message, params object[] args)
		{
			var s = "";
			int err = ProcessArgs(args, ref s);
			var ret = (err == 0) ? finish_message(receiver, message) : err;
			
			if(SWriteMessageToDebug)
			{
				s = String.Format("Message: {0} {1}", receiver, message) + s;
				s += " Start: " + err;
				s += " End: " + ret;
				Debug.WriteLine(s);
			}
		
			return ret;
		}


		[DllImport("libpdcsharp.dll", EntryPoint="libpd_finish_list")]
		private static extern  int finish_list([In] [MarshalAs(UnmanagedType.LPStr)] string recv) ;
		
		/// <summary>
		/// Sends a list to an object in pd
		/// </summary>
		/// <param name="receiver"> </param>
		/// <param name="args"> list of arguments of type Integer, Float, or String </param>
		/// <returns> error code, 0 on success </returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int SendList(string receiver, params object[] args)
		{
			string s = "";
			int err = ProcessArgs(args, ref s);
			var ret = (err == 0) ? finish_list(receiver) : err;
			
			if(SWriteMessageToDebug)
			{
				s = String.Format("List: {0}", receiver) + s;
				s += " Start: " + err;
				s += " End: " + ret;
				Debug.WriteLine(s);
			}
			
			return ret;
		}
		
		
		[DllImport("libpdcsharp.dll", EntryPoint="libpd_add_float")]
		private static extern  void add_float(float x) ;
		
		[DllImport("libpdcsharp.dll", EntryPoint="libpd_add_symbol")]
		private static extern  void add_symbol([In] [MarshalAs(UnmanagedType.LPStr)] string sym) ;
		
		//parse args helper with debug string
		private static int ProcessArgs(object[] args, ref string debug)
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
					if(SWriteMessageToDebug) debug += " i:" + arg.ToString();
				}
				else if (arg is float?)
				{
					add_float((float)((float?) arg));
					if(SWriteMessageToDebug) debug += " f:" + arg.ToString();
				}
				else if (arg is double?)
				{
					add_float((float)((double?) arg));
					if(SWriteMessageToDebug) debug += " d:" + arg.ToString();
				}
				else if (arg is string)
				{
					add_symbol((string) arg);
					if(SWriteMessageToDebug) debug += " s:" + arg.ToString();
				}
				else
				{
					if(SWriteMessageToDebug) debug += " illegal argument: " + arg.ToString();
					return -101; // illegal argument
				}
			}
			return 0;
		}
		
		#endregion Message sending
		
	}
}