using System;
using LibPDBinding.Native;

namespace LibPDBinding.Managed
{
	/// <summary>
	/// Pd Patch.
	/// </summary>
	public sealed class Patch : IDisposable
	{
		readonly IntPtr _handle;

		internal Patch (IntPtr handle)
		{
			_handle = handle;
			DollarZero = General.getdollarzero (_handle);
		}

		~Patch ()
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
			General.closefile (_handle);
		}

		/// <summary>
		/// Gets $0 of Pd patch.
		/// </summary>
		/// <value>The dollar zero.</value>
		public int DollarZero {	get; private set; }
	}

}
