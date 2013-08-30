/* scandir: Scan a directory, collecting all (selected) items into a an array.
 *
 * This code borrowed from 'libit', which can be found here:
 *
 * http://www.iro.umontreal.ca/~pinard/libit/dist/scandir/
 *
 * The original author put this code in the public domain.
 * It has been modified slightly to get rid of warnings, etc.
 *
 * Below is the email I received from pinard@iro.umontreal.ca (Fran�ois Pinard)
 * when I sent him an email asking him about the license, etc. of this
 * code which I obtained from his site.
 *
 * I think the correct spelling of his name is Rich Salz.  I think he's now 
 * rsalz@datapower.com...
 * -- 
 * Rich Salz, Chief Security Architect
 * DataPower Technology                           http://www.datapower.com
 * XS40 XML Security Gateway   http://www.datapower.com/products/xs40.html
 *
 *	Copyright(C):	none (public domain)
 *	License:	none (public domain)
 *	Author:		Rich Salz <rsalz@datapower.com>
 *
 *
 *
 *	-- Alan Robertson
 *	   alanr@unix.sh
 *
 **************************************************************************
 *
 * Subject:	Re: Scandir replacement function
 * Date:	18 May 2001 12:00:48 -0400
 * From:	pinard@iro.umontreal.ca (Fran�ois Pinard)
 * To:		Alan Robertson <alanr@unix.sh>
 * References:	1
 *
 *
 * [Alan Robertson]
 *
 * > Hi, I'd like to use your scandir replacement function found here:
 * > http://www.iro.umontreal.ca/~pinard/libit/dist/scandir/ But, it does
 * > not indicate authorship or licensing terms in it.  Could you tell me
 * > who wrote this code, under what license you distribute it, and whether
 * > and under what terms I may further distribute it?
 *
 * Hello, Alan.  These are (somewhat) explained in UNSHAR.HDR found in the
 * same directory.  The routines have been written by Rick Saltz (I'm not
 * completely sure of the spelling) a long while ago.  I think that nowadays,
 * Rick is better known as the main author of the nice INN package.
 *
 **************************************************************************
 *
 * I spent a little time verifying this with Rick Salz.
 * The results are below:
 *
 **************************************************************************
 *
 * Date: Tue, 20 Sep 2005 21:52:09 -0400 (EDT)
 * From: Rich Salz <rsalz@datapower.com>
 * To: Alan Robertson <alanr@unix.sh>
 * Subject: Re: Verifying permissions/licenses/etc on some old code of yours -
 *  scandir.c
 * In-Reply-To: <433071CA.8000107@unix.sh>
 * Message-ID: <Pine.LNX.4.44L0.0509202151270.9198-100000@smtp.datapower.com>
 * Content-Type: TEXT/PLAIN; charset=US-ASCII
 *
 * yes, it's most definitely in the public domain.
 *
 * I'm glad you find it useful.  I'm surprised it hasn't been replaced by,
 * e.g,. something in GLibC.  Ii'm impressed you tracked me down.
 *
 *	/r$
 *
 * -- 
 * Rich Salz                  Chief Security Architect
 * DataPower Technology       http://www.datapower.com
 * XS40 XML Security Gateway  http://www.datapower.com/products/xs40.html
 * ---------------------------------------------------------------------->
 * Subject:	scandir, ftw REDUX
 * Date: 	1 Jan 88 00:47:01 GMT
 * From: 	rsalz@pebbles.bbn.com
 * Newsgroups:  comp.sources.misc
 *
 *
 * Forget my previous message -- I just decided for completeness's sake to
 * implement the SysV ftw(3) routine, too.  
 *
 * To repeat, these are public-domain implementations of the SystemV ftw()
 * routine, the BSD scandir() and alphasort() routines, and documentation for
 * same.  The FTW manpage could be more readable, but so it goes.
 *
 * Anyhow, feel free to post these, and incorporate them into your existing
 * packages.  I have readdir() routiens for MSDOS and the Amiga if anyone
 *  wants them, and should have them for VMS by the end of January; let me
 *  know if you want copies.
 *
 *                        Yours in filesystems,
 *                                /r$
 *
 *                                Anyhow, feel free to post
 * ----------------------------------------------------------------------<
 *
 */

#include "scandir.h"
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#ifndef NULL
# define NULL ((void *) 0)
#endif

/* Initial guess at directory allocated.  */
#define INITIAL_ALLOCATION 20

int scandir (const char *directory_name,
             struct dirent ***array_pointer,
             int (*select_function) (const struct dirent *),
             /* This is what the linux man page says */
             int (*compare_function) (const struct dirent**, const struct dirent**)
	)
{
  DIR *directory;
  struct dirent **array;
  struct dirent *entry;
  struct dirent *copy;
  int allocated = INITIAL_ALLOCATION;
  int counter = 0;

  /* Get initial list space and open directory.  */

  if (directory = opendir (directory_name), directory == NULL)
    return -1;

  if (array = (struct dirent **) malloc (allocated * sizeof (struct dirent *)),
      array == NULL)
    return -1;

  /* Read entries in the directory.  */

  while (entry = readdir (directory), entry)
    if (select_function == NULL || (*select_function) (entry))
      {
	/* User wants them all, or he wants this one.  Copy the entry.  */

	/*
	 * On some OSes the declaration of "entry->d_name" is a minimal-length
	 * placeholder.  Example: Solaris:
	 * 	/usr/include/sys/dirent.h:
	 *		"char d_name[1];"
	 *	man page "dirent(3)":
	 *		The field d_name is the beginning of the character array
	 *		giving the name of the directory entry. This name is
	 *		null terminated and may have at most MAXNAMLEN chars.
	 * So our malloc length may need to be increased accordingly.
	 *	sizeof(entry->d_name): space (possibly minimal) in struct.
	 * 	strlen(entry->d_name): actual length of the entry. 
	 *
	 *			John Kavadias <john_kavadias@hotmail.com>
	 *			David Lee <t.d.lee@durham.ac.uk>
	 */
	int namelength = strlen(entry->d_name) + 1;	/* length with NULL */
	int extra = 0;

	if (sizeof(entry->d_name) <= namelength) {
		/* allocated space <= required space */
		extra += namelength - sizeof(entry->d_name);
	}

	if (copy = (struct dirent *) malloc (sizeof (struct dirent) + extra),
	    copy == NULL)
	  {
	    closedir (directory);
	    free (array);
	    return -1;
	  }
	copy->d_ino = entry->d_ino;
	copy->d_reclen = entry->d_reclen;
	strcpy (copy->d_name, entry->d_name);

	/* Save the copy.  */

	if (counter + 1 == allocated)
	  {
	    allocated <<= 1;
	    array = (struct dirent **)
	      realloc ((char *) array, allocated * sizeof (struct dirent *));
	    if (array == NULL)
	      {
		closedir (directory);
		free (array);
		free (copy);
		return -1;
	      }
	  }
	array[counter++] = copy;
      }

  /* Close things off.  */

  array[counter] = NULL;
  *array_pointer = array;
  closedir (directory);

  /* Sort?  */

  if (counter > 1 && compare_function)
    qsort ((char *) array, counter, sizeof (struct dirent *)
  	,	(int (*)(const void *, const void *))(compare_function));

  return counter;
}

int alphasort(const void *dirent1, const void *dirent2)
{
  return(strcmp((*(const struct dirent **)dirent1)->d_name,
                (*(const struct dirent **)dirent2)->d_name));
}
