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


(use-modules (ice-9 optargs)
	     (srfi srfi-1)
	     (srfi srfi-13)
	     (ice-9 rdelim)
	     (ice-9 pretty-print))

(define protos-in-one-string "

 bool libpd_init(bool use_gui, const char* libdir);
 void libpd_cleanup(void);
 void libpd_set_hook_data(void* data);
 void libpd_clear_search_path(void);
 void libpd_add_to_search_path(const char* sym);

 void* libpd_openfile(const char* basename, const char* dirname);
 void libpd_closefile(void* p);
 void libpd_savefile(void* p);
 void* libpd_request_savefile(void* x);
 bool libpd_wait_until_file_is_saved(void* request, float max_seconds_to_wait);

 int libpd_getdollarzero(void* p);

 void libpd_show_gui(void);
 void libpd_hide_gui(void);

 int libpd_blocksize(void);
 int libpd_init_audio(int inChans, int outChans, int sampleRate);
 int libpd_process_raw(const float* inBuffer, float* outBuffer);
 int libpd_process_float_noninterleaved(int ticks, const float** inBuffer, float** outBuffer);
 int libpd_process_short(const int ticks,
    const short* inBuffer, short* outBuffer);
 int libpd_process_float(int ticks,
    const float* inBuffer, float* outBuffer);
 int libpd_process_double(int ticks,
    const double* inBuffer, double* outBuffer);

 int libpd_arraysize(const char* name);

 int libpd_read_array(float* dest, const char* src, int offset, int n);
 int libpd_write_array(const char* dest, int offset, float* src, int n);

 int libpd_bang(const char* recv);
 int libpd_float(const char* recv, float x);
 int libpd_symbol(const char* recv, const char* sym);

 void libpd_set_float(t_atom* v, float x);
 void libpd_set_symbol(t_atom* v, const char* sym);
 int libpd_list(const char* recv, int argc, t_atom* argv);
 int libpd_message(const char* recv, const char* msg, int argc, t_atom* argv);

 int libpd_start_message(int max_length);
 void libpd_add_float(float x);
 void libpd_add_symbol(const char* sym);
 int libpd_finish_list(const char* recv);
 int libpd_finish_message(const char* recv, const char* msg);

 int libpd_exists(const char* sym);
 void* libpd_bind(const char* sym, void* data);
 void libpd_unbind(void* p);

 void libpd_set_printhook(t_libpd_printhook hook);
 void libpd_set_banghook(t_libpd_banghook hook);
 void libpd_set_floathook(t_libpd_floathook hook);
 void libpd_set_symbolhook(t_libpd_symbolhook hook);
 void libpd_set_listhook(t_libpd_listhook hook);
 void libpd_set_messagehook(t_libpd_messagehook hook);

 int libpd_noteon(int channel, int pitch, int velocity);
 int libpd_controlchange(int channel, int controller, int value);
 int libpd_programchange(int channel, int value);
 int libpd_pitchbend(int channel, int value);
 int libpd_aftertouch(int channel, int value);
 int libpd_polyaftertouch(int channel, int pitch, int value);
 int libpd_midibyte(int port, int byte);
 int libpd_sysex(int port, int byte);
 int libpd_sysrealtime(int port, int byte);

 void libpd_set_noteonhook(t_libpd_noteonhook hook);
 void libpd_set_controlchangehook(t_libpd_controlchangehook hook);
 void libpd_set_programchangehook(t_libpd_programchangehook hook);
 void libpd_set_pitchbendhook(t_libpd_pitchbendhook hook);
 void libpd_set_aftertouchhook(t_libpd_aftertouchhook hook);
 void libpd_set_polyaftertouchhook(t_libpd_polyaftertouchhook hook);
 void libpd_set_midibytehook(t_libpd_midibytehook hook);


")

(define extra-pointer-types '(
 t_libpd_printhook
 t_libpd_banghook
 t_libpd_floathook
 t_libpd_symbolhook
 t_libpd_listhook
 t_libpd_messagehook

 t_libpd_noteonhook
 t_libpd_controlchangehook
 t_libpd_programchangehook
 t_libpd_pitchbendhook
 t_libpd_aftertouchhook
 t_libpd_polyaftertouchhook
 t_libpd_midibytehook
))


(define (c-butlast l)
  (reverse (cdr (reverse l))))

(define (last l)
  (car (reverse l)))

(define <-> string-append)

(define (c-display . args)
  (if (null? args)
      (newline)
      (begin
        (display (car args))
        (display " ")
        (apply c-display (cdr args)))))

