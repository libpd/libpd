using System;
using System.Runtime.InteropServices;

namespace LibPDBinding.Native
{
	static class Defines
	{
		public const string DllName = "libpdcsharp";
		public const CallingConvention CallingConvention = System.Runtime.InteropServices.CallingConvention.Cdecl;
	}
}