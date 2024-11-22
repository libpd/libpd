const pd = @import("pd");
const rl = @import("raylib");
const strlen = @import("std").mem.len;

fn AudioController(
	sample_rate: u32,
	bit_depth: u32,
	channels: u32,
) type { return struct {
	const Self = @This();

	/// handles freeing of the ring buffer, if in queued mode
	base: pd.Base,
	/// raylib audio stream
	stream: rl.AudioStream,
	/// receiver for "toZig"
	rec_tozig: *anyopaque,
	/// receiver for "env"
	rec_env: *anyopaque,

	const block = 64 * channels;
	/// buffer that holds excess samples to be used in the next callback
	var bridge: [block]i16 = undefined;
	var onset: usize = block;

	fn callback(buffer: ?*anyopaque, frames: c_uint) callconv(.C) void {
		var buf: []i16 = @as([*]i16, @alignCast(@ptrCast(buffer orelse return)))
			[0..frames * channels];
		const prev = block - onset;
		@memcpy(buf[0..prev], bridge[onset..]);
		buf = buf[prev..];

		// pd delivers samples in blocks, so we round down to a multiple of block size
		const ticks = buf.len / block;
		pd.processShort(@intCast(ticks), null, buf.ptr) catch return;
		buf = buf[ticks * block..];

		if (buf.len > 0) {
			// fill the buffer with part of a block and use the rest in next callback
			pd.processShort(1, null, &bridge) catch return;
			@memcpy(buf, bridge[0..buf.len]);
			onset = buf.len;
		} else {
			onset = block;
		}
	}

	fn printHook(msg: [*c]const u8) callconv(.C) void {
		rl.traceLog(.log_info, @ptrCast(msg[0..strlen(msg)]));
	}

	fn init() !Self {
		rl.initAudioDevice();
		errdefer rl.closeAudioDevice();

		// may help with audio stuttering
		//rl.setAudioStreamBufferSizeDefault(4096);

		const stream = rl.loadAudioStream(sample_rate, bit_depth, channels);
		errdefer rl.unloadAudioStream(stream);

		rl.setAudioStreamCallback(stream, &callback);
		rl.playAudioStream(stream);

		// Pd initialization
		const base = try pd.Base.init(0, channels, sample_rate, false);
		errdefer base.close();

		// subscribe to receive source names
		const rec_tozig = try pd.bind("toZig");
		errdefer pd.unbind(rec_tozig);
		const rec_env = try pd.bind("env");

		// set hooks
		pd.setPrintHook(&printHook);

		// add the data/pd folder to the search path
		pd.addToSearchPath("pd/lib");

		// audio processing on
		pd.computeAudio(true);

		return Self{
			.base = base,
			.stream = stream,
			.rec_tozig = rec_tozig,
			.rec_env = rec_env
		};
	}

	fn close(self: *const Self) void {
		self.base.close();
		pd.unbind(self.rec_tozig);
		pd.unbind(self.rec_env);

		rl.traceLog(.log_info, "PD: audio processing stopped successfully");
		rl.unloadAudioStream(self.stream);
		rl.closeAudioDevice();
	}
};}

pub fn main() !void {
	//---------------------------------------------------------------------------
	// Initialization
	const screenWidth = 800;
	const screenHeight = 450;
	rl.initWindow(screenWidth, screenHeight,
		"raylib-zig [core] example - libpd audio streaming");
	defer rl.closeWindow();

	const audio = try AudioController(48000, 16, 2).init();
	defer audio.close();

	const patch = try pd.Patch.fromFile("test.pd", "./pd");
	defer patch.close();

	rl.setTargetFPS(30); // Set our game to run at 30 frames-per-second

	//---------------------------------------------------------------------------
	// Main game loop
	while (!rl.windowShouldClose()) { // Detect window close button or ESC key
		//------------------------------------------------------------------------
		// Update
		if (rl.isMouseButtonPressed(.mouse_button_left)) {
			pd.sendBang("tone")
				catch rl.traceLog(.log_warning, "couldn't find `tone`");
		}
		if (rl.isMouseButtonPressed(.mouse_button_right)) {
			pd.sendBang("fromZig")
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
