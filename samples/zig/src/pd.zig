const c = @import("m_pd.zig");

pub extern const pd_compatibilitylevel: c_int;

pub const Int = c.t_int;
pub const Float = c.t_float;
pub const Sample = c.t_sample;

pub const Method = c.t_method;
pub const NewMethod = c.t_newmethod;

pub const Word = extern union {
	// we're going to trust pd to give us valid pointers of the respective types
	float: Float,
	symbol: *Symbol,
	gpointer: *GPointer,
	array: *Array,
	binbuf: *BinBuf,
	index: c_int,
};

// ----------------------------------- Atom ------------------------------------
// -----------------------------------------------------------------------------
pub const Atom = extern struct {
	type: c.t_atomtype,
	w: Word,

	pub const int = c.atom_getint;
	pub const float = c.atom_getfloat;
	pub const symbol = c.atom_getsymbol;
	pub const toSymbol = c.atom_gensym;

	pub fn bufPrint(self: *const Atom, buf: []u8) void {
		c.atom_string(self, buf.ptr, @intCast(buf.len));
	}

	// static methods
	pub fn intArg(av: []const Atom, which: usize) Int {
		return c.atom_getintarg(@intCast(which), @intCast(av.len), av.ptr);
	}
	pub fn floatArg(av: []const Atom, which: usize) Float {
		return c.atom_getfloatarg(@intCast(which), @intCast(av.len), av.ptr);
	}
	pub fn symbolArg(av: []const Atom, which: usize) *Symbol {
		return c.atom_getsymbolarg(@intCast(which), @intCast(av.len), av.ptr);
	}
};

// variadic functions can't work with enum literals, even if the enum has
// an explicit tag type, so this enum is really just being used as a namespace.
pub const AtomType = enum {
	pub const NULL: u32 = 0;
	pub const FLOAT: u32 = 1;
	pub const SYMBOL: u32 = 2;
	pub const POINTER: u32 = 3;
	pub const SEMI: u32 = 4;
	pub const COMMA: u32 = 5;
	pub const DEFFLOAT: u32 = 6;
	pub const DEFSYM: u32 = 7;
	pub const DEFSYMBOL: u32 = 7;
	pub const DOLLAR: u32 = 8;
	pub const DOLLSYM: u32 = 9;
	pub const GIMME: u32 = 10;
	pub const CANT: u32 = 11;
};


// ---------------------------------- BinBuf -----------------------------------
// -----------------------------------------------------------------------------
pub const BinBuf = opaque {
	pub const free = c.binbuf_free;
	pub const duplicate = c.binbuf_duplicate;
	pub const fromText = c.binbuf_text;
	pub const text = c.binbuf_gettext;
	pub const clear = c.binbuf_clear;
	pub const add = c.binbuf_add;
	pub const addV = c.binbuf_addv;
	pub const addBinBuf = c.binbuf_addbinbuf;
	pub const addSemi = c.binbuf_addsemi;
	pub const restore = c.binbuf_restore;
	pub const print = c.binbuf_print;
	pub const nAtoms = c.binbuf_getnatom;
	pub const vec = c.binbuf_getvec;
	pub const eval = c.binbuf_eval;
	pub const read = c.binbuf_read;
	pub const readViaCanvas = c.binbuf_read_via_canvas;
	pub const readViaPath = c.binbuf_read_via_path;
	pub const write = c.binbuf_write;

	pub fn resize(self: *BinBuf, newsize: usize) !void {
		if (self.binbuf_resize(@intCast(newsize)) == 0)
			return error.OutOfMemory;
	}
};
pub const binbuf = c.binbuf_new;
pub const evalFile = c.binbuf_evalfile;
pub const realizeDollSym = c.binbuf_realizedollsym;


// ----------------------------------- Class -----------------------------------
// -----------------------------------------------------------------------------
pub const MethodEntry = extern struct {
	name: *Symbol,
	fun: GotFn,
	arg: [6]u8,
};

