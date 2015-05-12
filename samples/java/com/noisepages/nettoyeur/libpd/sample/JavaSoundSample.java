/**
 * 
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 * 
 */

package com.noisepages.nettoyeur.libpd.sample;

import java.io.IOException;

import org.puredata.core.PdBase;

/**
 *
 * Minimal tutorial example of how to use libpd with JavaSound.
 *
 * @author Peter Brinkmann (peter.brinkmann@gmail.com)
 *
 */
public class JavaSoundSample {

	public static void main(String[] args) throws InterruptedException, IOException {
		JavaSoundThread audioThread = new JavaSoundThread(44100, 2, 16);
		int patch = PdBase.openPatch("samples/java/com/noisepages/nettoyeur/libpd/sample/test.pd");
		audioThread.start();
		Thread.sleep(5000);  // Sleep for five seconds; this is where the main application code would go in a real program.
		audioThread.interrupt();
		audioThread.join();
		PdBase.closePatch(patch);
	}
}
