using System;

namespace LibPDBinding.Managed
{
	public sealed class PdProcessException : Exception
	{
		public int ErrorCode { get; private set; }

		internal PdProcessException (int errorCode, string cFunction)
		{
			ErrorCode = errorCode;
			CFunctionCall = cFunction;
		}

		public string CFunctionCall { get; private set; }
	}
}
