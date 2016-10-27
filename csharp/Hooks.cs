using System;
using System.Runtime.InteropServices;

namespace LibPDBinding
{
	public delegate void LibPDPrint (string text);
	public delegate void LibPDBang (string recv);
	public delegate void LibPDFloat (string recv, float x);
	public delegate void LibPDSymbol (string recv, string sym);
	public delegate void LibPDList (string recv, object[] args);
	public delegate void LibPDMessage (string recv, string msg, object[] args);

	[UnmanagedFunctionPointer (CallingConvention.Cdecl)]
    internal delegate void LibPDPrintHook ([In] [MarshalAs (UnmanagedType.LPStr)] string text);

	[UnmanagedFunctionPointer (CallingConvention.Cdecl)]
    internal delegate void LibPDBangHook ([In] [MarshalAs (UnmanagedType.LPStr)] string recv);

	[UnmanagedFunctionPointer (CallingConvention.Cdecl)]
    internal delegate void LibPDFloatHook ([In] [MarshalAs (UnmanagedType.LPStr)] string recv, float x);

	[UnmanagedFunctionPointer (CallingConvention.Cdecl)]
    internal delegate void LibPDSymbolHook ([In] [MarshalAs (UnmanagedType.LPStr)] string recv, [In] [MarshalAs (UnmanagedType.LPStr)] string sym);

	[UnmanagedFunctionPointer (CallingConvention.Cdecl)]
    internal delegate void LibPDListHook ([In] [MarshalAs (UnmanagedType.LPStr)] string recv, int argc, IntPtr argv);

	[UnmanagedFunctionPointer (CallingConvention.Cdecl)]
    internal delegate void LibPDMessageHook ([In] [MarshalAs (UnmanagedType.LPStr)] string recv, [In] [MarshalAs (UnmanagedType.LPStr)] string msg, int argc, IntPtr argv);

}