pub const BangMethod = ?*const fn (*Pd) callconv(.C) void;
pub const PointerMethod = ?*const fn (*Pd, *GPointer) callconv(.C) void;
pub const FloatMethod = ?*const fn (*Pd, Float) callconv(.C) void;
pub const SymbolMethod = ?*const fn (*Pd, *Symbol) callconv(.C) void;
pub const ListMethod = ?*const fn (*Pd, *Symbol, u32, [*]Atom) callconv(.C) void;
pub const AnyMethod = ?*const fn (*Pd, *Symbol, u32, [*]Atom) callconv(.C) void;

pub const SaveFn = ?*const fn (*GObj, ?*BinBuf) callconv(.C) void;
pub const PropertiesFn = ?*const fn (*GObj, *GList) callconv(.C) void;
pub const ClassFreeFn = ?*const fn (*Class) callconv(.C) void;

pub const Class = extern struct {
	name: *Symbol,
	helpname: *Symbol,
	externdir: *Symbol,
	size: usize,
	methods: [*]MethodEntry,
	nmethod: u32,
	freemethod: Method,
	bangmethod: BangMethod,
	pointermethod: PointerMethod,
	floatmethod: FloatMethod,
	symbolmethod: SymbolMethod,
	listmethod: ListMethod,
	anymethod: AnyMethod,
	wb: ?*const WidgetBehavior,
	pwb: ?*const ParentWidgetBehavior,
	savefn: SaveFn,
	propertiesfn: PropertiesFn,
	next: *Class,
	floatsignalin: c_int,
	flags: u8,
	classfreefn: ClassFreeFn,

	pub const free = c.class_free;
	pub const addMethod = c.class_addmethod;
	pub const addBang = c.class_addbang;
	pub const addPointer = c.class_addpointer;
	pub const addFloat = c.class_doaddfloat;
	pub const addSymbol = c.class_addsymbol;
	pub const addList = c.class_addlist;
	pub const addAnything = c.class_addanything;
	pub const setHelpSymbol = c.class_sethelpsymbol;
	pub const setWidget = c.class_setwidget;
	pub const setParentWidget = c.class_setparentwidget;
	pub const name = c.class_getname;
	pub const helpName = c.class_gethelpname;
	pub const helpDir = c.class_gethelpdir;
	pub const setDrawCommand = c.class_setdrawcommand;
	pub const doMainSignalIn = c.class_domainsignalin;
	pub const setSaveFn = c.class_setsavefn;
	pub const saveFn = c.class_getsavefn;
	pub const setPropertiesFn = c.class_setpropertiesfn;
	pub const propertiesFn = c.class_getpropertiesfn;
	pub const setFreeFn = c.class_setfreefn;
	pub const new = c.pd_new;

	pub fn isDrawCommand(self: *const Class) bool {
		return (self.class_isdrawcommand() != 0);
	}

	pub fn find(self: *const Class, sym: *Symbol) ?*Pd {
		return c.pd_findbyclass(sym, self);
	}

	pub const DEFAULT: u32 = 0;     // flags for new classes below
	pub const PD: u32 = 1;          // non-canvasable (bare) pd such as an inlet
	pub const GOBJ: u32 = 2;        // pd that can belong to a canvas
	pub const PATCHABLE: u32 = 3;   // pd that also can have inlets and outlets
	pub const TYPEMASK: u32 = 3;
	pub const NOINLET: u32 = 8;          // suppress left inlet
	pub const MULTICHANNEL: u32 = 0x10;  // can deal with multichannel signals
	pub const NOPROMOTESIG: u32 = 0x20;  // don't promote scalars to signals
	pub const NOPROMOTELEFT: u32 = 0x40; // not even the main (left) inlet
};
pub const class = c.class_new;
pub const class64 = c.class_new64;
pub const addCreator = c.class_addcreator;

pub extern const garray_class: *Class;
pub extern const scalar_class: *Class;
pub extern const glob_pdobject: *Class;


// ----------------------------------- Clock -----------------------------------
// -----------------------------------------------------------------------------
pub const Clock = opaque {
	pub const set = c.clock_set;
	pub const delay = c.clock_delay;
	pub const unset = c.clock_unset;
	pub const free = c.clock_free;

	pub fn setUnit(self: *Clock, timeunit: f64, in_samples: bool) void {
		c.clock_setunit(self, timeunit, @intFromBool(in_samples));
	}
};
pub const clock = c.clock_new;
pub const logicalTime = c.clock_getlogicaltime;
pub const sysTime = c.clock_getsystime;
pub const timeSince = c.clock_gettimesince;
pub const sysTimeAfter = c.clock_getsystimeafter;

