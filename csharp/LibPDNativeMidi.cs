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
using LibPDBinding.Native;
using System;

namespace LibPDBinding
{
	public static partial class LibPD
	{
		#region Send Midi

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
		[Obsolete("Use LibPDBinding.Managed.Midi.SendNoteOn()")]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static int SendNoteOn (int channel, int pitch, int velocity)
		{
			return Midi.noteon (channel, pitch, velocity);
		}


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
		[Obsolete("Use LibPDBinding.Managed.Midi.SendControlChange()")]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static int SendControlChange (int channel, int controller, int value)
		{
			return Midi.controlchange (channel, controller, value);
		}


		/// <summary>
		/// sends a program change event to Pd
		/// </summary>
		/// <param name="channel">
		///            starting at 0 </param>
		/// <param name="value">
		///            0..0x7f </param>
		/// <returns> error code, 0 on success </returns>
		[Obsolete("Use LibPDBinding.Managed.Midi.SendProgramChange()")]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static int SendProgramChange (int channel, int value)
		{
			return Midi.programchange (channel, value);
		}


		/// <summary>
		/// sends a pitch bend event to pd
		/// </summary>
		/// <param name="channel">
		///            starting at 0 </param>
		/// <param name="value">
		///            -8192..8191 (note that Pd has some offset bug in its pitch
		///            bend objects, but libpd corrects for this) </param>
		/// <returns> error code, 0 on success </returns>
		[Obsolete("Use LibPDBinding.Managed.Midi.SendPitchbend()")]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static int SendPitchbend (int channel, int value)
		{
			return Midi.pitchbend (channel, value);
		}


		/// <summary>
		/// sends an aftertouch event to pd
		/// </summary>
		/// <param name="channel">
		///            starting at 0 </param>
		/// <param name="value">
		///            0..0x7f </param>
		/// <returns> error code, 0 on success </returns>
		[Obsolete("Use LibPDBinding.Managed.Midi.SendAftertouch()")]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static int SendAftertouch (int channel, int value)
		{
			return Midi.aftertouch (channel, value);
		}


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
		[Obsolete("Use LibPDBinding.Managed.Midi.SendPolyAftertouch()")]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static int SendPolyAftertouch (int channel, int pitch, int value)
		{
			return Midi.polyaftertouch (channel, pitch, value);
		}


		/// <summary>
		/// sends one raw MIDI byte to pd
		/// </summary>
		/// <param name="port">
		///            0..0x0fff </param>
		/// <param name="value">
		///            0..0xff </param>
		/// <returns> error code, 0 on success </returns>
		[Obsolete("Use LibPDBinding.Managed.Midi.SendMidiByte()")]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static int SendMidiByte (int port, int value)
		{
			return Midi.midibyte (port, value);
		}


		/// <summary>
		/// sends one byte of a sysex message to pd
		/// </summary>
		/// <param name="port">
		///            0..0x0fff </param>
		/// <param name="value">
		///            0..0x7f </param>
		/// <returns> error code, 0 on success </returns>
		[Obsolete("Use LibPDBinding.Managed.Midi.SendSysex()")]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static int SendSysex (int port, int value)
		{
			return Midi.sysex (port, value);
		}


		/// <summary>
		/// sends one byte to the realtimein object of pd
		/// </summary>
		/// <param name="port">
		///            0..0x0fff </param>
		/// <param name="value">
		///            0..0xff </param>
		/// <returns> error code, 0 on success </returns>
		[Obsolete("Use LibPDBinding.Managed.Midi.SendSysRealtime()")]
		[MethodImpl (MethodImplOptions.Synchronized)]
		public static int SendSysRealtime (int port, int value)
		{
			return Midi.sysrealtime (port, value);
		}

		#endregion Send Midi
		
	}
}
