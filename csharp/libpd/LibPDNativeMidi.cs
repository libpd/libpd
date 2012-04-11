/*
 * Created by SharpDevelop.
 * User: Tebjan Halm
 * Date: 11.04.2012
 * Time: 11:40
 * 
 * 
 */
using System.Runtime.InteropServices;

namespace LibPDBinding
{

	public static partial class LibPD
	{
		private static void SetupMidi()
		{
			
		}
		
		/// Return Type: int
		///channel: int
		///pitch: int
		///velocity: int
		[DllImport("libpd.dll", EntryPoint="libpd_noteon")]
		public static extern  int noteon(int channel, int pitch, int velocity) ;

		
		/// Return Type: int
		///channel: int
		///controller: int
		///value: int
		[DllImport("libpd.dll", EntryPoint="libpd_controlchange")]
		public static extern  int controlchange(int channel, int controller, int value) ;

		
		/// Return Type: int
		///channel: int
		///value: int
		[DllImport("libpd.dll", EntryPoint="libpd_programchange")]
		public static extern  int programchange(int channel, int value) ;

		
		/// Return Type: int
		///channel: int
		///value: int
		[DllImport("libpd.dll", EntryPoint="libpd_pitchbend")]
		public static extern  int pitchbend(int channel, int value) ;

		
		/// Return Type: int
		///channel: int
		///value: int
		[DllImport("libpd.dll", EntryPoint="libpd_aftertouch")]
		public static extern  int aftertouch(int channel, int value) ;

		
		/// Return Type: int
		///channel: int
		///pitch: int
		///value: int
		[DllImport("libpd.dll", EntryPoint="libpd_polyaftertouch")]
		public static extern  int polyaftertouch(int channel, int pitch, int value) ;

		
		/// Return Type: int
		///port: int
		///param1: int
		///param2: byte
		[DllImport("libpd.dll", EntryPoint="libpd_midibyte")]
		public static extern  int midibyte(int port, int param1, byte param2) ;

		
		/// Return Type: int
		///port: int
		///param1: int
		///param2: byte
		[DllImport("libpd.dll", EntryPoint="libpd_sysex")]
		public static extern  int sysex(int port, int param1, byte param2) ;

		
		/// Return Type: int
		///port: int
		///param1: int
		///param2: byte
		[DllImport("libpd.dll", EntryPoint="libpd_sysrealtime")]
		public static extern  int sysrealtime(int port, int param1, byte param2) ;
	}
}
