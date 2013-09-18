/* (C) Guenter Geiger <geiger@epy.co.at> */

/* this doesn't run on Windows (yet?) */
#ifndef _WIN32

#include <m_pd.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sched.h>

void sys_rmpollfn(int fd);
void sys_addpollfn(int fd, void* fn, void *ptr);

/* ------------------------ shell ----------------------------- */

#define INBUFSIZE 1024

static t_class *shell_class;


static void drop_priority(void) 
{
#if (_POSIX_PRIORITY_SCHEDULING - 0) >=  200112L
    struct sched_param par;
    int p1 ,p2, p3;
    par.sched_priority = 0;
    sched_setscheduler(0,SCHED_OTHER,&par);
#endif
}


typedef struct _shell
{
     t_object x_obj;
     int      x_echo;
     char *sr_inbuf;
     int sr_inhead;
     int sr_intail;
     void* x_binbuf;
     int fdpipe[2];
     int fdinpipe[2];
     int pid;
     int x_del;
     t_outlet* x_done;
     t_clock* x_clock;
} t_shell;

static int shell_pid;


void shell_cleanup(t_shell* x)
{
     sys_rmpollfn(x->fdpipe[0]);

     if (x->fdpipe[0]>0) close(x->fdpipe[0]);
     if (x->fdpipe[1]>0) close(x->fdpipe[1]);
     if (x->fdinpipe[0]>0) close(x->fdinpipe[0]);
     if (x->fdinpipe[1]>0) close(x->fdinpipe[1]);

     x->fdpipe[0] = -1;
     x->fdpipe[1] = -1;
     x->fdinpipe[0] = -1;
     x->fdinpipe[1] = -1;
     clock_unset(x->x_clock);
}

void shell_check(t_shell* x)
{
	int ret;
	int status;
	ret = waitpid(x->pid,&status,WNOHANG);
	if (ret == x->pid) {
	     shell_cleanup(x);
	     if (WIFEXITED(status)) {
		  outlet_float(x->x_done,WEXITSTATUS(status));
	     }
	     else outlet_float(x->x_done,0);
	}
	else {
	     if (x->x_del < 100) x->x_del+=2; /* increment poll times */
	     clock_delay(x->x_clock,x->x_del);
	}
}


void shell_bang(t_shell *x)
{
     post("bang");
}

/* snippet from pd's code */
static void shell_doit(void *z, t_binbuf *b)
{
    t_shell *x = (t_shell *)z;
    int msg, natom = binbuf_getnatom(b);
    t_atom *at = binbuf_getvec(b);

    for (msg = 0; msg < natom;)
    {
    	int emsg;
	for (emsg = msg; emsg < natom && at[emsg].a_type != A_COMMA
	    && at[emsg].a_type != A_SEMI; emsg++)
	    	;
	if (emsg > msg)
	{
	    int i;
	    for (i = msg; i < emsg; i++)
	    	if (at[i].a_type == A_DOLLAR || at[i].a_type == A_DOLLSYM)
	    {
	    	pd_error(x, "netreceive: got dollar sign in message");
		goto nodice;
	    }
	    if (at[msg].a_type == A_FLOAT)
	    {
	    	if (emsg > msg + 1)
		    outlet_list(x->x_obj.ob_outlet,  0, emsg-msg, at + msg);
		else outlet_float(x->x_obj.ob_outlet,  at[msg].a_w.w_float);
	    }
	    else if (at[msg].a_type == A_SYMBOL)
	    	outlet_anything(x->x_obj.ob_outlet,  at[msg].a_w.w_symbol,
		    emsg-msg-1, at + msg + 1);
	}
    nodice:
    	msg = emsg + 1;
    }
}


void shell_read(t_shell *x, int fd)
{
     char buf[INBUFSIZE];
     t_binbuf* bbuf = binbuf_new();
     int i;
     int readto =
	  (x->sr_inhead >= x->sr_intail ? INBUFSIZE : x->sr_intail-1);
     int ret;

     ret = read(fd, buf,INBUFSIZE-1);
     buf[ret] = '\0';

     for (i=0;i<ret;i++)
       if (buf[i] == '\n') buf[i] = ';';
     if (ret < 0)
       {
	 error("shell: pipe read error");
	 sys_rmpollfn(fd);
	 x->fdpipe[0] = -1;
	 close(fd);
	 return;
       }
     else if (ret == 0)
       {
	 post("EOF on socket %d\n", fd);
	 sys_rmpollfn(fd);
	 x->fdpipe[0] = -1;
	 close(fd);
	 return;
       }
     else
       {
	 int natom;
	 t_atom *at;
	 binbuf_text(bbuf, buf, strlen(buf));
	 
	 natom = binbuf_getnatom(bbuf);
	 at = binbuf_getvec(bbuf);
	 shell_doit(x,bbuf);
       }
     binbuf_free(bbuf);
}


