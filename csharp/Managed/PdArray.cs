using LibPDBinding.Native;

namespace LibPDBinding.Managed
{
	public class PdArray
	{
		readonly string _name;

		public string Name { get { return _name; } }

		internal PdArray (string name)
		{
			_name = name;
		}

		public int Size {
			get { return PInvoke.arraysize (_name); }
		}

		public float[] Read (int offset, int length)
		{
			float[] arrayContent = new float [length];
			int status = PInvoke.read_array (arrayContent, _name, offset, length);
			if (status != 0) {
				throw new PdProcessException (status, "read_array");
			}
			return arrayContent;
		}

		public void Write (float[] newContent, int offset, int length)
		{
			int status = PInvoke.write_array (_name, offset, newContent, length);
			if (status != 0) {
				throw new PdProcessException (status, "write_array");
			}
		}

		public void Resize (int length)
		{
			Messaging.SendMessage (_name, "resize", length);
		}
	}
}
