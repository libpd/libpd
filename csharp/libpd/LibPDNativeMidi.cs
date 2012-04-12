/*
 * Created by SharpDevelop.
 * User: Tebjan Halm
 * Date: 11.04.2012
 * Time: 11:40
 * 
 * 
 */
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

namespace LibPDBinding
{
	#region Delegates
	
	/// Return Type: void
	///channel: int
	///pitch: int
	///velocity: int
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate void LibPDNoteOnHook(int channel, int pitch, int velocity);

	/// Return Type: void
	///channel: int
	///controller: int
	///value: int
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate void LibPDControlChangeHook(int channel, int controller, int value);

	/// Return Type: void
	///channel: int
	///value: int
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate void LibPDProgramChangeHook(int channel, int value);

	/// Return Type: void
	///channel: int
	///value: int
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate void LibPDPitchBendHook(int channel, int value);

	/// Return Type: void
	///channel: int
	///value: int
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate void LibPDAftertouchHook(int channel, int value);

	/// Return Type: void
	///channel: int
	///pitch: int
	///value: int
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate void LibPDPolyAftertouchHook(int channel, int pitch, int value);

	/// Return Type: void
	///port: int
	///param1: int
	///param2: byte
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate void LibPDMidiByteHook(int port, int byt);

	#endregion Delegates

	public static partial class LibPD
	{
		private static LibPDNoteOnHook noteonHook;
		private static LibPDControlChangeHook controlchangeHook;
		private static LibPDProgramChangeHook programchangeHook;
		private static LibPDPitchBendHook pitchbendHook;
		private static LibPDAftertouchHook aftertouchHook;
		private static LibPDPolyAftertouchHook polyaftertouchHook;
		private static LibPDMidiByteHook midibyteHook;
		
		private static void SetupMidiHooks()
		{
			noteonHook = new LibPDNoteOnHook(RaiseNoteOnEvent);
			set_noteonhook(noteonHook);
			
			controlchangeHook = new LibPDControlChangeHook(RaiseControlChangeEvent);
			set_controlchangehook(controlchangeHook);
			
			programchangeHook = new LibPDProgramChangeHook(RaiseProgramChangeEvent);
			set_programchangehook(programchangeHook);
			
			pitchbendHook = new LibPDPitchBendHook(RaisePitchbendEvent);
			set_pitchbendhook(pitchbendHook);
			
			aftertouchHook = new LibPDAftertouchHook(RaiseAftertouchEvent);
			set_aftertouchhook(aftertouchHook);
			
			polyaftertouchHook = new LibPDPolyAftertouchHook(RaisePolyAftertouchEvent);
			set_polyaftertouchhook(polyaftertouchHook);
			
			midibyteHook = new LibPDMidiByteHook(RaiseMidiByteEvent);
			set_midibytehook(midibyteHook);
			
		}
		
		public static event LibPDNoteOnHook NoteOn;
		public static event LibPDControlChangeHook ControlChange;
		public static event LibPDProgramChangeHook ProgramChange;
		public static event LibPDPitchBendHook Pitchbend;
		public static event LibPDAftertouchHook Aftertouch;
		public static event LibPDPolyAftertouchHook PolyAftertouch;
		public static event LibPDMidiByteHook MidiByte;
		
		#region Rise events
		
		private static void RaiseNoteOnEvent(int channel, int pitch, int velocity)
		{
			// Event will be null if there are no subscribers
			if (NoteOn != null)
			{
				// Use the () operator to raise the event.
				NoteOn(channel, pitch, velocity);
			}
		}
		
		private static void RaiseControlChangeEvent(int channel, int controller, int val)
		{
			// Event will be null if there are no subscribers
			if (ControlChange != null)
			{
				// Use the () operator to raise the event.
				ControlChange(channel, controller, val);
			}
		}
		private static void RaiseProgramChangeEvent(int channel, int val)
		{
			// Event will be null if there are no subscribers
			if (ProgramChange != null)
			{
				// Use the () operator to raise the event.
				ProgramChange(channel, val);
			}
		}
		private static void RaisePitchbendEvent(int channel, int val)
		{
			// Event will be null if there are no subscribers
			if (Pitchbend != null)
			{
				// Use the () operator to raise the event.
				Pitchbend(channel, val);
			}
		}
		private static void RaiseAftertouchEvent(int channel, int val)
		{
			// Event will be null if there are no subscribers
			if (Aftertouch != null)
			{
				// Use the () operator to raise the event.
				Aftertouch(channel, val);
			}
		}
		private static void RaisePolyAftertouchEvent(int channel, int pitch, int val)
		{
			// Event will be null if there are no subscribers
			if (PolyAftertouch != null)
			{
				// Use the () operator to raise the event.
				PolyAftertouch(channel, pitch, val);
			}
		}
		private static void RaiseMidiByteEvent(int port, int byt)
		{
			// Event will be null if there are no subscribers
			if (MidiByte != null)
			{
				// Use the () operator to raise the event.
				MidiByte(port, byt);
			}
		}
		
