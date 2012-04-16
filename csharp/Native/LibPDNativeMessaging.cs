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
	

	//the receiver part of libpd
	public static partial class LibPD
	{
		
	
		
		#region Message sending
		
		//store bindings
		private static Dictionary<string, IntPtr> Bindings = new Dictionary<string, IntPtr>();
		
		/// <summary>
		/// If set to true each sent message will be written to Debug output
		/// </summary>
		public static bool WriteMessageToDebug
		{
			[MethodImpl(MethodImplOptions.Synchronized)]
			get;
			[MethodImpl(MethodImplOptions.Synchronized)]
			set;
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
			return ParseArgsStringUnsync(argsString);
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
		
		
		
		#endregion private
		
	}
}