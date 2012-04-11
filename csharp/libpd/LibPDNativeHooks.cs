/*
 * Created by SharpDevelop.
 * User: Tebjan Halm
 * Date: 11.04.2012
 * Time: 07:01
 * 
 * 
 */

using System.Runtime.InteropServices;

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

	/// Return Type: void
	///recv: char*
	///msg: char*
	///argc: int
	///argv: char*
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate void LibPDMessageStringHook([In] [MarshalAs(UnmanagedType.LPStr)] string recv, [In] [MarshalAs(UnmanagedType.LPStr)] string msg, int argc, [In] [MarshalAs(UnmanagedType.LPStr)] string argv);


	//the receiver part of libpd
	public static partial class LibPD
	{
		private static LibPDPrintHook PrintHook;
		private static LibPDMessageStringHook MessageHook;
		
		private static void SetupHooks()
		{
			PrintHook = new LibPDPrintHook(RaisePrintEvent);
			set_printhook(PrintHook);
			
			MessageHook = new LibPDMessageStringHook(RaiseMessageEvent);
			set_messagestrhook(MessageHook);
		}
		
		public static event LibPDPrintHook Print;
		public static event LibPDMessageStringHook Message;
		
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
            // Event will be null if there are no subscribers
            if (Print != null)
            {
                // Use the () operator to raise the event.
                Message(recv, msg, argc, argv);
            }
        }
	}
}