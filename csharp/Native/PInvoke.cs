using System;
using System.Runtime.InteropServices;

namespace LibPDBinding.Native
{
    public static class PInvoke
    {
        private const string DllName = "libpdcsharp";
        private const CallingConvention CallingConvention = System.Runtime.InteropServices.CallingConvention.Cdecl;

        [DllImport(DllName, EntryPoint="libpd_noteon", CallingConvention = CallingConvention)]
        internal static extern  int noteon(int channel, int pitch, int velocity) ;

        [DllImport(DllName, EntryPoint="libpd_controlchange", CallingConvention = CallingConvention)]
        internal static extern  int controlchange(int channel, int controller, int value) ;

        [DllImport(DllName, EntryPoint="libpd_programchange", CallingConvention = CallingConvention)]
        internal static extern  int programchange(int channel, int value) ;

        [DllImport(DllName, EntryPoint="libpd_pitchbend", CallingConvention = CallingConvention)]
        internal static extern  int pitchbend(int channel, int value) ;

        [DllImport(DllName, EntryPoint="libpd_aftertouch", CallingConvention = CallingConvention)]
        internal static extern  int aftertouch(int channel, int value) ;

        [DllImport(DllName, EntryPoint="libpd_polyaftertouch", CallingConvention = CallingConvention)]
        internal static extern  int polyaftertouch(int channel, int pitch, int value) ;

        [DllImport(DllName, EntryPoint="libpd_midibyte", CallingConvention = CallingConvention)]
        internal static extern  int midibyte(int port, int value) ;

        [DllImport(DllName, EntryPoint="libpd_sysex", CallingConvention = CallingConvention)]
        internal static extern  int sysex(int port, int value) ;

        [DllImport(DllName, EntryPoint="libpd_sysrealtime", CallingConvention = CallingConvention)]
        internal static extern  int sysrealtime(int port, int value) ;

        /// Init PD
        [DllImport(DllName, EntryPoint="libpd_init", CallingConvention = CallingConvention)]
        internal static extern void libpd_init() ;

        /// Return Type: void
        [DllImport(DllName, EntryPoint="libpd_clear_search_path", CallingConvention = CallingConvention)]
        internal static extern  void clear_search_path() ;

        /// Return Type: void
        ///sym: char*
        [DllImport(DllName, EntryPoint="libpd_add_to_search_path", CallingConvention = CallingConvention)]
        internal static extern  void add_to_search_path([In] [MarshalAs(UnmanagedType.LPStr)] string sym) ;

        /// Return Type: void*
        ///basename: char*
        ///dirname: char*
        [DllImport(DllName, EntryPoint="libpd_openfile", CallingConvention = CallingConvention)]
        internal static extern  IntPtr openfile([In] [MarshalAs(UnmanagedType.LPStr)] string basename, [In] [MarshalAs(UnmanagedType.LPStr)] string dirname) ;

        /// Return Type: void
        ///p: void*
        [DllImport(DllName, EntryPoint="libpd_closefile", CallingConvention = CallingConvention)]
        internal static extern  void closefile(IntPtr p) ;

        /// Return Type: int
        ///p: void*
        [DllImport(DllName, EntryPoint="libpd_getdollarzero", CallingConvention = CallingConvention)]
        internal static extern  int getdollarzero(IntPtr p) ;

        /// Return Type: int
        ///sym: char*
        [DllImport(DllName, EntryPoint="libpd_exists", CallingConvention = CallingConvention)]
        internal static extern  int exists([In] [MarshalAs(UnmanagedType.LPStr)] string sym) ;

        /// Return Type: int
        [DllImport(DllName, EntryPoint="libpd_blocksize", CallingConvention = CallingConvention)]
        internal static extern  int blocksize() ;

        [DllImport(DllName, EntryPoint="libpd_init_audio", CallingConvention = CallingConvention)]
        internal static extern  int init_audio(int inputChannels, int outputChannels, int sampleRate) ;

        /// Return Type: int
        ///inBuffer: float*
        ///outBuffer: float*
        [DllImport(DllName, EntryPoint="libpd_process_raw", CallingConvention = CallingConvention)]
        internal static extern  int process_raw([In] float[] inBuffer, [Out] float[] outBuffer) ;

        [DllImport(DllName, EntryPoint="libpd_process_raw", CallingConvention = CallingConvention)]
        internal static extern unsafe  int process_raw(float* inBuffer, float* outBuffer) ;

        /// Return Type: int
        ///ticks: int
        ///inBuffer: short*
        ///outBuffer: short*
        [DllImport(DllName, EntryPoint="libpd_process_short", CallingConvention = CallingConvention)]
        internal static extern  int process_short(int ticks, [In] short[] inBuffer, [Out] short[] outBuffer) ;

        [DllImport(DllName, EntryPoint="libpd_process_short", CallingConvention = CallingConvention)]
        internal static extern unsafe  int process_short(int ticks, short* inBuffer, short* outBuffer) ;

        /// Return Type: int
        ///ticks: int
        ///inBuffer: float*
        ///outBuffer: float*
        [DllImport(DllName, EntryPoint="libpd_process_float", CallingConvention = CallingConvention)]
        internal static extern  int process_float(int ticks, [In] float[] inBuffer, [Out] float[] outBuffer) ;

        [DllImport(DllName, EntryPoint="libpd_process_float", CallingConvention = CallingConvention)]
        internal static extern unsafe  int process_float(int ticks, float* inBuffer, float* outBuffer) ;

        /// Return Type: int
        ///ticks: int
        ///inBuffer: double*
        ///outBuffer: double*
        [DllImport(DllName, EntryPoint="libpd_process_double", CallingConvention = CallingConvention)]
        internal static extern  int process_double(int ticks, [In] double[] inBuffer, [Out] double[] outBuffer) ;

