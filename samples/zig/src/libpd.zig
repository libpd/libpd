const c = @import("m_pd.zig");
const pd = @import("pd.zig");

pub const clearSearchPath = c.libpd_clear_search_path;
pub const addToSearchPath = c.libpd_add_to_search_path;
pub const openFile = c.libpd_openfile;
pub const closeFile = c.libpd_closefile;
pub const getDollarZero = c.libpd_getdollarzero;
pub const blockSize = c.libpd_blocksize;

pub const processFloat = c.libpd_process_float;
pub const processShort = c.libpd_process_short;
pub const processDouble = c.libpd_process_double;
pub const processRawFloat = c.libpd_process_raw;
pub const processRawShort = c.libpd_process_raw_short;
pub const processRawDouble = c.libpd_process_raw_double;

pub const arraySize = c.libpd_arraysize;
pub const resizeArray = c.libpd_resize_array;
pub const readArray = c.libpd_read_array;
pub const writeArray = c.libpd_write_array;
pub const readArrayDouble = c.libpd_read_array_double;
pub const writeArrayDouble = c.libpd_write_array_double;

pub const addFloat = c.libpd_add_float;
pub const addDouble = c.libpd_add_double;
pub const addSymbol = c.libpd_add_symbol;
pub const setFloat = c.libpd_set_float;
pub const setDouble = c.libpd_set_double;
pub const setSymbol = c.libpd_set_symbol;

pub const bind = c.libpd_bind;
pub const unbind = c.libpd_unbind;
pub const exists = c.libpd_exists;

pub const PrintHook = c.t_libpd_printhook;
pub const BangHook = c.t_libpd_banghook;
pub const FloatHook = c.t_libpd_floathook;
pub const DoubleHook = c.t_libpd_doublehook;
pub const SymbolHook = c.t_libpd_symbolhook;
pub const ListHook = c.t_libpd_listhook;
pub const MessageHook = c.t_libpd_messagehook;
extern fn libpd_print_concatenator(s: [*c]const u8) void;
extern fn libpd_set_concatenated_printhook(hook: PrintHook) void;
pub const setPrintHook = libpd_set_concatenated_printhook;
pub const setBangHook = c.libpd_set_banghook;
pub const setFloatHook = c.libpd_set_floathook;
pub const setDoubleHook = c.libpd_set_doublehook;
pub const setSymbolHook = c.libpd_set_symbolhook;
pub const setListHook = c.libpd_set_listhook;
pub const setMessageHook = c.libpd_set_messagehook;

pub const isFloat = c.libpd_is_float;
pub const isSymbol = c.libpd_is_symbol;
pub const getFloat = c.libpd_get_float;
pub const getDouble = c.libpd_get_double;
pub const getSymbol = c.libpd_get_symbol;
pub const nextAtom = c.libpd_next_atom;

pub const NoteOnHook = c.t_libpd_noteonhook;
pub const ControlChangeHook = c.t_libpd_controlchangehook;
pub const ProgramChangeHook = c.t_libpd_programchangehook;
pub const PitchBendHook = c.t_libpd_pitchbendhook;
pub const AftertouchHook = c.t_libpd_aftertouchhook;
pub const PolyAftertouchHook = c.t_libpd_polyaftertouchhook;
pub const MidiByteHook = c.t_libpd_midibytehook;
pub const setNoteOnHook = c.libpd_set_noteonhook;
pub const setControlChangeHook = c.libpd_set_controlchangehook;
pub const setProgramChangeHook = c.libpd_set_programchangehook;
pub const setPitchBendHook = c.libpd_set_pitchbendhook;
pub const setAftertouchHook = c.libpd_set_aftertouchhook;
pub const setPolyAftertouchHook = c.libpd_set_polyaftertouchhook;
pub const setMidiByteHook = c.libpd_set_midibytehook;

pub const startGui = c.libpd_start_gui;
pub const stopGui = c.libpd_stop_gui;
pub const pollGui = c.libpd_poll_gui;
pub const newInstance = c.libpd_new_instance;
pub const setInstance = c.libpd_set_instance;
pub const freeInstance = c.libpd_free_instance;
pub const thisInstance = c.libpd_this_instance;
pub const mainInstance = c.libpd_main_instance;
pub const numInstances = c.libpd_num_instances;

