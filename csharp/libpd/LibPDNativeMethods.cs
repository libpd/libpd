/*
 * Created by SharpDevelop.
 * User: Tebjan Halm
 * Date: 08.04.2012
 * Time: 20:18
 * 
 */

using System.Runtime.InteropServices;

namespace LibPDBinding
{
	/// Return Type: void
	///recv: charr
	[System.Runtime.InteropServices.UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
	public delegate void t_libpd_printhook([System.Runtime.InteropServices.In] [System.Runtime.InteropServices.MarshalAsAttribute(System.Runtime.InteropServices.UnmanagedType.LPStr)] string recv);

	public static class LibPD
	{
		
		/// Return Type: void
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_init")]
		public static extern  void init() ;

		
		/// Return Type: void
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_clear_search_path")]
		public static extern  void clear_search_path() ;

		
		/// Return Type: void
		///sym: char*
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_add_to_search_path")]
		public static extern  void add_to_search_path([System.Runtime.InteropServices.InAttribute()] [System.Runtime.InteropServices.MarshalAsAttribute(System.Runtime.InteropServices.UnmanagedType.LPStr)] string sym) ;

		
		/// Return Type: void*
		///basename: char*
		///dirname: char*
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_openfile")]
		public static extern  System.IntPtr openfile([System.Runtime.InteropServices.InAttribute()] [System.Runtime.InteropServices.MarshalAsAttribute(System.Runtime.InteropServices.UnmanagedType.LPStr)] string basename, [System.Runtime.InteropServices.InAttribute()] [System.Runtime.InteropServices.MarshalAsAttribute(System.Runtime.InteropServices.UnmanagedType.LPStr)] string dirname) ;

		
		/// Return Type: void
		///p: void*
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_closefile")]
		public static extern  void closefile(System.IntPtr p) ;

		
		/// Return Type: int
		///p: void*
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_getdollarzero")]
		public static extern  int getdollarzero(System.IntPtr p) ;

		
		/// Return Type: int
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_blocksize")]
		public static extern  int blocksize() ;

		
		/// Return Type: int
		///inChans: int
		///outChans: int
		///sampleRate: int
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_init_audio")]
		public static extern  int init_audio(int inChans, int outChans, int sampleRate) ;

		
		/// Return Type: int
		///inBuffer: float*
		///outBuffer: float*
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_process_raw")]
		public static extern  int process_raw(ref float inBuffer, ref float outBuffer) ;

		
		/// Return Type: int
		///ticks: int
		///inBuffer: short*
		///outBuffer: short*
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_process_short")]
		public static extern  int process_short(int ticks, ref short inBuffer, ref short outBuffer) ;

		
		/// Return Type: int
		///ticks: int
		///inBuffer: float*
		///outBuffer: float*
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_process_float")]
		public static extern  int process_float(int ticks, ref float inBuffer, ref float outBuffer) ;

		
		/// Return Type: int
		///ticks: int
		///inBuffer: double*
		///outBuffer: double*
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_process_double")]
		public static extern  int process_double(int ticks, ref double inBuffer, ref double outBuffer) ;

		
		/// Return Type: int
		///name: char*
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_arraysize")]
		public static extern  int arraysize([System.Runtime.InteropServices.InAttribute()] [System.Runtime.InteropServices.MarshalAsAttribute(System.Runtime.InteropServices.UnmanagedType.LPStr)] string name) ;

		
		/// Return Type: int
		///dest: float*
		///src: char*
		///offset: int
		///n: int
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_read_array")]
		public static extern  int read_array(ref float dest, [System.Runtime.InteropServices.InAttribute()] [System.Runtime.InteropServices.MarshalAsAttribute(System.Runtime.InteropServices.UnmanagedType.LPStr)] string src, int offset, int n) ;

		
		/// Return Type: int
		///dest: char*
		///offset: int
		///src: float*
		///n: int
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_write_array")]
		public static extern  int write_array([System.Runtime.InteropServices.InAttribute()] [System.Runtime.InteropServices.MarshalAsAttribute(System.Runtime.InteropServices.UnmanagedType.LPStr)] string dest, int offset, ref float src, int n) ;

		
		/// Return Type: int
		///recv: char*
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_bang")]
		public static extern  int send_bang([System.Runtime.InteropServices.InAttribute()] [System.Runtime.InteropServices.MarshalAsAttribute(System.Runtime.InteropServices.UnmanagedType.LPStr)] string recv) ;

		
		/// Return Type: int
		///recv: char*
		///x: float
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_float")]
		public static extern  int send_float([System.Runtime.InteropServices.InAttribute()] [System.Runtime.InteropServices.MarshalAsAttribute(System.Runtime.InteropServices.UnmanagedType.LPStr)] string recv, float x) ;

		
		/// Return Type: int
		///recv: char*
		///sym: char*
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_symbol")]
		public static extern  int send_symbol([System.Runtime.InteropServices.InAttribute()] [System.Runtime.InteropServices.MarshalAsAttribute(System.Runtime.InteropServices.UnmanagedType.LPStr)] string recv, [System.Runtime.InteropServices.InAttribute()] [System.Runtime.InteropServices.MarshalAsAttribute(System.Runtime.InteropServices.UnmanagedType.LPStr)] string sym) ;

		
		/// Return Type: int
		///max_length: int
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_start_message")]
		public static extern  int start_message(int max_length) ;

		
		/// Return Type: void
		///x: float
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_add_float")]
		public static extern  void add_float(float x) ;

		
		/// Return Type: void
		///sym: char*
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_add_symbol")]
		public static extern  void add_symbol([System.Runtime.InteropServices.InAttribute()] [System.Runtime.InteropServices.MarshalAsAttribute(System.Runtime.InteropServices.UnmanagedType.LPStr)] string sym) ;

		
		/// Return Type: int
		///recv: char*
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_finish_list")]
		public static extern  int finish_list([System.Runtime.InteropServices.InAttribute()] [System.Runtime.InteropServices.MarshalAsAttribute(System.Runtime.InteropServices.UnmanagedType.LPStr)] string recv) ;

		
		/// Return Type: int
		///recv: char*
		///msg: char*
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_finish_message")]
		public static extern  int finish_message([System.Runtime.InteropServices.InAttribute()] [System.Runtime.InteropServices.MarshalAsAttribute(System.Runtime.InteropServices.UnmanagedType.LPStr)] string recv, [System.Runtime.InteropServices.InAttribute()] [System.Runtime.InteropServices.MarshalAsAttribute(System.Runtime.InteropServices.UnmanagedType.LPStr)] string msg) ;

		
		/// Return Type: int
		///sym: char*
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_exists")]
		public static extern  int exists([System.Runtime.InteropServices.InAttribute()] [System.Runtime.InteropServices.MarshalAsAttribute(System.Runtime.InteropServices.UnmanagedType.LPStr)] string sym) ;

		
		/// Return Type: void*
		///sym: char*
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_bind")]
		public static extern  System.IntPtr bind([System.Runtime.InteropServices.InAttribute()] [System.Runtime.InteropServices.MarshalAsAttribute(System.Runtime.InteropServices.UnmanagedType.LPStr)] string sym) ;

		
		/// Return Type: void
		///p: void*
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_unbind")]
		public static extern  void unbind(System.IntPtr p) ;

		
		/// Return Type: int
		///channel: int
		///pitch: int
		///velocity: int
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_noteon")]
		public static extern  int noteon(int channel, int pitch, int velocity) ;

		
		/// Return Type: int
		///channel: int
		///controller: int
		///value: int
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_controlchange")]
		public static extern  int controlchange(int channel, int controller, int value) ;

		
		/// Return Type: int
		///channel: int
		///value: int
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_programchange")]
		public static extern  int programchange(int channel, int value) ;

		
		/// Return Type: int
		///channel: int
		///value: int
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_pitchbend")]
		public static extern  int pitchbend(int channel, int value) ;

		
		/// Return Type: int
		///channel: int
		///value: int
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_aftertouch")]
		public static extern  int aftertouch(int channel, int value) ;

		
		/// Return Type: int
		///channel: int
		///pitch: int
		///value: int
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_polyaftertouch")]
		public static extern  int polyaftertouch(int channel, int pitch, int value) ;

		
		/// Return Type: int
		///port: int
		///param1: int
		///param2: byte
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_midibyte")]
		public static extern  int midibyte(int port, int param1, byte param2) ;

		
		/// Return Type: int
		///port: int
		///param1: int
		///param2: byte
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_sysex")]
		public static extern  int sysex(int port, int param1, byte param2) ;

		
		/// Return Type: int
		///port: int
		///param1: int
		///param2: byte
		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_sysrealtime")]
		public static extern  int sysrealtime(int port, int param1, byte param2) ;
		
//		/// Return Type: void
//		///hook: t_libpd_printhook
//		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_set_printhook")]
//		public static extern  void set_printhook(t_libpd_printhook hook) ;
//		
//		/// Return Type: void
//		[System.Runtime.InteropServices.DllImportAttribute("libpd.dll", EntryPoint="libpd_test_printhook")]
//		public static extern  void test_printhook() ;


	}
}
