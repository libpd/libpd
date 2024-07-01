const std = @import("std");

pub const Options = struct {
	static: bool = true,
	util: bool = true,
	extra: bool = true,
	multi: bool = false,
	double: bool = false,
};

pub fn build(b: *std.Build) !void {
	const target = b.standardTargetOptions(.{});
	const optimize = b.standardOptimizeOption(.{});

	//---------------------------------------------------------------------------
	// Raylib dependency
	const raylib_dep = b.dependency("raylib_zig", .{
		.target = target,
		.optimize = optimize,
	});
	const raylib = raylib_dep.module("raylib");
	const raylib_artifact = raylib_dep.artifact("raylib");

	//---------------------------------------------------------------------------
	// Libpd dependency
	const libpd = try addLibPd(b, target, optimize, .{});

	//---------------------------------------------------------------------------
	// Web export
	if (target.query.os_tag == .emscripten) {
		const emcc = @import("emcc.zig");
		const exe_lib = emcc.compileForEmscripten(
			b, "pdtest", "src/main.zig", target, optimize);
		exe_lib.linkLibrary(libpd);
		exe_lib.linkLibrary(raylib_artifact);
		exe_lib.root_module.addImport("raylib", raylib);

		// Note that raylib itself is not actually added to the exe_lib output file,
		// so it also needs to be linked with emscripten.
		const link_step = try emcc.linkWithEmscripten(
			b, &[_]*std.Build.Step.Compile{ exe_lib, raylib_artifact, libpd });
		link_step.addArg("-sUSE_OFFSET_CONVERTER");
		link_step.addArg("--preload-file=pd");

		b.getInstallStep().dependOn(&link_step.step);
		const run_step = try emcc.emscriptenRunStep(b);
		run_step.step.dependOn(&link_step.step);
		const run_option = b.step("run", "Run the app");
		run_option.dependOn(&run_step.step);
		return;
	}

	//---------------------------------------------------------------------------
	// Add executable
	const exe = b.addExecutable(.{
		.name = "pdtest",
		.root_source_file = b.path("src/main.zig"),
		.target = target,
		.optimize = optimize,
	});
	b.installArtifact(exe);
	exe.linkLibrary(libpd);
	exe.linkLibrary(raylib_artifact);
	exe.root_module.addImport("raylib", raylib);

	//---------------------------------------------------------------------------
	// Add run step
	const run_cmd = b.addRunArtifact(exe);
	run_cmd.step.dependOn(b.getInstallStep());
	const run_step = b.step("run", "Run the app");
	run_step.dependOn(&run_cmd.step);

	//---------------------------------------------------------------------------
	// Add test step
	const unit_tests = b.addTest(.{
		.root_source_file = b.path("src/libpd.zig"),
		.target = target,
		.optimize = optimize,
		.link_libc = true,
	});
	unit_tests.linkLibrary(libpd);
	const run_unit_tests = b.addRunArtifact(unit_tests);
	const test_step = b.step("test", "Run unit tests");
	test_step.dependOn(&run_unit_tests.step);
}