pub const FreeHook = c.t_libpd_freehook;
pub const setInstanceData = c.libpd_set_instancedata;
pub const getInstanceData = c.libpd_get_instancedata;
pub const setVerbose = c.libpd_set_verbose;
pub const getVerbose = c.libpd_get_verbose;


pub fn init(ich: u32, och: u32, srate: u32) !void {
	if (c.libpd_init() != 0)
		return error.AlreadyInitialized;
	c.libpd_set_printhook(libpd_print_concatenator);
	if (c.libpd_init_audio(@intCast(ich), @intCast(och), @intCast(srate)) != 0)
		return error.InitAudioFail;
}

pub fn sendBang(recv: [*:0]const u8) !void {
	if (c.libpd_bang(recv) != 0)
		return error.ReceiverNotFound;
}

pub fn sendFloat(recv: [*:0]const u8, x: f32) !void {
	if (c.libpd_float(recv, x) != 0)
		return error.ReceiverNotFound;
}

pub fn sendDouble(recv: [*:0]const u8, x: f64) !void {
	if (c.libpd_double(recv, x) != 0)
		return error.ReceiverNotFound;
}

pub fn sendSymbol(recv: [*:0]const u8, s: [*:0]const u8) !void {
	if (c.libpd_symbol(recv, s) != 0)
		return error.ReceiverNotFound;
}

pub fn sendList(recv: [*:0]const u8, argc: c_int, argv: [*]pd.Atom) !void {
	if (c.libpd_list(recv, argc, argv) != 0)
		return error.ReceiverNotFound;
}

pub fn sendMessage(recv: [*:0]const u8, msg: [*:0]const u8, ac: c_int, av: [*]pd.Atom)
!void {
	if (c.libpd_message(recv, msg, ac, av) != 0)
		return error.ReceiverNotFound;
}

pub fn startMessage(maxlen: u32) !void {
	if (c.libpd_start_message(@intCast(maxlen)) != 0)
		return error.MessageTooLong;
}

pub fn finishList(recv: [*:0]const u8) !void {
	if (c.libpd_finish_list(recv) != 0)
		return error.ReceiverNotFound;
}

pub fn finishMessage(recv: [*:0]const u8, msg: [*:0]const u8) !void {
	if (c.libpd_finish_message(recv, msg) != 0)
		return error.ReceiverNotFound;
}

pub fn computeAudio(state: bool) void {
	_ = c.libpd_start_message(1);
	addFloat(@floatFromInt(@intFromBool(state)));
	_ = c.libpd_finish_message("pd", "dsp");
}


// Sending MIDI

pub fn sendNoteOn(channel: u32, pitch: u7, velocity: u7) void {
	_ = c.libpd_noteon(@intCast(channel), @intCast(pitch), @intCast(velocity));
}

pub fn sendControlChange(channel: u32, controller: u7, value: u7) void {
	_ = c.libpd_controlchange(@intCast(channel), @intCast(controller), @intCast(value));
}

pub fn sendProgramChange(channel: u32, value: u7) void {
	_ = c.libpd_programchange(@intCast(channel), @intCast(value));
}

pub fn sendPitchBend(channel: u32, value: i14) void {
	_ = c.libpd_pitchbend(@intCast(channel), @intCast(value));
}

pub fn sendAftertouch(channel: u32, value: u7) void {
	_ = c.libpd_aftertouch(@intCast(channel), @intCast(value));
}

pub fn sendPolyAftertouch(channel: u32, pitch: u7, value: u7) void {
	_ = c.libpd_polyaftertouch(@intCast(channel), @intCast(pitch), @intCast(value));
}

pub fn sendMidiByte(port: u12, byte: u8) void {
	_ = c.libpd_midibyte(@intCast(port), @intCast(byte));
}

pub fn sendSysex(port: u12, byte: u8) void {
	_ = c.libpd_sysex(@intCast(port), @intCast(byte));
}

pub fn sendSysRealTime(port: u12, byte: u8) void {
	_ = c.libpd_sysrealtime(@intCast(port), @intCast(byte));
}


const testing = @import("std").testing;

test init {
	try init(0, 2, 48000);
	try testing.expectError(error.AlreadyInitialized, init(0, 2, 48000));
}

test startMessage {
	try testing.expectError(error.MessageTooLong, startMessage(1234567890));
	try startMessage(1);
}
