const rl = @import("raylib");
const lpd = @import("libpd.zig");
const strlen = @import("std").mem.len;

const audio = struct {
	var stream: rl.AudioStream = undefined;

	const sampleRate = 48000;
	const bitDepth = 16;
	const channels = 2;
	const block = 64 * channels;
	const c_exp: u3 = @log2(@as(f32, @floatFromInt(channels)));
	const b_exp: u3 = @log2(@as(f32, @floatFromInt(block)));

	var bridge: [block]i16 = undefined;
	var onset: usize = block;

	fn callback(buffer: ?*anyopaque, frames: c_uint) callconv(.C) void {
		var buf: []i16 = @as([*]i16, @alignCast(@ptrCast(buffer orelse return)))
			[0..frames << c_exp];
		const prev = block - onset;
		@memcpy(buf[0..prev], bridge[onset..]);
		buf = buf[prev..];

		// pd delivers samples in blocks, so we round down to a multiple of block size
		const ticks = buf.len >> b_exp;
		_ = lpd.processShort(@intCast(ticks), null, buf.ptr);
		buf = buf[ticks << b_exp..];

		if (buf.len > 0) {
			// fill the buffer with part of a block and use the rest in next callback
			_ = lpd.processShort(1, null, &bridge);
			@memcpy(buf, bridge[0..buf.len]);
			onset = buf.len;
		} else {
			onset = block;
		}
	}

	fn printHook(msg: [*c]const u8) callconv(.C) void {
		rl.traceLog(.log_info, @ptrCast(msg[0..strlen(msg)]));
	}

	fn init() !void {
		rl.initAudioDevice();
		errdefer rl.closeAudioDevice();

		// may help with audio stuttering
		//rl.setAudioStreamBufferSizeDefault(4096);

		stream = rl.loadAudioStream(sampleRate, bitDepth, channels);
		errdefer rl.unloadAudioStream(stream);

		rl.setAudioStreamCallback(stream, &callback);
		rl.playAudioStream(stream);

		// Pd initialization
		try lpd.init(0, channels, sampleRate);

		// subscribe to receive source names
		_ = lpd.bind("toZig");
		_ = lpd.bind("env");

		// set hooks
		lpd.setPrintHook(&printHook);

		// add the data/pd folder to the search path
		lpd.addToSearchPath("pd/lib");

		// audio processing on
		lpd.computeAudio(true);
	}

	fn close() void {
		lpd.computeAudio(false);
		rl.traceLog(.log_info, "PD: audio processing stopped successfully");
		rl.unloadAudioStream(stream);
		rl.closeAudioDevice();
	}
};

pub fn main() !void {
	//---------------------------------------------------------------------------
	// Initialization
	const screenWidth = 800;
	const screenHeight = 450;
	rl.initWindow(screenWidth, screenHeight,
		"raylib-zig [core] example - libpd audio streaming");
	defer rl.closeWindow();

	try audio.init();
	defer audio.close();

	const patch = lpd.openFile("test.pd", "./pd") orelse return;
	defer lpd.closeFile(patch);

	rl.setTargetFPS(30); // Set our game to run at 30 frames-per-second

	//---------------------------------------------------------------------------
	// Main game loop
	while (!rl.windowShouldClose()) { // Detect window close button or ESC key
		//------------------------------------------------------------------------
		// Update
		if (rl.isMouseButtonPressed(.mouse_button_left)) {
			lpd.sendBang("tone")
				catch rl.traceLog(.log_warning, "couldn't find `tone`");
		}
		if (rl.isMouseButtonPressed(.mouse_button_right)) {
			lpd.sendBang("fromZig")
				catch rl.traceLog(.log_warning, "couldn't find `fromZig`");
		}

		//------------------------------------------------------------------------
		// Draw
		rl.beginDrawing();
		defer rl.endDrawing();

		if (rl.isMouseButtonDown(.mouse_button_left)) {
			rl.drawText("mouse1", rl.getScreenWidth() - 220, 10, 20, rl.Color.red);
		}

		rl.clearBackground(rl.Color.ray_white);
	}
}