pub fn timeSinceWithUnits(prevsystime: f64, units: f64, in_samples: bool) f64 {
	return c.clock_gettimesincewithunits(prevsystime, units, @intFromBool(in_samples));
}


// ------------------------------------ Dsp ------------------------------------
// -----------------------------------------------------------------------------
pub const PerfRoutine = c.t_perfroutine;
pub const dsp = struct {
	pub const add = c.dsp_add;
	pub const addV = c.dsp_addv;
	pub const addPlus = c.dsp_add_plus;
	pub const addCopy = c.dsp_add_copy;
	pub const addScalarCopy = c.dsp_add_scalarcopy;
	pub const addZero = c.dsp_add_zero;
};


// ---------------------------------- GArray -----------------------------------
// -----------------------------------------------------------------------------
pub const GArray = opaque {
	pub const floatArray = c.garray_getfloatarray;
	pub const redraw = c.garray_redraw;
	pub const nPoints = c.garray_npoints;
	pub const vec = c.garray_vec;
	pub const resize = c.garray_resize;
	pub const resizeLong = c.garray_resize_long;
	pub const useInDsp = c.garray_usedindsp;
	pub const setSaveIt = c.garray_setsaveit;
	pub const glist = c.garray_getglist;
	pub const array = c.garray_getarray;

	pub fn floatWords(self: *GArray) ?[]Word {
		var len: c_int = undefined;
		var ptr: [*]Word = undefined;
		return if (c.garray_getfloatwords(self, &len, &ptr) != 0)
			ptr[0..@intCast(len)] else null;
	}
};


// ----------------------------------- GList -----------------------------------
// -----------------------------------------------------------------------------
pub const GList = opaque {
	pub const makeFilename = c.canvas_makefilename;
	pub const dir = c.canvas_getdir;
	pub const dataProperties = c.canvas_dataproperties;
	pub const open = c.canvas_open;
	pub const sampleRate = c.canvas_getsr;
	pub const signalLength = c.canvas_getsignallength;
	pub const undoSetState = c.pd_undo_set_objectstate;
	// static methods
	pub const setArgs = c.canvas_setargs;
	pub const args = c.canvas_getargs;
	pub const current = c.canvas_getcurrent;
};


// --------------------------------- GPointer ----------------------------------
// -----------------------------------------------------------------------------
pub const Scalar = c.struct__scalar;
pub const GStub = extern struct {
	un: extern union {
		glist: *GList,
		array: *Array,
	},
	which: enum(c_int) { none, glist, array },
	refcount: c_int,
};

pub const GPointer = extern struct {
	un: extern union {
		scalar: *Scalar,
		w: *Word,
	},
	valid: c_int,
	stub: *GStub,

	pub const init = c.gpointer_init;
	pub const copy = c.gpointer_copy;
	pub const unset = c.gpointer_unset;
	pub const check = c.gpointer_check;
};


// ----------------------------------- Inlet -----------------------------------
// -----------------------------------------------------------------------------
pub const Inlet = extern struct {
	pd: Pd,
	next: ?*Inlet,
	owner: *Object,
	dest: ?*Pd,
	symfrom: *Symbol,
	un: extern union {
		symto: *Symbol,
		pointerslot: *GPointer,
		floatslot: *Float,
		symslot: **Symbol,
		floatsignalvalue: Float,
	},

	pub const free = c.inlet_free;
};


// ---------------------------------- Memory -----------------------------------
// -----------------------------------------------------------------------------
const Allocator = @import("std").mem.Allocator;
const assert = @import("std").debug.assert;

fn alloc(_: *anyopaque, len: usize, _: u8, _: usize) ?[*]u8 {
	assert(len > 0);
	return @ptrCast(c.getbytes(len));
}

fn resize(_: *anyopaque, buf: []u8, _: u8, new_len: usize, _: usize) bool {
	return (new_len <= buf.len);
}

fn free(_: *anyopaque, buf: []u8, _: u8, _: usize) void {
	c.freebytes(buf.ptr, buf.len);
}

