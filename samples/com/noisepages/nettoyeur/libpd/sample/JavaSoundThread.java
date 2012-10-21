/**
 * 
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 * 
 */

package com.noisepages.nettoyeur.libpd.sample;

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
 * Simple example of how to use libpd with JavaSound.  This example currently only does audio output,
 * but it's easy enough to add support for audio input.
 *
 * Note that the simple expedient of requesting a source data line from AudioSystem may not be sound on systems
 * with multiple sound cards or other complications.  In general, you will need to deal with mixers and other
 * subtleties.
 *
 * @author Peter Brinkmann (peter.brinkmann@gmail.com)
 *
 */
public class JavaSoundThread extends Thread {

	private final float sampleRate;  // Sample rate in Hz.
	private final int outChans;      // Number of output channels.
	private final int ticks;         // Number of Pd ticks per buffer.
	private volatile boolean terminated;

	public JavaSoundThread(float sampleRate, int outChans, int ticks) {
		this.sampleRate = sampleRate;
		this.outChans = outChans;
		this.ticks = ticks;
		PdBase.openAudio(0, outChans, (int) sampleRate);
		PdBase.computeAudio(true);
		setPriority(Thread.MAX_PRIORITY);
	}
	
	@Override
	public void run() {
		terminated = false;
		try {
			perform();
		} catch (LineUnavailableException e) {
			e.printStackTrace();
		}
	}
	
	@Override
	public void interrupt() {
		terminated = true;  // Needed since the interrupted flag is cleared by JavaSound.
		super.interrupt();
	}
	
	private void perform() throws LineUnavailableException {
		// JavaSound setup.
		int sampleSize = 2;
		AudioFormat audioFormat = new AudioFormat(sampleRate, 8 * sampleSize, outChans, true, true);
		DataLine.Info info = new DataLine.Info(SourceDataLine.class, audioFormat);
		SourceDataLine sourceDataLine = (SourceDataLine) AudioSystem.getLine(info);
		sourceDataLine.open(audioFormat);
		sourceDataLine.start();

		// Buffer setup for exchanging samples between libpd and JavaSound.
		// Question: Is this the best possible solution?  It seems to involve too
		// much copying.
		int frames = PdBase.blockSize() * ticks;
		short[] dummy = new short[0];
		short[] samples = new short[frames * outChans];
		byte[] rawSamples = new byte[samples.length * sampleSize];
		ByteBuffer buf = ByteBuffer.wrap(rawSamples);
		ShortBuffer shortBuf = buf.asShortBuffer();

		while (!terminated) {  // Note: sourceDataLine.write seems to clear the interrupted flag, and so Thread.interrupted() doesn't work here.
			PdBase.process(ticks, dummy, samples);
			shortBuf.rewind();
			shortBuf.put(samples);
			sourceDataLine.write(rawSamples, 0, rawSamples.length);
		}

		// Shutdown.
		sourceDataLine.drain();
		sourceDataLine.stop();
		sourceDataLine.close();
	}
}
