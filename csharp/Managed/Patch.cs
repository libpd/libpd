using LibPDBinding.Native;
using System;

namespace LibPDBinding.Managed
{
	/// <summary>
	/// Pd Patch.
	/// </summary>
	public sealed class Patch : IDisposable
	{
		readonly IntPtr _handle;
		readonly Pd _pd;

		internal Patch (IntPtr handle, Pd pd)
		{
			_handle = handle;
			_pd = pd;
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
			_pd.Activate ();
			General.closefile (_handle);
		}

		/// <summary>
		/// Gets $0 of Pd patch.
		/// </summary>
		/// <value>The dollar zero.</value>
		public int DollarZero {	get; private set; }
	}

}
