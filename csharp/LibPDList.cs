/*
 * Created by SharpDevelop.
 * User: Tebjan Halm
 * Date: 12.04.2012
 * Time: 00:08
 * 
 * 
 */
using System;

namespace LibPDBinding
{
		/// <summary>
	/// Handles list construction and sending
	/// </summary>
	public class LibPDList
	{
		public LibPDList(params object[] args)
		{
			Args = args;
		}
		
		/// <summary>
		/// Arguments of this message
		/// </summary>
		public object[] Args
		{
			get;
			private set;
		}
		
		/// <summary>
		/// How often the message/list was sent
		/// </summary>
		public int TimesSent
		{
			get;
			protected set;
		}
		
		/// <summary>
		/// Send this message to the receiver in PD
		/// </summary>
		/// <param name="receiver">Identifier of the receiver</param>
		public virtual void SendTo(string receiver)
		{
			LibPD.SendList(receiver, Args);
			TimesSent = TimesSent + 1;
		}
		
		public override string ToString()
		{
			return string.Format("{0} TimesSent={1}", String.Join(" ", Args), TimesSent);
		}
		
		/// <summary>
		/// Create a list from a list string
		/// </summary>
		/// <param name="message">list as string</param>
		/// <returns>New list</returns>
		public static LibPDList ParseList(string list)
		{
			var args = list.Split(new char[]{' '}, StringSplitOptions.RemoveEmptyEntries);
			return new LibPDList(args);
		}

	}
}
