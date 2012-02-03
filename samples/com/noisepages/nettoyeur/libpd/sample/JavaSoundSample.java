/**
 * 
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 * 
 */

package com.noisepages.nettoyeur.libpd.sample;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ShortBuffer;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.DataLine;
import javax.sound.sampled.LineUnavailableException;
import javax.sound.sampled.SourceDataLine;

import org.puredata.core.PdBase;

/**
 *
 * Minimal tutorial example of how to use libpd with JavaSound.  This example currently only does audio output,
 * but it should be easy enough to add support for audio input.
 *
 * @author Peter Brinkmann (peter.brinkmann@gmail.com)
 *
 */
public class JavaSoundSample {

	public static void main(String[] args) throws LineUnavailableException, IOException {
		
		// Audio configuration.
		float sampleRate = 44100;  // Sample rate in Hz.
		int outChans = 2;          // Number of output channels.
		int ticks = 16;            // Number of Pd ticks per buffer.
		
		// libpd configuration.
		PdBase.openAudio(0, outChans, (int) sampleRate);
		int patch = PdBase.openPatch("samples/com/noisepages/nettoyeur/libpd/sample/test.pd");
		PdBase.computeAudio(true);
		
		// JavaSound setup.
		int sampleSize = 2;
		int frames = PdBase.blockSize() * ticks;
		AudioFormat audioFormat = new AudioFormat(sampleRate, 8 * sampleSize, outChans, true, true);
		DataLine.Info info = new DataLine.Info(SourceDataLine.class, audioFormat);
		SourceDataLine sourceDataLine = (SourceDataLine) AudioSystem.getLine(info);
		sourceDataLine.open(audioFormat);
		sourceDataLine.start();
		
		// Buffer setup for exchanging samples between libpd and JavaSound.
		// Question: Is this the best possible solution?  It seems to involve too
		// much copying.
		short[] dummy = new short[0];
		short[] samples = new short[frames * outChans];
		byte[] rawSamples = new byte[samples.length * sampleSize];
		ByteBuffer buf = ByteBuffer.wrap(rawSamples);
		ShortBuffer shortBuf = buf.asShortBuffer();
		
		// Audio rendering loop.
		long duration = 10;  // Run for ten seconds.
		for (long count = 0; count < duration * sampleRate; count += frames) {
			PdBase.process(ticks, dummy, samples);
			shortBuf.rewind();
			shortBuf.put(samples);
			sourceDataLine.write(rawSamples, 0, rawSamples.length);
		}
		
		// Shutdown.
		sourceDataLine.stop();
		PdBase.closePatch(patch);
	}
}