(define protos (c-butlast (string-split protos-in-one-string #\;)))


;; Code copied from snd-rt
(define (parse-c-proto funcdef cont)
  (let* ((temp (map string-trim-both (string-split funcdef #\()))
	 (retname (string-split (car temp) #\space))
	 (rettype (string-trim-both (apply <-> (map (lambda (x) (<-> x " ")) (c-butlast retname)))))
	 (name (last retname))
	 (args (let* ((temp1 (string-trim-right funcdef))
		      (temp2 (string-trim-both (substring temp1
							  (1+ (string-index temp1 #\())
							  (1- (string-length temp1))))))
		 (if (or (= (string-length temp2) 0)
			 (string=? temp2 "void"))
		     '()
		     (map (lambda (x)
			    (if (string-index x #\()
				(list "void*" (eval-c-get-unique-name))
				(let ((dassplit (map string-trim-both (string-split (string-trim-both x) #\space))))
				  (if (= 1 (length dassplit))
				      (list (car dassplit) (eval-c-get-unique-name))
				      (list (string-trim-both (apply <-> (map (lambda (x)
										(<-> x " "))
									      (c-butlast dassplit))))
					    (string-trim-both (last dassplit)))))))
			  (string-split temp2 #\,)))))
	 )
    (cont rettype name args)))

(define (get-stripped-name funcname)
  (substring funcname 6))

(define (get-funcdefname funcname)
  (<-> "Funcdef_for_" (get-stripped-name funcname)))
  
(define (create-funcdefprotos)
  (for-each (lambda (funcdef)
              (parse-c-proto funcdef
                             (lambda (rettype name args)
                               (display (<-> "typedef " rettype " (*" (get-funcdefname name) ") ("))
                               (if (null? args)
                                   (display "void")
                                   (for-each (lambda (arg n)
                                               (if (> n 0)
                                                   (display ", "))
                                               (display (car arg)))
                                             args
                                             (iota (length args))))
                               (c-display ");"))))
            
            protos))
(define (create-struct-_pd)
  (c-display "struct _pd {")
  (for-each (lambda (funcdef)
              (parse-c-proto funcdef
                             (lambda (rettype name args)
                               (c-display (<-> "  " (get-funcdefname name) " " name ";")))))
            protos)
  (newline)
  (c-display "  void *handle;")
  (c-display "  char *libfilename;")
  (c-display "};"))
  

(define (create-load-symbols-func)
  (c-display "static void load_symbols(struct _pd *pd) {")
  (for-each (lambda (funcdef)
              (parse-c-proto funcdef
                             (lambda (rettype name args)
                               (c-display (<-> "  pd->" name " = dlsym(pd->handle, \"" name "\");")))))
            protos)
  (c-display "}"))

(define (create-bridge-functions)
  (for-each (lambda (funcdef)
              (parse-c-proto funcdef
                             (lambda (rettype name args)
                               (display (<-> rettype " libpds_" (get-stripped-name name) "(pd_t *pd"))
                               (for-each (lambda (arg)
                                           (display (<-> ", " (car arg) " " (cadr arg))))
                                         args)
                               (display ") { ")
                               (if (not (string=? rettype "void"))
                                   (display "return "))
                               (display (<-> "pd->" name "("))
                               (for-each (lambda (arg n)
                                           (if (> n 0)
                                               (display ", "))
                                           (display (cadr arg)))
                                         args
                                         (iota (length args)))
                               (c-display "); }"))))
            protos))

(define (create-header-protos)
  (c-display "#ifndef __M_LIBPD_H__")
  (c-display "#define __M_LIBPD_H__")
  (c-display "")
  (c-display "#ifdef __cplusplus")
  (c-display "extern \"C\"")
  (c-display "{")
  (c-display "#endif")
  (c-display "")
  (c-display "#include <stdbool.h>")
  (c-display "")
  (c-display "#include \"m_pd.h\"")
  (c-display "#include \"z_libpd.h\"")
  (c-display "")
  (c-display "typedef struct _pd pd_t;")
  (newline)
  (c-display "pd_t *libpds_create(bool use_gui, const char* libdir);")
  (c-display "char *libpds_strerror(void);")
  (c-display "void libpds_delete(pd_t *pd);")

  (for-each (lambda (funcdef)
              (parse-c-proto funcdef
                             (lambda (rettype name args)
                               (if (and (not (string=? name "libpd_init"))
                                        (not (string=? name "libpd_cleanup")))
                                   (begin
                                     (display (<-> rettype " libpds_" (get-stripped-name name) "(pd_t *pd"))
                                     (for-each (lambda (arg)
                                                 (display (<-> ", " (car arg) " " (cadr arg))))
                                               args)
                                     (c-display ");"))))))
            protos)
  (newline)
  (c-display "#ifdef __cplusplus")
  (c-display "}")
  (c-display "#endif")
  (c-display "")
  (c-display "#endif"))



(define (main args)
  (define arg (if (null? (cdr args))
                  'gakk'
                  (string->symbol (cadr args))))
  (cond ((eq? arg 'c-file)
         (create-funcdefprotos)
         (newline)
         (create-struct-_pd)
         (newline)
         (create-load-symbols-func)
         (newline)
         (create-bridge-functions))
        ((eq? arg 'h-file)
         (create-header-protos))
        (else
         (c-display "Error. Unknown args. Ether c-file or h-file"))))