const mem_vtable = Allocator.VTable{
	.alloc = alloc,
	.resize = resize,
	.free = free,
};
pub const mem = Allocator{
	.ptr = undefined,
	.vtable = &mem_vtable,
};


// ---------------------------------- Object -----------------------------------
// -----------------------------------------------------------------------------
pub const GObj = extern struct {
	pd: Pd,
	next: ?*GObj,
};

pub const Object = extern struct {
	g: GObj,            // header for graphical object
	binbuf: *BinBuf,    // holder for the text
	outlets: ?*Outlet,  // linked list of outlets
	inlets: ?*Inlet,    // linked list of inlets
	xpix: i16,          // x&y location (within the toplevel)
	ypix: i16,
	width: i16,         // requested width in chars, 0 if auto
	type: enum(u8) {
		text = 0,        // just a textual comment
		object = 1,      // a MAX style patchable object
		message = 2,     // a MAX type message
		atom = 3,        // a cell to display a number or symbol
	},

	pub const list = c.obj_list;
	pub const saveFormat = c.obj_saveformat;
	extern fn outlet_new(*Object, ?*Symbol) ?*Outlet;
	pub const outlet = outlet_new;
	extern fn inlet_new(*Object, *Pd, ?*Symbol, ?*Symbol) ?*Inlet;
	pub const inlet = inlet_new;
	pub const inletPointer = c.pointerinlet_new;
	pub const inletFloat = c.floatinlet_new;
	pub const inletSymbol = c.symbolinlet_new;
	pub const inletSignal = c.signalinlet_new;

	pub fn inletFloatArg(obj: *Object, fp: *Float, av: []const Atom, i: usize)
	*Inlet {
		fp.* = Atom.floatArg(av, i);
		return obj.inletFloat(fp).?;
	}

	pub fn inletSymbolArg(obj: *Object, sp: **Symbol, av: []const Atom, i: usize)
	*Inlet {
		sp.* = Atom.symbolArg(av, i);
		return obj.inletSymbol(sp);
	}

};


// ---------------------------------- Outlet -----------------------------------
// -----------------------------------------------------------------------------
pub const Outlet = opaque {
	pub const bang = c.outlet_bang;
	pub const pointer = c.outlet_pointer;
	pub const float = c.outlet_float;
	pub const symbol = c.outlet_symbol;
	extern fn outlet_list(*Outlet, ?*Symbol, c_int, [*]Atom) void;
	pub const list = outlet_list;
	pub const anything = c.outlet_anything;
	pub const toSymbol = c.outlet_getsymbol;
	pub const free = c.outlet_free;
};


// ------------------------------------ Pd -------------------------------------
// -----------------------------------------------------------------------------
pub const Pd = extern struct {
	_: *const Class,

	pub const free = c.pd_free;
	pub const bind = c.pd_bind;
	pub const unbind = c.pd_unbind;
	pub const pushSymbol = c.pd_pushsym;
	pub const popSymbol = c.pd_popsym;
	pub const bang = c.pd_bang;
	pub const pointer = c.pd_pointer;
	pub const float = c.pd_float;
	pub const symbol = c.pd_symbol;
	pub const list = c.pd_list;
	pub const anything = c.pd_anything;
	pub const vMess = c.pd_vmess;
	pub const typedMess = c.pd_typedmess;
	pub const forwardMess = c.pd_forwardmess;
	pub const checkObject = c.pd_checkobject;
	pub const parentWidget = c.pd_getparentwidget;
	pub const gfxStub = c.gfxstub_new;
	pub const guiStub = c.pdgui_stub_vnew;
	pub const func = c.getfn;
	pub const zFunc = c.zgetfn;
	pub const newest = c.pd_newest; // static
};


// --------------------------------- Resample ----------------------------------
// -----------------------------------------------------------------------------
pub const Resample = extern struct {
	method: i32,
	downsample: i32,
	upsample: i32,
	vec: [*]Sample,
	n: u32,
	coeffs: [*]Sample,
	coefsize: u32,
	buffer: [*]Sample,
	bufsize: u32,

	pub const init = c.resample_init;
	pub const free = c.resample_free;
	pub const dsp = c.resample_dsp;
	pub const fromDsp = c.resamplefrom_dsp;
	pub const toDsp = c.resampleto_dsp;
};


