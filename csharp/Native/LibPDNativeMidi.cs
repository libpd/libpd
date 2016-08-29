/*
 * 
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 * 
 * 
 * Created by SharpDevelop.
 * User: Tebjan Halm
 * Date: 11.04.2012
 * Time: 11:40
 * 
 */
 
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

namespace LibPDBinding
{
	public static partial class LibPD
	{
		#region Send Midi
		[DllImport(DllName, EntryPoint="libpd_noteon", CallingConvention = CallingConvention)]
		private static extern  int noteon(int channel, int pitch, int velocity) ;
		
		/// <summary>
		/// sends a note on event to pd
		/// </summary>
		/// <param name="channel">
		///            starting at 0 </param>
		/// <param name="pitch">
		///            0..0x7f </param>
		/// <param name="velocity">
		///            0..0x7f </param>
		/// <returns> error code, 0 on success </returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int SendNoteOn(int channel, int pitch, int velocity)
		{
			return noteon(channel, pitch, velocity);
		}


		[DllImport(DllName, EntryPoint="libpd_controlchange", CallingConvention = CallingConvention)]
		private static extern  int controlchange(int channel, int controller, int value) ;

		/// <summary>
		/// sends a control change event to pd
		/// </summary>
		/// <param name="channel">
		///            starting at 0 </param>
		/// <param name="controller">
		///            0..0x7f </param>
		/// <param name="value">
		///            0..0x7f </param>
		/// <returns> error code, 0 on success </returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int SendControlChange(int channel, int controller, int value)
		{
			return controlchange(channel, controller, value);
		}
		
		
		[DllImport(DllName, EntryPoint="libpd_programchange", CallingConvention = CallingConvention)]
		private static extern  int programchange(int channel, int value) ;

		/// <summary>
		/// sends a program change event to Pd
		/// </summary>
		/// <param name="channel">
		///            starting at 0 </param>
		/// <param name="value">
		///            0..0x7f </param>
		/// <returns> error code, 0 on success </returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int SendProgramChange(int channel, int value)
		{
			return programchange(channel, value);
		}
		
	
		[DllImport(DllName, EntryPoint="libpd_pitchbend", CallingConvention = CallingConvention)]
		private static extern  int pitchbend(int channel, int value) ;

		/// <summary>
		/// sends a pitch bend event to pd
		/// </summary>
		/// <param name="channel">
		///            starting at 0 </param>
		/// <param name="value">
		///            -8192..8191 (note that Pd has some offset bug in its pitch
		///            bend objects, but libpd corrects for this) </param>
		/// <returns> error code, 0 on success </returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int SendPitchbend(int channel, int value)
		{
			return pitchbend(channel, value);
		}
		
	
		[DllImport(DllName, EntryPoint="libpd_aftertouch", CallingConvention = CallingConvention)]
		private static extern  int aftertouch(int channel, int value) ;

		/// <summary>
		/// sends an aftertouch event to pd
		/// </summary>
		/// <param name="channel">
		///            starting at 0 </param>
		/// <param name="value">
		///            0..0x7f </param>
		/// <returns> error code, 0 on success </returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int SendAftertouch(int channel, int value)
		{
			return aftertouch(channel, value);
		}
		
	
		[DllImport(DllName, EntryPoint="libpd_polyaftertouch", CallingConvention = CallingConvention)]
		private static extern  int polyaftertouch(int channel, int pitch, int value) ;
		
		/// <summary>
		/// sends a polyphonic aftertouch event to pd
		/// </summary>
		/// <param name="channel">
		///            starting at 0 </param>
		/// <param name="pitch">
		///            0..0x7f </param>
		/// <param name="value">
		///            0..0x7f </param>
		/// <returns> error code, 0 on success </returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int SendPolyAftertouch(int channel, int pitch, int value)
		{
			return polyaftertouch(channel, pitch, value);
		}

		
		[DllImport(DllName, EntryPoint="libpd_midibyte", CallingConvention = CallingConvention)]
		private static extern  int midibyte(int port, int value) ;
		
		/// <summary>
		/// sends one raw MIDI byte to pd
		/// </summary>
		/// <param name="port">
		///            0..0x0fff </param>
		/// <param name="value">
		///            0..0xff </param>
		/// <returns> error code, 0 on success </returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int SendMidiByte(int port, int value)
		{
			return midibyte(port, value);
		}
		
		
		[DllImport(DllName, EntryPoint="libpd_sysex", CallingConvention = CallingConvention)]
		private static extern  int sysex(int port, int value) ;

		/// <summary>
		/// sends one byte of a sysex message to pd
		/// </summary>
		/// <param name="port">
		///            0..0x0fff </param>
		/// <param name="value">
		///            0..0x7f </param>
		/// <returns> error code, 0 on success </returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int SendSysex(int port, int value)
		{
			return sysex(port, value);
		}
		
		
		[DllImport(DllName, EntryPoint="libpd_sysrealtime", CallingConvention = CallingConvention)]
		private static extern  int sysrealtime(int port, int value) ;
		
		/// <summary>
		/// sends one byte to the realtimein object of pd
		/// </summary>
		/// <param name="port">
		///            0..0x0fff </param>
		/// <param name="value">
		///            0..0xff </param>
		/// <returns> error code, 0 on success </returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int SendSysRealtime(int port, int value)
		{
			return sysrealtime(port, value);
		}
		
		#endregion Send Midi
		
	}
}