pub fn addLibPd(
	b: *std.Build,
	target: std.Build.ResolvedTarget,
	optimize: std.builtin.OptimizeMode,
	options: Options,
) !*std.Build.Step.Compile {
	const lib = if (options.static) b.addStaticLibrary(.{
		.name = "pd", .target = target, .optimize = optimize
	}) else b.addSharedLibrary(.{
		.name = "pd", .target = target, .optimize = optimize, .pic = true
	});
	lib.linkLibC();

	const root = "../../";
	const src = "pure-data/src/";
	const wrap = "libpd_wrapper/";
	const util = wrap ++ "util/";

	var files = std.ArrayList([]const u8).init(b.allocator);
	for (&[_][]const u8{
		"d_arithmetic", "d_array", "d_ctl", "d_dac", "d_delay", "d_fft",
		"d_fft_fftsg", "d_filter", "d_global", "d_math", "d_misc", "d_osc",
		"d_resample", "d_soundfile", "d_soundfile_aiff", "d_soundfile_caf",
		"d_soundfile_next", "d_soundfile_wave", "d_ugen",
		"g_all_guis", "g_array", "g_bang", "g_canvas", "g_clone", "g_editor",
		"g_editor_extras", "g_graph", "g_guiconnect", "g_io", "g_mycanvas",
		"g_numbox", "g_radio", "g_readwrite", "g_rtext", "g_scalar",
		"g_slider", "g_template", "g_text", "g_toggle", "g_traversal",
		"g_undo", "g_vumeter",
		"m_atom", "m_binbuf", "m_class", "m_conf", "m_glob", "m_memory",
		"m_obj", "m_pd", "m_sched",
		"s_audio", "s_audio_dummy", "s_inter", "s_inter_gui", "s_loader",
		"s_main", "s_net", "s_path", "s_print", "s_utf8",
		"x_acoustics", "x_arithmetic", "x_array", "x_connective", "x_file",
		"x_gui", "x_interface", "x_list", "x_midi", "x_misc", "x_net",
		"x_scalar", "x_text", "x_time", "x_vexp", "x_vexp_if", "x_vexp_fun",
	}) |s| {
		try files.append(b.fmt("{s}{s}{s}", .{ src, s, ".c" }));
	}

	for (&[_][]const u8{ "s_libpdmidi", "x_libpdreceive", "z_hooks", "z_libpd" }) |s| {
		try files.append(b.fmt("{s}{s}{s}", .{ wrap, s, ".c" }));
	}

	if (options.util) {
		for (&[_][]const u8{ "z_print_util", "z_queued", "z_ringbuffer" }) |s| {
			try files.append(b.fmt("{s}{s}{s}", .{ util, s, ".c" }));
		}
	}

	if (options.extra) {
		const extra = "pure-data/extra/";
		for (&[_][]const u8{
			"bob~", "bonk~", "choice", "fiddle~", "loop~", "lrshift~", "pique", "pd~",
			"sigmund~", "stdout",
		}) |s| {
			try files.append(b.fmt("{s}{s}/{s}{s}", .{ extra, s, s, ".c" }));
		}
		try files.append(extra ++ "pd~/pdsched.c");
		lib.defineCMacro("LIBPD_EXTRA", null);
	}

	lib.defineCMacro("PD", null);
	lib.defineCMacro("PD_INTERNAL", null);
	lib.defineCMacro("USEAPI_DUMMY", null);
	lib.defineCMacro("HAVE_UNISTD_H", null);

	if (options.multi) {
		lib.defineCMacro("PDINSTANCE", null);
		lib.defineCMacro("PDTHREADS", null);
	}

	if (options.double) {
		lib.defineCMacro("PD_FLOATSIZE", "64");
	}

	lib.addIncludePath(b.path(root ++ wrap));
	lib.addIncludePath(b.path(root ++ util));
	lib.addIncludePath(b.path(root ++ src));

	var flags = std.ArrayList([]const u8).init(b.allocator);
	try flags.append("-fno-sanitize=undefined");
	if (optimize != .Debug) {
		try flags.appendSlice(&.{
			"-ffast-math", "-funroll-loops", "-fomit-frame-pointer"
		});
	}

	const os = target.result.os.tag;
	switch (os) {
		.windows => {
			lib.defineCMacro("WINVER", "0x502");
			lib.defineCMacro("WIN32", null);
			lib.defineCMacro("_WIN32", null);
			lib.linkSystemLibrary("ws2_32");
			lib.linkSystemLibrary("kernel32");
		},
		.macos => {
			lib.defineCMacro("HAVE_ALLOCA_H", null);
			lib.defineCMacro("HAVE_LIBDL", null);
			lib.defineCMacro("HAVE_MACHINE_ENDIAN_H", null);
			lib.defineCMacro("_DARWIN_C_SOURCE", null);
			lib.defineCMacro("_DARWIN_UNLIMITED_SELECT", null);
			lib.defineCMacro("FD_SETSIZE", "10240");
			lib.linkSystemLibrary("dynamiclib");
			lib.linkSystemLibrary("dl");
		},
		.emscripten, .wasi => {
			const sysroot = b.sysroot
				orelse @panic("Pass '--sysroot \"$EMSDK/upstream/emscripten\"'");
			const cache_include = std.fs.path.join(b.allocator, &.{
				sysroot, "cache", "sysroot", "include"
			}) catch @panic("Out of memory");
			defer b.allocator.free(cache_include);

			var dir = std.fs.openDirAbsolute(cache_include, std.fs.Dir.OpenDirOptions{
				.access_sub_paths = true, .no_follow = true
			}) catch @panic("No emscripten cache. Generate it!");
			dir.close();
			lib.addSystemIncludePath(.{ .cwd_relative = cache_include });
		},
		else => { // Linux and FreeBSD
			try flags.appendSlice(&.{
				"-Wno-int-to-pointer-cast", "-Wno-pointer-to-int-cast"
			});
			lib.defineCMacro("HAVE_ENDIAN_H", null);
			if (os == .linux) {
				lib.defineCMacro("HAVE_ALLOCA_H", null);
				lib.defineCMacro("HAVE_LIBDL", null);
				lib.linkSystemLibrary("dl");
			}
		}
	}

	if (os != .emscripten and os != .wasi and !options.static) {
		lib.linkSystemLibrary("pthread");
		lib.linkSystemLibrary("m");
	}

	lib.addCSourceFiles(.{
		.root = b.path(root),
		.files = files.items,
		.flags = flags.items,
	});

	return lib;
}
