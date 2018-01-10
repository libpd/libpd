using System;

namespace LibPDBinding.Managed.Data
{
	/// <summary>
	/// Pd float message.
	/// </summary>
	public class Float : IAtom<float>, IAtom
	{
		public float Value { get; private set; }

		object IAtom.Value {
			get {
				return Value;
			}
		}

		public Float (float value)
		{
			Value = value;
		}
	}
}