		#endregion Rise events
		
		#region Send Midi
		

		[DllImport("libpd.dll", EntryPoint="libpd_noteon")]
		private static extern  int noteon(int channel, int pitch, int velocity) ;
		
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int SendNoteOn(int channel, int pitch, int velocity)
		{
			return noteon(channel, pitch, velocity);
		}


		[DllImport("libpd.dll", EntryPoint="libpd_controlchange")]
		private static extern  int controlchange(int channel, int controller, int value) ;

		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int SendControlChange(int channel, int controller, int value)
		{
			return controlchange(channel, controller, value);
		}
		
		
		[DllImport("libpd.dll", EntryPoint="libpd_programchange")]
		private static extern  int programchange(int channel, int value) ;

		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int SendProgramChange(int channel, int value)
		{
			return programchange(channel, value);
		}
		
	
		[DllImport("libpd.dll", EntryPoint="libpd_pitchbend")]
		private static extern  int pitchbend(int channel, int value) ;

		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int SendPitchbend(int channel, int value)
		{
			return pitchbend(channel, value);
		}
		
	
		[DllImport("libpd.dll", EntryPoint="libpd_aftertouch")]
		private static extern  int aftertouch(int channel, int value) ;

		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int SendAftertouch(int channel, int value)
		{
			return aftertouch(channel, value);
		}
		
	
		[DllImport("libpd.dll", EntryPoint="libpd_polyaftertouch")]
		private static extern  int polyaftertouch(int channel, int pitch, int value) ;
		
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int SendPolyAftertouch(int channel, int pitch, int value)
		{
			return polyaftertouch(channel, pitch, value);
		}

		
		[DllImport("libpd.dll", EntryPoint="libpd_midibyte")]
		private static extern  int midibyte(int port, int param1, int param2) ;
		
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int SendMidiByte(int port, int param1, int param2)
		{
			return midibyte(port, param1, param2);
		}
		
		
		[DllImport("libpd.dll", EntryPoint="libpd_sysex")]
		private static extern  int sysex(int port, int param1, int param2) ;

		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int SendSysex(int port, int param1, int param2)
		{
			return sysex(port, param1, param2);
		}
		
		
		[DllImport("libpd.dll", EntryPoint="libpd_sysrealtime")]
		private static extern  int sysrealtime(int port, int param1, int param2) ;
		
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int SendSysRealtime(int port, int param1, int param2)
		{
			return sysrealtime(port, param1, param2);
		}
		
		#endregion Send Midi
		
		#region Hook setter
		
		/// Return Type: void
		///hook: t_libpd_noteonhook
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_set_noteonhook")]
		private static extern  void set_noteonhook(LibPDNoteOnHook hook) ;

		
		/// Return Type: void
		///hook: t_libpd_controlchangehook
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_set_controlchangehook")]
		private static extern  void set_controlchangehook(LibPDControlChangeHook hook) ;

		
		/// Return Type: void
		///hook: t_libpd_programchangehook
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_set_programchangehook")]
		private static extern  void set_programchangehook(LibPDProgramChangeHook hook) ;

		
		/// Return Type: void
		///hook: t_libpd_pitchbendhook
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_set_pitchbendhook")]
		private static extern  void set_pitchbendhook(LibPDPitchBendHook hook) ;

		
		/// Return Type: void
		///hook: t_libpd_aftertouchhook
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_set_aftertouchhook")]
		private static extern  void set_aftertouchhook(LibPDAftertouchHook hook) ;

		
		/// Return Type: void
		///hook: t_libpd_polyaftertouchhook
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_set_polyaftertouchhook")]
		private static extern  void set_polyaftertouchhook(LibPDPolyAftertouchHook hook) ;

		
		/// Return Type: void
		///hook: t_libpd_midibytehook
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_set_midibytehook")]
		private static extern  void set_midibytehook(LibPDMidiByteHook hook) ;
		
		#endregion Hook setter

	}
}
