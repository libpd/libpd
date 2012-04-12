/*
 * Created by SharpDevelop.
 * User: Tebjan Halm
 * Date: 08.04.2012
 * Time: 20:18
 * 
 */

using System;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace LibPDBinding
{

	/// <summary>
	/// Static methods of libpd.dll
	/// </summary>
	public static partial class LibPD
	{
		//only call this once a lifetime
		static LibPD()
		{
			SetupHooks();
			SetupMidiHooks();
			libpd_init();
		}
		
		public static bool WriteMessageToDebug
		{
			[MethodImpl(MethodImplOptions.Synchronized)]
			get;
			[MethodImpl(MethodImplOptions.Synchronized)]
			set;
		}
				
		/// Return Type: void
		[DllImport("libpd.dll", EntryPoint="libpd_clear_search_path")]
		private static extern  void clear_search_path() ;
		
		/// <summary>
		/// clears the search path for pd externals
		/// </summary>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void ClearSearchPath()
		{
			clear_search_path();
		}

		
		/// Return Type: void
		///sym: char*
		[DllImport("libpd.dll", EntryPoint="libpd_add_to_search_path")]
		private static extern  void add_to_search_path([In] [MarshalAs(UnmanagedType.LPStr)] string sym) ;
		
		/// <summary>
		/// adds a directory to the search path
		/// </summary>
		/// <param name="sym">directory to add</param>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void AddToSearchPath(string sym)
		{
			add_to_search_path(sym);
		}
		
		
		/// <summary>
		/// same as "compute audio" checkbox in pd gui, or [;pd dsp 0/1(
		/// 
		/// Note: Maintaining a DSP state that's separate from the state of the audio
		/// rendering thread doesn't make much sense in libpd. In most applications,
		/// you probably just want to call {@code computeAudio(true)} at the
		/// beginning and then forget that this method exists.
		/// </summary>
		/// <param name="state"></param>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static void ComputeAudio(bool state)
		{
			SendMessage("pd", "dsp", state ? 1 : 0);
		}

		/// Return Type: void*
		///basename: char*
		///dirname: char*
		[DllImport("libpd.dll", EntryPoint="libpd_openfile")]
		public static extern  IntPtr openfile([In] [MarshalAs(UnmanagedType.LPStr)] string basename, [In] [MarshalAs(UnmanagedType.LPStr)] string dirname) ;

		/// Return Type: void
		///p: void*
		[DllImport("libpd.dll", EntryPoint="libpd_closefile")]
		public static extern  void closefile(IntPtr p) ;
		
		/// Return Type: int
		///p: void*
		[DllImport("libpd.dll", EntryPoint="libpd_getdollarzero")]
		public static extern  int getdollarzero(IntPtr p) ;
				
		/// Return Type: int
		///sym: char*
		[DllImport("libpd.dll", EntryPoint="libpd_exists")]
		public static extern  int exists([In] [MarshalAs(UnmanagedType.LPStr)] string sym) ;

		/// Return Type: void*
		///sym: char*
		[DllImport("libpd.dll", EntryPoint="libpd_bind")]
		public static extern  IntPtr bind([In] [MarshalAs(UnmanagedType.LPStr)] string sym) ;
		
		/// Return Type: void
		///p: void*
		[DllImport("libpd.dll", EntryPoint="libpd_unbind")]
		public static extern  void unbind(IntPtr p) ;
		
		/// <summary>
		/// sends a message to an object in pd
		/// </summary>
		/// <param name="receiver"> </param>
		/// <param name="message"> </param>
		/// <param name="args">
		///            list of arguments of type Integer, Float, or String; no more
		///            than 32 arguments </param>
		/// <returns> error code, 0 on success </returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int SendMessage(string receiver, string message, params object[] args)
		{
			if(WriteMessageToDebug)
			{
				var s = String.Format("Message: {0} {1}", receiver, message);
				int err = ProcessArgsDebug(args, ref s);
				var ret = (err == 0) ? finish_message(receiver, message) : err;
				s += " Start: " + err;
				s += " End: " + ret;
				Debug.WriteLine(s);
				return ret;
			}
			else
			{
				int err = ProcessArgs(args);
				return (err == 0) ? finish_message(receiver, message) : err;
			}
		}
		
		/// <summary>
		/// sends a list to an object in pd
		/// </summary>
		/// <param name="receiver"> </param>
		/// <param name="args">
		///            list of arguments of type Integer, Float, or String; no more
		///            than 32 arguments </param>
		/// <returns> error code, 0 on success </returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int SendList(string receiver, params object[] args)
		{
			if(WriteMessageToDebug)
			{
				var s = String.Format("List: {0}", receiver);
				int err = ProcessArgsDebug(args, ref s);
				var ret = (err == 0) ? finish_list(receiver) : err;
				s += " Start: " + err;
				s += " End: " + ret;
				Debug.WriteLine(s);
				return ret;
			}
			else
			{
				int err = ProcessArgs(args);
				return (err == 0) ? finish_list(receiver) : err;
			}
		}
		
		#region AUDIO
		
		/// Return Type: int
		[DllImport("libpd.dll", EntryPoint="libpd_blocksize")]
		public static extern  int blocksize() ;
		
		
		[DllImport("libpd.dll", EntryPoint="libpd_init_audio")]
		private static extern  int init_audio(int inputChannels, int outputChannels, int sampleRate) ;
		
		/// <summary>
		/// sets up pd audio; must be called before process callback
		/// </summary>
		/// <param name="inputChannels"> </param>
		/// <param name="outputChannels"> </param>
		/// <param name="sampleRate"> </param>
		/// <returns> error code, 0 on success </returns>
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int InitAudio(int inputChannels, int outputChannels, int sampleRate)
		{
			return init_audio(inputChannels, outputChannels, sampleRate);
		}

		/// Return Type: int
		///inBuffer: float*
		///outBuffer: float*
		[DllImport("libpd.dll", EntryPoint="libpd_process_raw")]
		public static extern  int process_raw(ref float inBuffer, ref float outBuffer) ;

		
		/// Return Type: int
		///ticks: int
		///inBuffer: short*
		///outBuffer: short*
		[DllImport("libpd.dll", EntryPoint="libpd_process_short")]
		public static extern  int process_short(int ticks, ref short inBuffer, ref short outBuffer) ;

		
		/// Return Type: int
		///ticks: int
		///inBuffer: float*
		///outBuffer: float*
		[DllImport("libpd.dll", EntryPoint="libpd_process_float")]
		private static extern  int process_float(int ticks, ref float inBuffer, ref float outBuffer) ;

		[MethodImpl(MethodImplOptions.Synchronized)]
		public static int Process(int ticks, ref float inBuffer, ref float outBuffer)
		{
			return process_float(ticks, ref inBuffer, ref outBuffer);
		}
		
		/// Return Type: int
		///ticks: int
		///inBuffer: double*
		///outBuffer: double*
		[DllImport("libpd.dll", EntryPoint="libpd_process_double")]
		public static extern  int process_double(int ticks, ref double inBuffer, ref double outBuffer) ;
		
		#endregion AUDIO

		
		/// Return Type: int
		///name: char*
		[DllImport("libpd.dll", EntryPoint="libpd_arraysize")]
		public static extern  int arraysize([In] [MarshalAs(UnmanagedType.LPStr)] string name) ;

		
		/// Return Type: int
		///dest: float*
		///src: char*
		///offset: int
		///n: int
		[DllImport("libpd.dll", EntryPoint="libpd_read_array")]
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static extern  int read_array(ref float dest, [In] [MarshalAs(UnmanagedType.LPStr)] string src, int offset, int n) ;

		
		/// Return Type: int
		///dest: char*
		///offset: int
		///src: float*
		///n: int
		[DllImport("libpd.dll", EntryPoint="libpd_write_array")]
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static extern  int write_array([In] [MarshalAs(UnmanagedType.LPStr)] string dest, int offset, ref float src, int n) ;

		
		/// Return Type: int
		///recv: char*
		[DllImport("libpd.dll", EntryPoint="libpd_bang")]
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static extern  int send_bang([In] [MarshalAs(UnmanagedType.LPStr)] string recv) ;

		
		/// Return Type: int
		///recv: char*
		///x: float
		[DllImport("libpd.dll", EntryPoint="libpd_float")]
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static extern  int send_float([In] [MarshalAs(UnmanagedType.LPStr)] string recv, float x) ;

		
		/// Return Type: int
		///recv: char*
		///sym: char*
		[DllImport("libpd.dll", EntryPoint="libpd_symbol")]
		[MethodImpl(MethodImplOptions.Synchronized)]
		public static extern  int send_symbol([In] [MarshalAs(UnmanagedType.LPStr)] string recv, [In] [MarshalAs(UnmanagedType.LPStr)] string sym) ;

		#region PRIVATE STUFF
		
		/// Init PD
		[DllImport("libpd.dll", EntryPoint="libpd_init")]
		private static extern  void libpd_init() ;
		
		/// Return Type: int
		///max_length: int
		[DllImport("libpd.dll", EntryPoint="libpd_start_message")]
		private static extern  int start_message(int max_length) ;
		
		/// Return Type: void
		///x: float
		[DllImport("libpd.dll", EntryPoint="libpd_add_float")]
		private static extern  void add_float(float x) ;

		
		/// Return Type: void
		///sym: char*
		[DllImport("libpd.dll", EntryPoint="libpd_add_symbol")]
		private static extern  void add_symbol([In] [MarshalAs(UnmanagedType.LPStr)] string sym) ;

		
		/// Return Type: int
		///recv: char*
		[DllImport("libpd.dll", EntryPoint="libpd_finish_list")]
		private static extern  int finish_list([In] [MarshalAs(UnmanagedType.LPStr)] string recv) ;

		
		/// Return Type: int
		///recv: char*
		///msg: char*
		[DllImport("libpd.dll", EntryPoint="libpd_finish_message")]
		private static extern  int finish_message([In] [MarshalAs(UnmanagedType.LPStr)] string recv, [In] [MarshalAs(UnmanagedType.LPStr)] string msg) ;

		
		//parse args helper
		private static int ProcessArgs(object[] args)
		{
			if (start_message(args.Length) != 0)
			{
				return -100;
			}
			foreach (object arg in args)
			{
				if (arg is int?)
				{
					add_float((int)((int?) arg));
				}
				else if (arg is float?)
				{
					add_float((float)((float?) arg));
				}
				else if (arg is double?)
				{
					add_float((float)((double?) arg));
				}
				else if (arg is string)
				{
					add_symbol((string) arg);
				}
				else
				{
					return -101; // illegal argument
				}
			}
			return 0;
		}
		
		//parse args helper with debug string
		private static int ProcessArgsDebug(object[] args, ref string debug)
		{
			if (start_message(args.Length) != 0)
			{
				return -100;
			}
			foreach (object arg in args)
			{
				if (arg is int?)
				{
					add_float((int)((int?) arg));
					debug += " i:" + arg.ToString();
				}
				else if (arg is float?)
				{
					add_float((float)((float?) arg));
					debug += " f:" + arg.ToString();
				}
				else if (arg is double?)
				{
					add_float((float)((double?) arg));
					debug += " d:" + arg.ToString();
				}
				else if (arg is string)
				{
					add_symbol((string) arg);
					debug += " s:" + arg.ToString();
				}
				else
				{
					debug += " illegal argument: " + arg.ToString();
					return -101; // illegal argument
				}
			}
			return 0;
		}
		
		#endregion PRIVATE STUFF

	}
}
