/**
 * 
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL WARRANTIES, see the
 * file, "LICENSE.txt," in this distribution.
 * 
 */

package org.puredata.core;

/**
 * PdBase natives loader.
 * 
 * @author mgsx
 *
 */
public abstract class PdBaseLoader {

	/**
	 * Load required native libraries prior to use {@link PdBase}.
	 */
	public abstract void load();
	
	/**
	 * Default {@link PdBaseLoader}. Called when {@link PdBase} is first accessed.
	 * This allows for implementing a custom loader depending on libpd use context. To do so,
	 * first change this {@link PdBaseLoader} with your own one prior to call any {@link PdBase} methods.
	 */
	public static PdBaseLoader loaderHandler = new PdBaseLoader() {
		@Override
		public void load() {
			try {
		      Class<?> inner[] = Class.forName("android.os.Build").getDeclaredClasses();
		      // Now we know we're running on an Android device.
		      System.loadLibrary("pd");
		      int version = -1;
		      for (Class<?> c : inner) {
		        if (c.getCanonicalName().equals("android.os.Build.VERSION")) {
		          try {
		            version = c.getDeclaredField("SDK_INT").getInt(null);
		          } catch (Exception e) {
		            version = 3; // SDK_INT is not available for Cupcake.
		          }
		          break;
		        }
		      }
		      if (version >= 9) {
		        System.out.println("loading pdnativeopensl for Android");
		        System.loadLibrary("pdnativeopensl");
		      } else {
		        System.out.println("loading pdnative for Android");
		        System.loadLibrary("pdnative");
		      }
		    } catch (Exception e) {
		      // Now we know we aren't running on an Android device.
		      NativeLoader.loadLibrary("libwinpthread-1", "windows");
		      NativeLoader.loadLibrary("pdnative");
		    }
		}
	};
}
