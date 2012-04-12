/*
 * Created by SharpDevelop.
 * User: Tebjan Halm
 * Date: 09.04.2012
 * Time: 10:24
 * 
 * 
 */
using System;
using System.Diagnostics;
using System.Globalization;
using System.Threading;

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
	
	/// <summary>
	/// Handles message construction and sending
	/// </summary>
	public class LibPDMessage : LibPDList
	{
		public LibPDMessage(string type, params object[] args)
			: base(args)
		{
			Type = type;
		}
		
		/// <summary>
		/// Type of the message
		/// </summary>
		public string Type
		{
			get;
			private set;
		}
		
		/// <summary>
		/// Send this message to the receiver in PD
		/// </summary>
		/// <param name="receiver">Identifier of the receiver</param>
		public override void SendTo(string receiver)
		{
			LibPD.SendMessage(receiver, Type, Args);
			TimesSent = TimesSent + 1;
		}
		
		public override string ToString()
		{
			return this.Type + " " + string.Format("{0} TimesSent={1}", String.Join(" ", Args), TimesSent);
		}
		
		/// <summary>
		/// Create a message from a message string
		/// </summary>
		/// <param name="message">message as string</param>
		/// <returns>New message</returns>
		public static LibPDMessage ParseMessage(string message)
		{
			if(string.IsNullOrEmpty(message)) return new LibPDMessage("bang", new object[0]);
			
			var type = message.Split(new char[]{' '}, StringSplitOptions.RemoveEmptyEntries)[0];
			var args = message.Replace(type, "").Split(new char[]{' '}, StringSplitOptions.RemoveEmptyEntries);
			
			return new LibPDMessage(type, args);
		}

	}
	
	//obj message
	public class LibPDObjMessage : LibPDMessage
	{
		public LibPDObjMessage(params object[] args)
			: base("obj", args)
		{
		}
	}
	
	//connect message
	public class LibPDConnectMessage : LibPDMessage
	{
		public LibPDConnectMessage(int fromObj, int fromPin, int toObj, int toPin)
			: base("connect", fromObj, fromPin, toObj, toPin)
		{
		}
	}
	
	//connect message
	public class LibPDDisconnectMessage : LibPDMessage
	{
		public LibPDDisconnectMessage(int fromObj, int fromPin, int toObj, int toPin)
			: base("disconnect", fromObj, fromPin, toObj, toPin)
		{
		}
	}
}
