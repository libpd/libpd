#ifdef __gnu_linux__
// find a replacement for <linux/if_packet.h>
// then this will work on other UNIXes

#include "m_pd.h"
#include "ext13.h"
#include <sys/types.h>
#include <string.h>
#include <math.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <linux/if_packet.h>


/* ------------------------ promiscous_tilde~ ----------------------------- */


#define DEFAULT_NIC     "eth0"  //NIC

static t_class *promiscous_tilde_class;


typedef struct _promiscous_tilde
{
     t_object x_obj;
     int opened;
     int sock;
     char* cbuf;
     t_symbol* interface;
} t_promiscous_tilde;


static int setnic_promisc(char *nic_name){
    int sock;           // socket desc
    struct ifreq f;
    struct sockaddr_ll	sll;

    memset(&sll, 0, sizeof(sll));
    sll.sll_family		= AF_PACKET;
    sll.sll_protocol	= htons(ETH_P_ALL);

    //if( (sock = socket(AF_INET, SOCK_PACKET, htons(ETH_P_ALL))) < 0){
    if( (sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0){
        post("promiscous~ failed open socket, must be root (euid)");
        return(-1);
    }
    //strcpy(f.ifr_name, nic_name);
    strncpy(f.ifr_name, nic_name, sizeof(f.ifr_name));

    if (ioctl(sock, SIOCGIFINDEX, &f) < 0) {
      post ("promiscous~: error on setting if index");
    }

#ifdef DEBUG
    post("promiscous~: ifname : %s",f.ifr_name);
    post("promiscous~: ifindex: %d",f.ifr_ifindex);
#endif

    sll.sll_ifindex		= f.ifr_ifindex;

    if (bind(sock, (struct sockaddr *) &sll, sizeof(sll)) < 0) {
      post("promiscous~: couldnt bind to interface: %s", f.ifr_name);
    }

    if( ioctl(sock, SIOCGIFFLAGS, &f) < 0) {
        post("promiscous~ failed to get interface flags, continue anyway");
        return(sock);
    }
    f.ifr_flags |= IFF_PROMISC;
    if( ioctl(sock, SIOCSIFFLAGS, &f) < 0) {
        post("promiscous~ failed to set promisous mode , continue anyway");
    }
    return(sock);
}


t_int *promiscous_tilde_perform(t_int *w)
{
  t_promiscous_tilde*  x = (t_promiscous_tilde*)(w[1]);
  int n = (int)(w[3]);/*number of samples*/
  int l = 0;
  int ll = 0;
  int r;
  int t;
  static unsigned char packet;
  fd_set fdset;
  struct timeval timeout;
  t_float* out = (t_float *)w[2];
  int cptr[n];
  
  if (x->opened){
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    FD_ZERO(&fdset);
    FD_SET(x->sock,&fdset);
    if (r = select(x->sock+1,&fdset,NULL,NULL,&timeout) && x->sock){
      l = recv(x->sock, &cptr,n, 0);
      //l = read(x->sock, (char*) &cptr,n);
    };
    if (l < 0) l = 0;
    while (l--){
      t = cptr[n-l];
      //post("sample: %d",t);
      //*out++ = t / 32767.;
      *out++ = (float) t / (float) pow(2, 32);
      ll++;
    }
  }
  while (ll < n){
    *out++ = 0.;
    //post("sample: %f",*out);
    ll++;
  }
  return (w + 4);
}

static void promiscous_tilde_dsp(t_promiscous_tilde *x, t_signal **sp)
{
	dsp_add(promiscous_tilde_perform, 3, x, sp[0]->s_vec,
	sp[0]->s_n);
}


static void promiscous_tilde_free(t_promiscous_tilde *x){
 /*LATER*/
}


static void *promiscous_tilde_new(t_symbol *ifname)
{
	t_promiscous_tilde *x = (t_promiscous_tilde *)pd_new(promiscous_tilde_class);
	outlet_new(&x->x_obj, gensym("signal"));

	if (*ifname->s_name)
	  x->interface = ifname;
	else
	  x->interface = gensym(DEFAULT_NIC);

	if((x->sock = setnic_promisc(x->interface->s_name))<0){
		post ("could not open interface");
		x->opened = 0;
	}else{
		x->opened = 1;
	}
	return (x);
}


void promiscous_tilde_setup(void)
{
	promiscous_tilde_class = class_new(gensym("promiscous~"), (t_newmethod) promiscous_tilde_new, 0,
		sizeof(t_promiscous_tilde), CLASS_NOINLET, A_DEFSYM, 0);
	class_addmethod(promiscous_tilde_class, (t_method) promiscous_tilde_dsp, gensym("dsp"), 0);
}

#endif
