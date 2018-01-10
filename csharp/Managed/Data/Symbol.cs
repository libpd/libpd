using System;

namespace LibPDBinding.Managed.Data
{
	/// <summary>
	/// Pd symbol message.
	/// </summary>
	public class Symbol : IAtom<string>, IAtom
	{
		public string Value { get; private set; }

		object IAtom.Value {
			get {
				return Value;
			}
		}

		public Symbol (string sym)
		{
			Value = sym;
		}
	}
}