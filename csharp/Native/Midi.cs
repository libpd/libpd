using System;
using System.Runtime.InteropServices;

namespace LibPDBinding.Native
{
	static class Midi
	{
		[DllImport (Defines.DllName, EntryPoint = "libpd_noteon", CallingConvention = Defines.CallingConvention)]
		public static extern int noteon (int channel, int pitch, int velocity);

		[DllImport (Defines.DllName, EntryPoint = "libpd_controlchange", CallingConvention = Defines.CallingConvention)]
		public static extern int controlchange (int channel, int controller, int value);

		[DllImport (Defines.DllName, EntryPoint = "libpd_programchange", CallingConvention = Defines.CallingConvention)]
		public static extern int programchange (int channel, int value);

		[DllImport (Defines.DllName, EntryPoint = "libpd_pitchbend", CallingConvention = Defines.CallingConvention)]
		public static extern int pitchbend (int channel, int value);

		[DllImport (Defines.DllName, EntryPoint = "libpd_aftertouch", CallingConvention = Defines.CallingConvention)]
		public static extern int aftertouch (int channel, int value);

		[DllImport (Defines.DllName, EntryPoint = "libpd_polyaftertouch", CallingConvention = Defines.CallingConvention)]
		public static extern int polyaftertouch (int channel, int pitch, int value);

		[DllImport (Defines.DllName, EntryPoint = "libpd_midibyte", CallingConvention = Defines.CallingConvention)]
		public static extern int midibyte (int port, int value);

		[DllImport (Defines.DllName, EntryPoint = "libpd_sysex", CallingConvention = Defines.CallingConvention)]
		public static extern int sysex (int port, int value);

		[DllImport (Defines.DllName, EntryPoint = "libpd_sysrealtime", CallingConvention = Defines.CallingConvention)]
		public static extern int sysrealtime (int port, int value);

		[DllImport (Defines.DllName, EntryPoint = "libpd_set_noteonhook", CallingConvention = Defines.CallingConvention)]
		public static extern void set_noteonhook (LibPDNoteOnHook hook);

		[DllImport (Defines.DllName, EntryPoint = "libpd_set_controlchangehook", CallingConvention = Defines.CallingConvention)]
		public static extern void set_controlchangehook (LibPDControlChangeHook hook);

		[DllImport (Defines.DllName, EntryPoint = "libpd_set_programchangehook", CallingConvention = Defines.CallingConvention)]
		public static extern void set_programchangehook (LibPDProgramChangeHook hook);

		[DllImport (Defines.DllName, EntryPoint = "libpd_set_pitchbendhook", CallingConvention = Defines.CallingConvention)]
		public static extern void set_pitchbendhook (LibPDPitchbendHook hook);

		[DllImport (Defines.DllName, EntryPoint = "libpd_set_aftertouchhook", CallingConvention = Defines.CallingConvention)]
		public static extern void set_aftertouchhook (LibPDAftertouchHook hook);

		[DllImport (Defines.DllName, EntryPoint = "libpd_set_polyaftertouchhook", CallingConvention = Defines.CallingConvention)]
		public static extern void set_polyaftertouchhook (LibPDPolyAftertouchHook hook);

		[DllImport (Defines.DllName, EntryPoint = "libpd_set_midibytehook", CallingConvention = Defines.CallingConvention)]
		public static extern void set_midibytehook (LibPDMidiByteHook hook);
	}	
}