        [DllImport(DllName, EntryPoint="libpd_process_double", CallingConvention = CallingConvention)]
        internal static extern unsafe int process_double(int ticks, double* inBuffer, double* outBuffer) ;

        /// Return Type: int
        ///name: char*
        [DllImport(DllName, EntryPoint="libpd_arraysize", CallingConvention = CallingConvention)]
        internal static extern  int arraysize([In] [MarshalAs(UnmanagedType.LPStr)] string name) ;

        [DllImport(DllName, EntryPoint="libpd_read_array", CallingConvention = CallingConvention)]
        internal static extern  int read_array([Out] float[] dest, [In] [MarshalAs(UnmanagedType.LPStr)] string src, int offset, int n) ;

        [DllImport(DllName, EntryPoint="libpd_read_array", CallingConvention = CallingConvention)]
        internal static extern unsafe  int read_array(float* dest, [In] [MarshalAs(UnmanagedType.LPStr)] string src, int offset, int n) ;

        [DllImport(DllName, EntryPoint="libpd_write_array", CallingConvention = CallingConvention)]
        internal static extern  int write_array([In] [MarshalAs(UnmanagedType.LPStr)] string dest, int offset, [In] float[] src, int n) ;

        [DllImport(DllName, EntryPoint="libpd_write_array", CallingConvention = CallingConvention)]
        internal static extern unsafe int write_array([In] [MarshalAs(UnmanagedType.LPStr)] string dest, int offset, float* src, int n) ;

        [DllImport(DllName, EntryPoint="libpd_set_printhook", CallingConvention = CallingConvention)]
        internal static extern void set_printhook(LibPDPrintHook hook);

        [DllImport(DllName, EntryPoint="libpd_set_banghook", CallingConvention = CallingConvention)]
        internal static extern  void set_banghook(LibPDBangHook hook) ;

        [DllImport(DllName, EntryPoint="libpd_set_floathook", CallingConvention = CallingConvention)]
        internal static extern  void set_floathook(LibPDFloatHook hook) ;

        [DllImport(DllName, EntryPoint="libpd_set_symbolhook", CallingConvention = CallingConvention)]
        internal static extern  void set_symbolhook(LibPDSymbolHook hook) ;

        [DllImport(DllName, EntryPoint="libpd_set_listhook", CallingConvention = CallingConvention)]
        internal static extern  void set_listhook(LibPDListHook hook) ;

        [DllImport(DllName, EntryPoint="libpd_set_messagehook", CallingConvention = CallingConvention)]
        internal static extern  void set_messagehook(LibPDMessageHook hook) ;

        [DllImport(DllName, EntryPoint="libpd_is_float", CallingConvention = CallingConvention)]
        internal static extern int atom_is_float(IntPtr a) ;

        [DllImport(DllName, EntryPoint="libpd_is_symbol", CallingConvention = CallingConvention)]
        internal static extern int atom_is_symbol(IntPtr a) ;

        [DllImport(DllName, EntryPoint="libpd_get_float", CallingConvention = CallingConvention)]
        internal static extern float atom_get_float(IntPtr a) ;

        [DllImport(DllName, EntryPoint="libpd_get_symbol", CallingConvention = CallingConvention)]
        internal static extern IntPtr atom_get_symbol(IntPtr a);

        [DllImport(DllName, EntryPoint="libpd_next_atom", CallingConvention = CallingConvention)]
        internal static extern IntPtr next_atom(IntPtr a);

        [DllImport(DllName, EntryPoint="libpd_bind", CallingConvention = CallingConvention)]
        internal static extern IntPtr bind([In] [MarshalAs(UnmanagedType.LPStr)] string sym) ;

        [DllImport(DllName, EntryPoint="libpd_unbind", CallingConvention = CallingConvention)]
        internal static extern void unbind(IntPtr p) ;

        [DllImport(DllName, EntryPoint="libpd_bang", CallingConvention = CallingConvention)]
        internal static extern  int send_bang([In] [MarshalAs(UnmanagedType.LPStr)] string recv) ;

        [DllImport(DllName, EntryPoint="libpd_float", CallingConvention = CallingConvention)]
        internal static extern  int send_float([In] [MarshalAs(UnmanagedType.LPStr)] string recv, float x);

        [DllImport(PInvoke.DllName, EntryPoint = "libpd_symbol", CallingConvention = PInvoke.CallingConvention)]
        internal static extern int send_symbol([In] [MarshalAs(UnmanagedType.LPStr)] string recv, [In] [MarshalAs(UnmanagedType.LPStr)] string sym);

        [DllImport(DllName, EntryPoint="libpd_start_message", CallingConvention = CallingConvention)]
        internal static extern  int start_message(int max_length) ;

        [DllImport(DllName, EntryPoint="libpd_finish_message", CallingConvention = CallingConvention)]
        internal static extern  int finish_message([In] [MarshalAs(UnmanagedType.LPStr)] string recv, [In] [MarshalAs(UnmanagedType.LPStr)] string msg) ;

        [DllImport(DllName, EntryPoint="libpd_finish_list", CallingConvention = CallingConvention)]
        internal static extern  int finish_list([In] [MarshalAs(UnmanagedType.LPStr)] string recv) ;

        [DllImport(DllName, EntryPoint="libpd_add_float", CallingConvention = CallingConvention)]
        internal static extern  void add_float(float x) ;

        [DllImport(DllName, EntryPoint="libpd_add_symbol", CallingConvention = CallingConvention)]
        internal static extern  void add_symbol([In] [MarshalAs(UnmanagedType.LPStr)] string sym) ;
    }
}