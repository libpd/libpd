#!/bin/sh
# -*- scheme -*-
exec guile -e main -s $0 $*
!#


'(
/* Copyright 2013 Kjetil S. Matheussen

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. */
)

(use-modules (ice-9 rdelim))
(use-modules (ice-9 ftw))

(define (to-string something)
  (cond ((number? something)
         (number->string something))
        ((symbol? something)
         (symbol->string something))
        ((char? something)
         (string something))
        (else
         something)))

(define (to-symbol something)
  (cond ((symbol? something) something)
        (else
         (string->symbol (to-string something)))))
#!
(map to-string '(50 50.2 gakkgakk "gakk"))
!#

(define (string-endswith string ends)
  (let ((l1 (string-length string))
        (l2 (string-length ends))) 
    (and (>= l1 l2)
         (string=? (substring string (- l1 l2)) ends))))
#!
(string-endswith "asdf" ".c")
(string-endswith "b" ".eec")
(string-endswith "basdf.c" ".c")
!#

(define (<-> . args)
  (apply string-append (map to-string args)))

(define (store . elements)
  (lambda (key . default)
    (cond ((memv key elements)
           (cadr (memv key elements)))
          ((null? default)
           (c-display key "not found"))
          (else
           (car default)))))

#!
((store #:gakk "asdf" #:hello 2 #:aiai '()) #:hello3 "")
!#

;; Copied from ftw.scm
(define (directory-files dir)
  (let ((dir-stream (opendir dir)))
    (let loop ((new (readdir dir-stream))
               (acc '()))
      (if (eof-object? new)
	  (begin
	    (closedir dir-stream)
	    acc)
          (loop (readdir dir-stream)
                (if (or (string=? "."  new)             ;;; ignore
                        (string=? ".." new))            ;;; ignore
                    acc
                    (cons new acc)))))))
#!
(directory-files "/tmp/")
!#


;; config start

(define CC "gcc")
(define CFLAGS "-Wall -g -DPD -DVERSION=\\\"libpd-extended-0.43.3\\\" -Ipure-data/src/ -DLIBPD -fPIC") ;; TODO: Add -O3

(define objsdir "externalobjs")

(define packages '())

(define (add-package package)
  (set! packages (cons package packages)))


(define sources-without-setups '(libchaos.c eblosc~.c pvocfreq.c lpc.c tables.c filters.c OSC-pattern-match.c OSC-client.c OSC-system-dependent.c OSC-timetag.c htmsocket.c iemnet_receiver.c iemnet_sender.c iemnet_data.c))


(add-package    (store #:path "externals/vanilla/"
                       #:sources '(adc~.c bang~.c bng.c cputime.c dac~.c delay.c fft~.c framp~.c ifft~.c key.c keyname.c keyup.c line.c list.c loadbang.c metro.c namecanvas.c netreceive.c netsend.c openpanel.c pipe.c print.c print~.c qlist.c random.c realtime.c rfft~.c rifft~.c savepanel.c textfile.c tgl.c timer.c vradio.c vsl.c vu.c midiin.c sysexin.c notein.c ctlin.c pgmin.c bendin.c touchin.c polytouchin.c midiclkin.c midirealtimein.c midiout.c noteout.c ctlout.c pgmout.c bendout.c touchout.c polytouchout.c makenote.c stripnote.c poly.c bag.c hradio.c hsl.c  cnv.c nbx.c)
  
                       #:links '((delay.c del.c)
                                 (hradio.c radiobut.c)
                                 (hradio.c radiobutton.c)
                                 (vsl.c vslider.c)
                                 (vradio.c vdl.c)
                                 (hradio.c hdl.c)
                                 (hsl.c hslider.c)
                                 (cnv.c my_canvas.c)
                                 (nbx.c my_numbox.c)
                                 (tgl.c toggle.c)
                                 (hradio rdb.c)
                                 )))
(add-package    (store #:path "externals/zexy/src/"
                    #:sources '(zexy.c z_zexy.c 
                                0x260x260x7e.c 0x2e.c 0x3c0x7e.c 0x3d0x3d0x7e.c 0x3e0x7e.c 0x7c0x7c0x7e.c 
                                a2l.c abs~.c absgn~.c atoi.c avg~.c  
                                blockmirror~.c blockshuffle~.c blockswap~.c 
                                date.c demultiplex~.c demultiplex.c dfreq~.c dirac~.c drip.c envrms~.c 
                                fifop.c freadln.c fwriteln.c glue.c index.c length.c lifop.c limiter~.c 
                                list2int.c list2lists.c list2symbol.c lister.c listfind.c liststorage.c lpt.c 
                                makesymbol.c matchbox.c mavg.c minmax.c msgfile.c multiline~.c multiplex~.c 
                                multiplex.c multireceive.c niagara.c noish~.c noisi~.c operating_system.c pack~.c 
                                pack.c packel.c pdf~.c prime.c quantize~.c rawprint.c regex.c relay.c repack.c 
                                repeat.c route~.c sfplay.c sfrecord.c sgn~.c sigzero~.c sleepgrain.c sort.c 
                                step~.c strcmp.c sum.c swap~.c symbol2list.c tabdump.c tabminmax.c tabread4~~.c 
                                tabset.c tavg~.c time.c unpack~.c unpack.c urn.c z~.c)
                    #:links '((multiplex~.c mux~.c))
                    #:cflags "-Dverbose=verbose_zexy -Dmux_tilde_tilde_setup=mux_tilde_setup"))

;; Removed from zexy:
;; * wrap.c (name clash with iem external, simpler to remove this)


(add-package    (store #:path "externals/maxlib/"
                    #:sources '(
allow.c
arbran.c
arraycopy.c
average.c
beat.c
beta.c
bilex.c
borax.c
cauchy.c
chord.c
delta.c
deny.c
dist.c
divide.c
divmod.c
edge.c
expo.c
fifo.c
gauss.c
gestalt.c
history.c
ignore.c
iso.c
lifo.c
limit.c
linear.c
listfifo.c
listfunnel.c
match.c
minus.c
mlife.c
multi.c
nchange.c
netclient.c
netdist.c
netrec.c
netserver.c
nroute.c
pitch.c
plus.c
poisson.c
pong.c
pulse.c
remote.c
rewrap.c
rhythm.c
scale.c
score.c
step.c
subst.c
sync.c
temperature.c
tilt.c
timebang.c
triang.c
unroute.c
velocity.c
weibull.c
)
                    #:links '((multiplex~.c mux~.c))
                    #:cflags ""))

;; Removed from maxlib:
;; * speedlim.c, split.c  (name clash with iem)
;; * wrap.c, urn.c (name clash with zexy)
;; * maxlib.c (lot of work to make all links)

(add-package    (store #:path "externals/bsaylor/"
                       #:sources '(aenv~.c partconv~.c pvoc~.c susloop~.c svf~.c zhzxh~.c)))

#!
(for-each (lambda (single-package)
            (add-package (store #:path (<-> "externals/" single-package "/")
                                #:sources (list (<-> single-package ".c")))))
          (map to-string '(freeverb~ arraysize bassemu~)))
!#

;; compile all c-files in directory.
(for-each (lambda (externals-dir)
            (let* ((links '((cmath~.c clog~.c) ;; ((target link) ...)
                            (cmath~.c cexp~.c)
                            (mean~.c cxmean.c)
                            (mean~.c cxstddev.c)
                            (mean~.c cxavgdev.c)
                            (pvocfreq.c shuffle.c)
                            (anything.c any.c)
                            (forpp.c for_pp.c)
                            (init.c ii.c)
                            (iem_prepend.c pp.c)
                            (toggle_mess.c tm.c)
                            (unsymbol.c unsym.c)
                            ))
                   (dontcompile (append '(abs~.c path.c speexin~.c streamin~.c mp3amp~.c mp3cast~.c mp3fileout~.c mp3streamin~.c mp3streamout~.c mp3write~.c test_OSC_timeTag.c test_OSC.c)
                                        (map cadr links)))
                   (c-files (filter (lambda (filename)
                                      (and (string-endswith filename ".c")
                                           (not (memq (string->symbol filename) dontcompile))))
                                    (directory-files externals-dir))))
              (add-package (store #:path externals-dir
                                  #:sources c-files
                                  #:cflags "-Iexternals/unauthorized/vocoder~"
                                  #:links links))))
          (map (lambda (dir)
                 (<-> "externals/" dir "/"))
               '(arraysize bassemu~ chaos creb/modules cxc earplug~ ekext ext13 flatgui freeverb~
                           ggee/control ggee/experimental ggee/filters ggee/signal
                           grh/adaptive/src hcs 
                           iem/comport/bird/ iem/comport/comport iem/iem_adaptfilt/src iem/iem_ambi/src iem/iem_bin_ambi/src iem/iem_delay/src iem/iem_roomsim/src iem/iem_spec2/src iem/iem_tab/src iem/iemguts/src iem/syslog iem/iemgui/src
                           iem16/src iemlib/iemlib1/src iemlib/iemlib2/src iemlib/iem_mp3/src iemlib/iem_t3_lib/src   
                           log mjlib moocow moonlib motex
                           mrpeach/binfile mrpeach/cmos mrpeach/flist2tab mrpeach/life2x mrpeach/midifile mrpeach/osc mrpeach/runningmean mrpeach/slipdec
                           mrpeach/slipenc mrpeach/str mrpeach/tab2flist mrpeach/tabfind mrpeach/which mrpeach/xbee
                           pan pddp pdogg pmpd sigpack smlib tof/src unauthorized windowing
                           oscx iem/iemnet
                           )))


;; * Not compiled:
;;   * Does not use static (or "inline static" if in h-files):
;;     * boids 
;;     * ggee/gui
;;     * iem/mediasettings
;;     * vbap
;;   * c++:
;;     * creb/modules++
;;     * grh/PDContainer
;;     * iem/iemxmlrpc
;;   * Too complicated:
;;     * Gem
;;     * hcs/usbhid
;;     * hid
;;     * loaders/tclpd (also requires c99, which is perfectly fine, but requires some work to enable up just for this one)
;;     * loaders/pdlua
;;     * loaders/urloader
;;     * miXed
;;     * pdp
;;     * unauthorized/{speexin~.c streamin~.c mp3amp~.c mp3cast~.c mp3fileout~.c mp3streamin~.c mp3streamout~.c mp3write~.c}
;;   * Not sure if necessary:
;;     * externals/loaders/hexloader
;;   * Requires Gem:
;;     * gem2pdp
;;   * Name-clash with other package:
;;     * ggee/other (messages is also in ext13/)
;;     * markex
;;     * unauthorized/path
;;     *  mrpeach/net
;;   * Doesnt compile right away (probably quite simple to fix):
;;     * iem/iemmatrix/src
;;   * No statics, lots of compilation warnings:
;;     * jasch_lib/detox jasch_lib/memchr jasch_lib/strchr jasch_lib/strcut jasch_lib/strlen jasch_lib/strtok jasch_lib/underscore
#!
(compile)
!#

;; extra objects
(for-each (lambda (externals-dir)
            (let* ((links '())
                   (dontcompile (append '()
                                        (map cadr links)))
                   (c-files (filter (lambda (filename)
                                      (and (string-endswith filename ".c")
                                           (not (memq (string->symbol filename) dontcompile))))
                                    (directory-files externals-dir))))
              (add-package (store #:path externals-dir
                                  #:sources c-files
                                  #:cflags ""
                                  #:links links))))
          (map (lambda (dir)
                 (<-> "pure-data/extra/" dir "/"))
               '(bonk~ choice fiddle~ loop~ lrshift~ pique sigmund~ stdout)))

;; Removed from extra:
;; pd~ (too complicated)

;; config finished


(define (c-display . args)
  (if (null? args)
      (newline)
      (begin
        (display (car args))
        (display " ")
        (apply c-display (cdr args)))))

(define (get-system-output command)
  (let ((logfilename "/tmp/snd-ls-logtemp"))
    (system (<-> command " > " logfilename))
    (let* ((ret "")
	   (fd (open-file logfilename "r"))
	   (line (read-line fd)))
      (while (not (eof-object? line))
	     (set! ret (<-> ret line))
	     (set! line (read-line fd)))
      (close fd)
      (system (<-> "rm " logfilename))
      ret)))

#!
(get-system-output "echo gakk")
!#


(define (my-system-0 command commands)
  (cond ((null? commands)
         (c-display "Executing command -" command "-")
         (system command))
        ((string=? "" command)
         (my-system-0 (to-string (car commands)) (cdr commands)))
        (else
         (my-system-0 (<-> command " " (to-string (car commands))) (cdr commands)))))

(define (my-system . commands)
  (my-system-0 "" commands))

#!
(my-system 'echo 'gakkgakk)
(= 0 (my-system "gcc externals/bsaylor/sse-conv.inc.c"))
!#

(define (nth n l . valueifnot)
  (if (null? valueifnot)
      (list-ref l n)
      (if (> (length l) n)
          (list-ref l n)
          (car valueifnot))))

#!
(nth 0 '(a b c))
(nth 1 '(a b c))
(nth 1 '(a b c) 'd)
(nth 2 '(a b c) 'd)
(nth 3 '(a b c) 'd)
(nth 4 '(a b c) 'd)
!#

;;

(define packages-sources (map to-string (filter (lambda (source)
                                                  (not (memq (to-symbol source) sources-without-setups)))
                                                (apply append (map (lambda (package) (package #:sources)) packages)))))
(define packages-links (apply append (map (lambda (package) (package #:links '())) packages)))

(define (get-package-from-source source)
  (let loop ((packages packages))
    (let ((package (car packages)))
      (if (memq (to-symbol source) (map to-symbol (package #:sources)))
          package
          (loop (cdr packages))))))

(define package-paths (map (lambda (package-source)
                             (let ((package (get-package-from-source package-source)))
                               (package #:path)))
                           packages-sources))

(define (file-exists path)
  (access? path R_OK))

#!
(file-exists "/tmp/radium_T29691.pd")
(file-exists "/tmp/asdf")
(begin packages-sources)
(apply append (map (lambda (package) (package #:sources)) packages))
!#

(define (file-writetime path)
  (stat:mtime (stat path)))

(define (needs-compilation source dest)
  (or (not (file-exists dest))
      (> (file-writetime source)
         (file-writetime dest))))
#!
(file-writetime "/tmp/radium_T29691.pd")
(file-writetime "zexy/src/atoi.c")
(file-writetime "/home/ksvalast/libpd/externalobjs/atoi.c.o")
(needs-compilation "externals/zexy/src/atoi.c" "/home/ksvalast/libpd/externalobjs/atoi.c.o")
!#

(define (compile-source path flags source)
  (let ((fullsourcepath (<-> path source))
        (fulldestpath (<-> objsdir "/" source ".o")))    
    (if (needs-compilation fullsourcepath fulldestpath)
        (my-system CC CFLAGS flags (<-> "-Dsetup=setup_libpd_" (get-symname (to-string source)))  "-c" fullsourcepath "-o" fulldestpath)
        0)))
        

(define (compile-all-sources path flags sources)
  (if (null? sources)
      #t
      (and (= 0 (compile-source path flags (car sources)))
           (compile-all-sources path flags (cdr sources)))))

#!
(compile-all-sources (vanilla #:path) "" (vanilla #:sources))
(compile-all-sources (zexy #:path) "" (zexy #:sources))
!#

(define (compile)
  (let loop ((packages packages))
    (if (null? packages)
        #t
        (let ((package (car packages)))
          (if (compile-all-sources (package #:path) (package #:cflags "") (package #:sources))
              (loop (cdr packages))
              #f)))))
#!
(compile)
!#

;; "gakk.c" -> "gakk"
;; "gakk" -> "gakk
(define (get-base-filename source)
  (if (not (string-rindex source #\.))
      source
      (substring source 0 (string-rindex source #\.))))

#!
(get-base-filename "asdf.c")
(get-base-filename "asdf.caeees")
(get-base-filename "asdhhh")
(get-base-filename "a")
!#

(define (print-object-files)
  (for-each (lambda (package)
              (for-each (lambda (source)
                          (display (<-> objsdir "/" source ".o ")))
                        (package #:sources)))
            packages)
  (newline))

(define (name-ends-with-tilde? source)
  (char=? #\~ (car (reverse (string->list source)))))

#!
(name-ends-with-tilde? "asdf~")
(name-ends-with-tilde? "asdf~we")
(name-ends-with-tilde? "~we")
!#

(define (name-ends-with-0x7e? name)
  (let ((len (string-length name)))
    (and (>= len (string-length "0x7e"))
         (string=? (substring name (- len 4) len) "0x7e"))))

#!
(name-ends-with-0x7e? "asdf")
(name-ends-with-0x7e? "0x7e")
(name-ends-with-0x7e? "0x7eb")
(name-ends-with-0x7e? "10x7e")

(substring  "10x7e" (- 5 4) 5)
!#

(define (get-symname source)
  (apply <-> (map (lambda (char)
                    (if (member char (string->list "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWZYZ0123456789_"))
                        char
                        (let ((n (char->integer char)))
                          (if (< n 16)
                              (<-> "0x0" (number->string n 16))
                              (<-> "0x"  (number->string n 16))))))
                  (string->list source))))
#!
(get-symname "gakk")
(get-symname "gakk~")
(get-symname ">")
(get-symname ">~")
(char->integer #\.)
(get-setup-funcname "0x260x260x7e.c")
!#

(define (get-setup-funcname source)
  (let ((name (get-base-filename source)))
    (let ((name (cond ((name-ends-with-tilde? name)
                       (<-> (string-drop-right name 1) "_tilde"))
                      (else
                       name))))
      (let ((converted-name (get-symname name)))
        (if (and (string=? name converted-name)
                 (member (car (string->list name)) (string->list "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWZYZ_")))
            (<-> name "_setup")
            (<-> "setup_" converted-name))))))

(define (get-matchname source)
  (let ((name (get-base-filename source)))
    (let ((name (cond ((name-ends-with-tilde? name)
                       (<-> (string-drop-right name 1) "_tilde"))
                      ((name-ends-with-0x7e? name)
                       (<-> (string-drop-right name 4) "_tilde"))
                      (else
                       name))))
      (let ((converted-name (get-symname name)))
        (if (and (string=? name converted-name)
                 (member (car (string->list name)) (string->list "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWZYZ_")))
            (<-> name "_setup")
            (<-> "setup_" converted-name))))))

#!
(get-setup-funcname "gakk.c")
(get-setup-funcname "gakk~.c")
(get-setup-funcname ">.c")
(get-setup-funcname ">~.c")
(get-setup-funcname "0x260x260x7e.c")
(get-setup-funcname "&&~.c")
(string=? (get-matchname "0x260x260x7e.c")
          (get-matchname "&&~.c"))
(get-setup-funcname "adc~.c")
!#

(define (gen-loader-file)
  (define setup-funcnames (map get-setup-funcname packages-sources))
  (define base-filenames (map get-base-filename packages-sources))

  (c-display "#include <stdlib.h>")
  (c-display "#include <stdio.h>")
  (c-display "#include <stdbool.h>")
  (c-display "#include <string.h>")
  (c-display "#include <m_pd.h>")
  (c-display "#include <s_stuff.h>")

  ;; protos
  (for-each (lambda (setup-funcname)
              (c-display "void" setup-funcname "(void);"))
            setup-funcnames)

  ;; exp protos
  (for-each (lambda (manual_setup_name)
              (c-display (<-> " void " manual_setup_name "_setup(void);")))
            '(expr expr_tilde fexpr_tilde))

  (c-display "static void my_class_set_extern_dir(char *path){")
  (c-display " char temp[4096];")
  (c-display " sprintf(temp,\"%s/../%s\",sys_libdir->s_name,path);")
  (c-display " class_set_extern_dir(gensym(temp));")
  (c-display "}")

  ;; loader func
  (c-display "int libpd_load_lib(char *classname){printf(\"Trying to load \\\"%s\\\"\\n\",classname);")

  ;; expr
  (for-each (lambda (manual_setup_name)
              (c-display (<-> " if(!strcmp(classname, \"" manual_setup_name "_setup\")){" manual_setup_name "_setup();return 1;}")))
            '(expr expr_tilde fexpr_tilde))

  (for-each (lambda (source setup-funcname base-filename path)
              (let* ((matchers (map get-base-filename (map to-string (map cadr (filter (lambda (f)
                                                                                         (string=? (to-string (car f)) source))
                                                                                       packages-links)))))
                     (strcmps (map (lambda (matcher)
                                     (<-> "!strcmp(classname, \"" (get-matchname matcher) "\")"))
                                   matchers)))
                (display (<-> " if(!strcmp(classname, \"" (get-matchname (to-string source)) "\")"))
                (for-each (lambda (strcmp)
                            (display (<-> "||" strcmp)))
                          strcmps)
                (c-display (<-> "){my_class_set_extern_dir(\"" path "\");" setup-funcname "();return 1;}"))
                ))
            packages-sources
            setup-funcnames
            base-filenames
            package-paths)

  (c-display " post(\"\\\"%s\\\" not found in path when libpds_create was called.\\n\", classname);")
  (c-display " return 0;")
  (c-display "}")
  )

#!
(gen-loader-file)
!#

(define (main args)
  (define arg (if (null? (cdr args))
                  'gakk'
                  (string->symbol (cadr args))))
  (cond ((eq? arg 'compile)
         (if (not (compile))
             (exit -1)))
        ((eq? arg 'print-object-files)
         (print-object-files))
        ((eq? arg 'gen-loader-file)
         (gen-loader-file))
        (else
         (c-display "Error. Unknown args. Ether compile, print-object-files or gen-loader-file")
         (exit -1))))
