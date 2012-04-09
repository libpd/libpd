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
	/// Handles message construction and sending
	/// </summary>
	public class LibPDMessage
	{
		public LibPDMessage(params string[] args)
		{
			Args = args;
		}
		
		/// <summary>
		/// Arguments of this message
		/// </summary>
		public string[] Args
		{
			get;
			private set;
		}
		
		/// <summary>
		/// How often the messages was sent
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
			var start = LibPD.start_message(Args.Length + 1);
			ParseAndAddArgs();
			var end = LibPD.finish_list(receiver);
			
			TimesSent = TimesSent + 1;
			
			Debug.WriteLine("Message: {0} {1} Start: {2} End: {2}", receiver, this.ToString(), start, end);
		}
		
		//add arguments to native pd message
		protected void ParseAndAddArgs()
		{
			var previousCulture = Thread.CurrentThread.CurrentCulture;
            Thread.CurrentThread.CurrentCulture = CultureInfo.InvariantCulture;
            
			foreach(var s in Args)
			{
				float f;
				if(float.TryParse(s, out f))
				{
					LibPD.add_float(f);
				}
				else
				{
					LibPD.add_symbol(s);
				}
			}
			
			Thread.CurrentThread.CurrentCulture = previousCulture;
		}
		
		public override string ToString()
		{
			return string.Format("{0} TimesSent={1}", String.Join(" ", Args), TimesSent);
		}

	}
	
	/// <summary>
	/// Typed message
	/// </summary>
	public class LibPDTypedMessage : LibPDMessage
	{
		public LibPDTypedMessage(string type, params string[] args)
			: base(args)
		{
			Type = type;
		}
		
		public string Type
		{
			get;
			private set;
		}
		
		public override void SendTo(string receiver)
		{
			var start = LibPD.start_message(Args.Length + 2);
			ParseAndAddArgs();
			var end = LibPD.finish_message(receiver, Type);
			
			TimesSent = TimesSent + 1;
			
			Debug.WriteLine("Message: {0} {1} Start: {2} End: {2}", receiver, this.ToString(), start, end);
		}
		
		public override string ToString()
		{
			return this.Type + " " + base.ToString();
		}

	}
	
	//obj message
	public class LibPDObjMessage : LibPDTypedMessage
	{
		public LibPDObjMessage(params string[] args)
			: base("obj", args)
		{
		}
	}
	
	//connect message
	public class LibPDConnectMessage : LibPDTypedMessage
	{
		public LibPDConnectMessage(int fromObj, int fromPin, int toObj, int toPin)
			: base("connect", fromObj.ToString(), fromPin.ToString(), toObj.ToString(), toPin.ToString())
		{
		}
	}
}
