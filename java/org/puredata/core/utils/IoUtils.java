/**
 * 
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL WARRANTIES, see the
 * file, "LICENSE.txt," in this distribution.
 * 
 */

package org.puredata.core.utils;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

/**
 * 
 * IoUtils is a collection of convenience methods for dealing with file I/O.
 * 
 * @author Peter Brinkmann (peter.brinkmann@gmail.com)
 * 
 */
public class IoUtils {

  /**
   * Extract a resource into a real file
   * 
   * @param in typically given as getResources().openRawResource(R.raw.something)
   * @param filename name of the resulting file
   * @param directory target directory
   * @return the resulting file
   * @throws IOException
   */
  public static File extractResource(InputStream in, String filename, File directory)
      throws IOException {
    int n = in.available();
    byte[] buffer = new byte[n];
    in.read(buffer);
    in.close();
    File file = new File(directory, filename);
    FileOutputStream out = new FileOutputStream(file);
    out.write(buffer);
    out.close();
    return file;
  }

  /**
   * Extract a zip resource into real files and directories, not overwriting existing files
   * 
   * @param in typically given as getResources().openRawResource(R.raw.something)
   * @param directory target directory
   * @return list of files that were unpacked, not including files that existed before
   * @throws IOException
   */
  public static List<File> extractZipResource(InputStream in, File directory) throws IOException {
    return extractZipResource(in, directory, false);
  }

  /**
   * Extract a zip resource into real files and directories
   * 
   * @param in typically given as getResources().openRawResource(R.raw.something)
   * @param directory target directory
   * @param overwrite indicates whether to overwrite existing files
   * @return list of files that were unpacked (if overwrite is false, this list won't include files
   *         that existed before)
   * @throws IOException
   */
  public static List<File> extractZipResource(InputStream in, File directory, boolean overwrite)
      throws IOException {
    final int BUFSIZE = 2048;
    byte buffer[] = new byte[BUFSIZE];
    ZipInputStream zin = new ZipInputStream(new BufferedInputStream(in, BUFSIZE));
    List<File> files = new ArrayList<File>();
    ZipEntry entry;
    directory.mkdirs();
    while ((entry = zin.getNextEntry()) != null) {
      File file = new File(directory, entry.getName());
      files.add(file);
      if (overwrite || !file.exists()) {
        if (entry.isDirectory()) {
          file.mkdirs();
        } else {
          file.getParentFile().mkdirs(); // Necessary because some zip files lack directory entries.
          BufferedOutputStream bos = new BufferedOutputStream(new FileOutputStream(file), BUFSIZE);
          int nRead;
          while ((nRead = zin.read(buffer, 0, BUFSIZE)) > 0) {
            bos.write(buffer, 0, nRead);
          }
          bos.flush();
          bos.close();
        }
      }
    }
    zin.close();
    return files;
  }

  /**
   * Find all files matching a given pattern in the given directory and below
   * 
   * @param dir
   * @param pattern
   * @return
   */
  public static List<File> find(File dir, String pattern) {
    final List<File> hits = new ArrayList<File>();
    final Pattern p = Pattern.compile(pattern);
    traverseTree(dir, new FileProcessor() {
      @Override
      public void processFile(File file) {
        if (p.matcher(file.getName()).matches()) {
          hits.add(file);
        }
      }
    });
    return hits;
  }

  private interface FileProcessor {
    void processFile(File file);
  }

  private static void traverseTree(File file, FileProcessor fp) {
    fp.processFile(file);
    File[] children = file.listFiles();
    if (children != null) {
      for (File child : children) {
        traverseTree(child, fp);
      }
    }
  }
}