static void shell_send(t_shell *x, t_symbol *s,int ac, t_atom *at)
{
     int i;
     char tmp[MAXPDSTRING];
     int size = 0;

     if (x->fdinpipe[0] == -1) return; /* nothing to send to */

     for (i=0;i<ac;i++) {
	  atom_string(at,tmp+size,MAXPDSTRING - size);
	  at++;
	  size=strlen(tmp);
	  tmp[size++] = ' ';	  
     }
     tmp[size-1] = '\0';
     post("sending %s",tmp); 
     write(x->fdinpipe[0],tmp,strlen(tmp));
}

static void shell_anything(t_shell *x, t_symbol *s, int ac, t_atom *at)
{
     int i;
     char* argv[255];
     t_symbol* sym;

     if (!strcmp(s->s_name,"send")) {
	  post("send");
	  shell_send(x,s,ac,at);
	  return;
     }

     argv[0] = s->s_name;

     if (x->fdpipe[0] != -1) { 
	  post("shell: old process still running");
	  kill(x->pid,SIGKILL);
	  shell_cleanup(x);
     }


     if (pipe(x->fdpipe) < 0) {
	  error("unable to create pipe");
	  return;
     }

     if (pipe(x->fdinpipe) < 0) {
	  error("unable to create input pipe");
	  return;
     }


     sys_addpollfn(x->fdpipe[0],shell_read,x);

     if (!(x->pid = fork())) {
         /* reassign stdout */
         dup2(x->fdpipe[1],1);
         dup2(x->fdinpipe[1],0);
         
         /* drop privileges */
         drop_priority();
         seteuid(getuid());          /* lose setuid priveliges */

#ifdef __APPLE__
	  for (i=1;i<=ac;i++) {
	       argv[i] = getbytes(255);
	       atom_string(at,argv[i],255);
	       at++;
	  }
	  argv[i] = 0;
	  execvp(s->s_name,argv);
#else
	  char* cmd = getbytes(1024);
	  char* tcmd = getbytes(1024);
	  strcpy(cmd,s->s_name);
      for (i=1;i<=ac;i++) {
	       atom_string(at,tcmd,255);
	       strcat(cmd," ");
               strcat(cmd,tcmd);
               at++;
	  }
	  verbose(4,"executing %s",cmd);
	  system(cmd);
#endif /* __APPLE__ */
	  exit(0);
     }
     x->x_del = 4;
     clock_delay(x->x_clock,x->x_del);

     if (x->x_echo)
	  outlet_anything(x->x_obj.ob_outlet, s, ac, at); 
}



void shell_free(t_shell* x)
{
    binbuf_free(x->x_binbuf);
}

static void *shell_new(void)
{
    t_shell *x = (t_shell *)pd_new(shell_class);

    x->x_echo = 0;
    x->fdpipe[0] = -1;
    x->fdpipe[1] = -1;
    x->fdinpipe[0] = -1;
    x->fdinpipe[1] = -1;

    x->sr_inhead = x->sr_intail = 0;
    if (!(x->sr_inbuf = (char*) malloc(INBUFSIZE))) bug("t_shell");;

    x->x_binbuf = binbuf_new();

    outlet_new(&x->x_obj, &s_list);
    x->x_done = outlet_new(&x->x_obj, &s_bang);
    x->x_clock = clock_new(x, (t_method) shell_check);
    return (x);
}

void shell_setup(void)
{
    shell_class = class_new(gensym("shell"), (t_newmethod)shell_new, 
			    (t_method)shell_free,sizeof(t_shell), 0,0);
    class_addbang(shell_class,shell_bang);
    class_addanything(shell_class, shell_anything);
}


#endif /* _WIN32 */


