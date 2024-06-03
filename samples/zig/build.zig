const std = @import("std");

pub fn build(b: *std.Build) !void {
	const target = b.standardTargetOptions(.{});
	const optimize = b.standardOptimizeOption(.{});

	//---------------------------------------------------------------------------
	// Raylib dependency
	const raylib_dep = b.dependency("raylib", .{
		.target = target, .optimize = optimize,
	});
	const rayzig_dep = b.dependency("raylib_zig", .{
		.target = target, .optimize = optimize,
	});
	const raylib = rayzig_dep.module("raylib");
	const raylib_artifact = raylib_dep.artifact("raylib");
	raylib.linkLibrary(raylib_artifact);

	//---------------------------------------------------------------------------
	// Pd library and module
	const libpd_dep = b.dependency("libpd_zig", .{
		.target = target,
		.optimize = optimize,
	});
	const pd = libpd_dep.module("pd");
	const libpd = libpd_dep.artifact("pd");

	//---------------------------------------------------------------------------
	// Web export
	if (target.query.os_tag == .emscripten) {
		const exe_lib = b.addStaticLibrary(.{
			.name = "pdtest",
			.root_source_file = b.path("src/main.zig"),
			.target = target,
			.optimize = optimize,
		});
		exe_lib.root_module.addImport("pd", pd);
		exe_lib.root_module.addImport("raylib", raylib);

		// Note that raylib itself is not actually added to the exe_lib output file,
		// so it also needs to be linked with emscripten.
		const emcc = @import("raylib_zig").emcc;
		var libs = std.ArrayList(*std.Build.Step.Compile).init(b.allocator);
		defer libs.deinit();
		try libs.appendSlice(&.{ exe_lib, raylib_artifact, libpd });
		const link_step = try emcc.linkWithEmscripten(b, libs.items);
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
	exe.root_module.addImport("pd", pd);
	exe.root_module.addImport("raylib", raylib);

	//---------------------------------------------------------------------------
	// Add run step
	const run_cmd = b.addRunArtifact(exe);
	run_cmd.step.dependOn(b.getInstallStep());
	const run_step = b.step("run", "Run the app");
	run_step.dependOn(&run_cmd.step);
}
