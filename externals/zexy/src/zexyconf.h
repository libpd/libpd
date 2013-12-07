#ifndef _ZEXYCONF_H_
#define _ZEXYCONF_H_
/* zexyconf.h: configuration for zexy:
 *		this is only included if no config.h is created
 *		adapt this to your own needs
 */

/* Define if you have the <regex.h> header file.  */
/*
#undef HAVE_REGEX_H 
*/

/* Define if you have the <alloca.h> header file.  */
/*
#undef HAVE_ALLOCA_H
*/

/* define if you want parallelport-support (direct access to the port address) */
/*
#undef Z_WANT_LPT
*/

/* define if you have the <linux/ppdev.h> header file.
 * (for parport _device_ support) 
 * you need Z_WANT_LPT for this to have an effect ! 
 */
/*
#undef HAVE_LINUX_PPDEV_H
*/

#endif /* _ZEXYCONF_H_ */

