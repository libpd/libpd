using LibPDBinding.Managed.Data;
using LibPDBinding.Managed.Utils;
using LibPDBinding.Native;

namespace LibPDBinding.Managed
{
	/// <summary>
	/// Array in Pd.
	/// </summary>
	public sealed class PdArray
	{
		readonly string _name;
		readonly Pd _pd;

		/// <summary>
		/// Gets the name of the array in Pd.
		/// </summary>
		/// <value>The name.</value>
		public string Name { get { return _name; } }

		internal PdArray (Pd pd, string name)
		{
			_pd = pd;
			_name = name;
		}

		/// <summary>
		/// Gets the size of the Pd array.
		/// </summary>
		/// <value>The size.</value>
		public int Size {
			get { 
				_pd.Activate ();
				return Audio.arraysize (_name);
			}
		}

		/// <summary>
		/// Read values from the array.
		/// </summary>
		/// <param name="start">Start position for reading.</param>
		/// <param name="length">Number of values to be read.</param>
		public float[] Read (int start, int length)
		{
			_pd.Activate ();
			float[] arrayContent = new float [length];
			int status = Audio.read_array (arrayContent, _name, start, length);
			if (status != 0) {
				throw new PdProcessException (status, "read_array");
			}
			return arrayContent;
		}

		/// <summary>
		/// Writes values to the array.
		/// </summary>
		/// <param name="newContent">New content to be written.</param>
		/// <param name="start">Start position for writing.</param>
		/// <param name="length">Number of values to be written.</param>
		public void Write (float[] newContent, int start, int length)
		{
			_pd.Activate ();
			int status = Audio.write_array (_name, start, newContent, length);
			if (status != 0) {
				throw new PdProcessException (status, "write_array");
			}
		}

		/// <summary>
		/// Resizes the Pd array. 
		/// 
		/// NB: This is an expensive method, use sparingly.
		/// </summary>
		/// <param name="length">The new size of the array.</param>
		public void Resize (int length)
		{
			_pd.Activate ();
			MessageInvocation.SendMessage (_name, "resize", new Float (length));
		}
	}
}
