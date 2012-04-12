/*
 * Created by SharpDevelop.
 * User: Tebjan Halm
 * Date: 11.04.2012
 * Time: 07:01
 * 
 * 
 */

using System;
using System.Diagnostics;
using System.Globalization;
using System.Runtime.InteropServices;
using System.Threading;

namespace LibPDBinding
{

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
	public delegate void LibPDListStringHook([In] [MarshalAs(UnmanagedType.LPStr)] string recv, int argc, [In] [MarshalAsAttribute(UnmanagedType.LPStr)] string argv);

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
	public delegate void LibPDMessageStringHook([In] [MarshalAs(UnmanagedType.LPStr)] string recv, [In] [MarshalAs(UnmanagedType.LPStr)] string msg, int argc, [In] [MarshalAs(UnmanagedType.LPStr)] string argv);

	/// <summary>
	/// Message Event Delegate
	/// </summary>
	public delegate void LibPDMessageHook(string recv, string msg, object[] args);

	//the receiver part of libpd
	public static partial class LibPD
	{
		private static LibPDPrintHook PrintHook;
		private static LibPDBangHook BangHook;
		private static LibPDFloatHook FloatHook;
		private static LibPDSymbolHook SymbolHook;
		private static LibPDListStringHook ListHook;
		private static LibPDMessageStringHook MessageHook;
		
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
		
		/// Return Type: void
		///hook: t_libpd_printhook
		[DllImport("libpd.dll", EntryPoint="libpd_set_printhook")]
		private static extern  void set_printhook(LibPDPrintHook hook) ;
		

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
		
		/// <summary>
		/// 
		/// </summary>
		/// <param name="argsString"></param>
		/// <returns></returns>
		public static object[] ParseArgsString(string argsString)
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
		
	}
}