// ---------------------------------- Signal -----------------------------------
// -----------------------------------------------------------------------------
pub const Signal = extern struct {
	len: u32,
	vec: [*]Sample,
	srate: Float,
	nchans: c_int,
	overlap: c_int,
	refcount: c_int,
	isborrowed: c_int,
	isscalar: c_int,
	borrowedfrom: ?*Signal,
	nextfree: ?*Signal,
	nextused: ?*Signal,
	nalloc: c_int,
};
pub const signal = c.signal_new;
pub const setMultiOut = c.signal_setmultiout;


// ---------------------------------- Symbol -----------------------------------
// -----------------------------------------------------------------------------
pub const Symbol = extern struct {
	name: [*:0]const u8,
	thing: ?*Pd,
	next: ?*Symbol,

	pub const setExternDir = c.class_set_extern_dir;
	pub const buf = c.text_getbufbyname;
	pub const notify = c.text_notifybyname;
	pub const val = c.value_get;
	pub const releaseVal = c.value_release;
	pub const float = c.value_getfloat;
	pub const setFloat = c.value_setfloat;
};
pub const symbol = c.gensym;

pub const s = struct {
	pub const pointer = &c.s_pointer;
	pub const float = &c.s_float;
	pub const symbol = &c.s_symbol;
	pub const bang = &c.s_bang;
	pub const list = &c.s_list;
	pub const anything = &c.s_anything;
	pub const signal = &c.s_signal;
	pub const _N = &c.s__N;
	pub const _X = &c.s__X;
	pub const x = &c.s_x;
	pub const y = &c.s_y;
	pub const _ = &c.s_;
};


// ---------------------------------- System -----------------------------------
// -----------------------------------------------------------------------------
pub const blockSize = c.sys_getblksize;
pub const sampleRate = c.sys_getsr;
pub const inChannels = c.sys_get_inchannels;
pub const outChannels = c.sys_get_outchannels;
pub const vgui = c.sys_vgui;
pub const gui = c.sys_gui;
pub const pretendGuiBytes = c.sys_pretendguibytes;
pub const queueGui = c.sys_queuegui;
pub const unqueueGui = c.sys_unqueuegui;
pub const version = c.sys_getversion;
pub const floatSize = c.sys_getfloatsize;
pub const realTime = c.sys_getrealtime;
pub const open = c.sys_open;
pub const close = c.sys_close;
pub const fopen = c.sys_fopen;
pub const fclose = c.sys_fclose;
pub const lock = c.sys_lock;
pub const unlock = c.sys_unlock;
pub const tryLock = c.sys_trylock;
pub const bashFilename = c.sys_bashfilename;
pub const unbashFilename = c.sys_unbashfilename;
pub const hostFontSize = c.sys_hostfontsize;
pub const zoomFontWidth = c.sys_zoomfontwidth;
pub const zoomFontHeight = c.sys_zoomfontheight;
pub const fontWidth = c.sys_fontwidth;
pub const fontHeight = c.sys_fontheight;

pub fn isAbsolutePath(dir: [*:0]const u8) bool {
	return (c.sys_isabsolutepath(dir) != 0);
}

pub const currentDir = c.canvas_getcurrentdir;
pub const suspendDsp = c.canvas_suspend_dsp;
pub const resumeDsp = c.canvas_resume_dsp;
pub const updateDsp = c.canvas_update_dsp;

pub const setFileName = c.glob_setfilename;

pub const canvasList = c.pd_getcanvaslist;
pub const dspState = c.pd_getdspstate;
pub const err = c.pd_error;


// ----------------------------------- Misc. -----------------------------------
// -----------------------------------------------------------------------------
pub const OutConnect = opaque {};
pub const Template = opaque {};
pub const Array = opaque {};

pub extern const pd_objectmaker: Pd;
pub extern const pd_canvasmaker: Pd;

pub const GotFn = c.t_gotfn;
pub const GotFn1 = c.t_gotfn1;
pub const GotFn2 = c.t_gotfn2;
pub const GotFn3 = c.t_gotfn3;
pub const GotFn4 = c.t_gotfn4;
pub const GotFn5 = c.t_gotfn5;

pub const nullFn = c.nullfn;

