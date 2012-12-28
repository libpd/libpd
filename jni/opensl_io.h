/*
 * Copyright 2012 Peter Brinkmann (peter.brinkmann@gmail.com)
 *
 * Based on sample code by Victor Lazzarini, available at
 * http://audioprograming.wordpress.com/2012/03/03/android-audio-streaming-with-opensl-es-and-the-ndk/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __OPENSL_IO_HPP__
#define __OPENSL_IO_HPP__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Processing callback; takes a processing context (which is just a pointer to
 * whatever data you want to pass to the callback), the sample rate, the
 * buffer size in frames, the number of input and output channels, as well as
 * input and output buffers whose size must be the number of channels times
 * the number of frames per buffer.
 */
typedef void (*opensl_process_t)
    (void *context, int sample_rate, int buffer_frames,
     int input_channels, const short *input_buffer,
     int output_channels, short *output_buffer);

/*
 * Abstract data type for streaming audio with OpenSL.
 */
struct _opensl_stream;
typedef struct _opensl_stream OPENSL_STREAM;

/*
 * Opens the audio device for the given sample rate and channel configuration;
 * registers an audio processing callback that will receive a context pointer
 * (which may be NULL if no context is needed).  The context is owned by the
 * caller.
 *
 * The internal buffer sizes are the buffer sizes (in frames) at which OpenSL
 * operates; 512 is a reasonable default. As of Android 4.2, the AudioManager
 * class in Java offers a method that will yield the recommended sample rate
 * and buffer size for OpenSL.
 *
 * The external buffer size is the buffer size for the processing callback. For
 * example, Pd (via libpd) uses 64 frames per buffer.
 *
 * For the time being, each channel number must be 0, 1, or 2; at least one
 * channel number must be nonzero.
 *
 * Returns NULL on failure.
 */
OPENSL_STREAM *opensl_open(
    int sample_rate, int input_channels, int output_channels,
    int internal__input_buffer_size, int internal__output_buffer_size,
    int external_buffer_size, opensl_process_t proc, void *context);

/*
 * Stops playback and frees all resources associated with the given stream,
 * except for the context pointer, which is owned by the caller; the cleanup of
 * the context (if any) is the responsibility of the caller.
 */
void opensl_close(OPENSL_STREAM *p);

/*
 * Returns nonzero value if the given stream is currently running.
 */
int opensl_is_running(OPENSL_STREAM *p);

/*
 * Starts the audio stream.
 *
 * Returns 0 on success.
 */
int opensl_start(OPENSL_STREAM *p);

/*
 * Pauses the audio stream.
 */
void opensl_pause(OPENSL_STREAM *p);

#ifdef __cplusplus
};
#endif

#endif // #ifndef __OPENSL_IO_HPP__
