using LibPDBinding.Managed.Data;
using System;

namespace LibPDBinding.Managed.Events
{
	public class FloatEventArgs : EventArgs
	{
		public string Receiver { get; private set; }

		public Float Float { get; private set; }

		public FloatEventArgs (string text, float value)
		{
			Receiver = text;
			Float = new Float (value);
		}
	}
}