pub const sys_font = c.sys_font;
pub const sys_fontweight = c.sys_fontweight;

pub const WidgetBehavior = opaque {};
pub const ParentWidgetBehavior = opaque {};

pub const post = c.post;
pub const startPost = c.startpost;
pub const postString = c.poststring;
pub const postFloat = c.postfloat;
pub const postAtom = c.postatom;
pub const endPost = c.endpost;
pub const bug = c.bug;

pub const LogLevel = enum(c_uint) {
	critical,
	err,
	normal,
	debug,
	verbose,
};
extern fn logpost(?*const anyopaque, LogLevel, [*]const u8, ...) void;
pub const logPost = logpost;
pub extern fn verbose(LogLevel, [*]const u8, ...) void;

pub const openViaPath = c.open_via_path;
pub const getEventNo = c.sched_geteventno;

pub extern var sys_idlehook: ?*const fn () callconv(.C) c_int;

pub const plusPerform = c.plus_perform;
pub const plusPerf8 = c.plus_perf8;
pub const zeroPerform = c.zero_perform;
pub const zeroPerf8 = c.zero_perf8;
pub const copyPerform = c.copy_perform;
pub const copyPerf8 = c.copy_perf8;
pub const scalarCopyPerform = c.scalarcopy_perform;
pub const scalarCopyPerf8 = c.scalarcopy_perf8;


pub const mayer = struct {
	pub const fht = c.mayer_fht;
	pub const fft = c.mayer_fft;
	pub const ifft = c.mayer_ifft;
	pub const realfft = c.mayer_realfft;
	pub const realifft = c.mayer_realifft;
};
pub const fft = c.pd_fft;

pub extern const cos_table: [*]f32;

pub fn ulog2(n: u64) u6 {
	var i = n;
	var r: u6 = 0;
	while (i > 1) {
		r += 1;
		i >>= 1;
	}
	return r;
}

pub const mToF = c.mtof;
pub const fToM = c.ftom;
pub const rmsToDb = c.rmstodb;
pub const powToDb = c.powtodb;
pub const dbToRms = c.dbtorms;
pub const dbToPow = c.dbtopow;
pub const q8Sqrt = c.q8_sqrt;
pub const q8Rsqrt = c.q8_rsqrt;
pub const qSqrt = c.qsqrt;
pub const qRsqrt = c.qrsqrt;

pub const GuiCallbackFn = c.t_guicallbackfn;
pub const gfxStubDeleteForKey = c.gfxstub_deleteforkey;
pub const pdGuiVmess = c.pdgui_vmess;
pub const pdGuiStubDeleteForKey = c.pdgui_stub_deleteforkey;
pub const cExtern = c.c_extern;
pub const cAddMess = c.c_addmess;

pub fn badFloat(f: Float) bool {
	return c.PD_BADFLOAT(f) != 0;
}
pub fn bigOrSmall(f: Float) bool {
	return c.PD_BIGORSMALL(f) != 0;
}

pub const MidiInstance = opaque {};
pub const InterfaceInstance = opaque {};
pub const CanvasInstance = opaque {};
pub const UgenInstance = opaque {};
pub const StuffInstance = opaque {};
pub const PdInstance = extern struct {
	systime: f64,
	clock_setlist: *Clock,
	canvaslist: *GList,
	templatelist: *Template,
	instanceno: c_int,
	symhash: **Symbol,
	midi: ?*MidiInstance,
	inter: ?*InterfaceInstance,
	ugen: ?*UgenInstance,
	gui: ?*CanvasInstance,
	stuff: ?*StuffInstance,
	newest: ?*Pd,
	islocked: c_int,
};

pub const this = &c.pd_maininstance;

pub const MAXPDSTRING = c.MAXPDSTRING;
pub const MAXPDARG = c.MAXPDARG;
pub const MAXLOGSIG = c.MAXLOGSIG;
pub const MAXSIGSIZE = c.MAXSIGSIZE;

pub const LOGCOSTABSIZE = c.LOGCOSTABSIZE;
pub const COSTABSIZE = c.COSTABSIZE;

pub const PD_USE_TE_XPIX = c.PD_USE_TE_XPIX;
pub const PDTHREADS = c.PDTHREADS;
pub const PERTHREAD = c.PERTHREAD;
