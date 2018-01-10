using System;

namespace LibPDBinding.Managed.Data
{
	public interface IAtom<T>
	{
		T Value {get;}
	}

	public interface IAtom
	{
		object Value {get;}		
	}
}

