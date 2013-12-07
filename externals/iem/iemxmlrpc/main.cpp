/*************************************************************
 *
 *    XMLRPC external for PD
 *
 * File: main.cpp
 *
 * Description: This is it all
 *
 * Author: Thomas Grill t.grill [at] gmx.net
 *         Winfried Ritsch some changes
 *
# License LGPL see LICENSE.txt
# IEM - Institute of Electronic Music and Acoustics, Graz
# Inffeldgasse 10/3, 8010 Graz, Austria
# http://iem.at 
#
# CHANGES: 
# V 0.1.3b, LICENSES added, post error removed.
# V 0.2 setup routine doubled for use as xmlrpc and iemxmlrpc
#************************************************************/

//! Version number of this external
#define __IEMXMLRPC_VERSION "0.2"

// prevent MSVC "extern" warning
#ifdef _MSC_VER
#pragma warning( disable : 4091 ) 
#endif

// necessary for a Sleep command
#ifdef WIN32
#include <windows.h>
#endif

// headers contained in the PD distribution
#include <m_pd.h>
#include <pthread.h>

// we are using the xmlrpc++ project (see http://sf.net/projects/xmlrpcpp )
#include <XmlRpc.h>

// include STL map class
#include <map>


#ifdef PD_DEVEL_VERSION
	// here, the sys_lock and sys_unlock functions are defined
	#undef USECLOCK
#else
	#define USECLOCK
#endif


//! Convert XMLRPC values to PD atoms
static bool Val2Atom(XmlRpc::XmlRpcValue &val,t_atom &atom)
{
    switch(val.getType()) {
    case XmlRpc::XmlRpcValue::TypeBoolean: SETFLOAT(&atom,(int)val?0:1); break;
    case XmlRpc::XmlRpcValue::TypeInt: SETFLOAT(&atom,(int)val); break;
    case XmlRpc::XmlRpcValue::TypeDouble: SETFLOAT(&atom,(double)val); break;
    case XmlRpc::XmlRpcValue::TypeString: SETSYMBOL(&atom,gensym((char *)((std::string &)val).c_str())); break;
    default:
        return false;
    }
    return true;
}

//! Convert PD atoms to XMLRPC values
static bool Atom2Val(const t_atom &atom,XmlRpc::XmlRpcValue &val)
{
    switch(atom.a_type) {
    case A_FLOAT: val = (double)atom_getfloat((t_atom *)&atom); break;
    case A_SYMBOL: val = (std::string)(atom_getsymbol((t_atom *)&atom)->s_name);
    default: return false;
    }
    return true;
}



//! proxy class holder
static t_class *px_class = NULL;


/*! \brief This is the proxy class receiving messages to a symbol
    \note No virtual table here!
*/
class bind_proxy {
public:
    //! Obligatory PD object structure
    t_object obj;

    //! this symbol
    const t_symbol *sym;

    //! mutex for value changes
    pthread_mutex_t *mutex;

    //! current value of bound sender
    XmlRpc::XmlRpcValue value;

    //! Initalize structure
    void Init(const t_symbol *s,pthread_mutex_t *mtx) 
    {
        sym = s; mutex = mtx;
        value.clear();
    }

    /*! \brief PD method called when a bound symbol changes its value
        \note This is called from PD system thread
    */
    static void px_method(bind_proxy *obj,const t_symbol *s,int argc,const t_atom *argv);
};


void bind_proxy::px_method(bind_proxy *obj,const t_symbol *s,int argc,const t_atom *argv)
{
    pthread_mutex_lock(obj->mutex);

    obj->value.clear();

    if(s == &s_float && argc == 1 && argv->a_type == A_FLOAT) 
        // float value
        obj->value = (double)atom_getfloat((t_atom *)argv);
    else if(argc == 0)
        // symbol value
        obj->value = (std::string)s->s_name;
    else if(s == &s_symbol && argc == 1 && argv->a_type == A_SYMBOL) 
        // this is probably never called
        obj->value = (std::string)atom_getsymbol((t_atom *)argv)->s_name;
    else {
        // anything or list

        // look if header is "list" and don't propagate it to the atom list
        int hdr = s == &s_list?0:1;

        // set the parameters
        obj->value.setSize(argc+hdr);

        if(hdr) obj->value[0] = s->s_name;

        for(int i = 0; i < argc; ++i) {
            if(!Atom2Val(argv[i],obj->value[i+hdr]))
#ifdef _DEBUG
                error("XMLRPC: Internal error - version %s, file %s, line %i",__IEMXMLRPC_VERSION,__FILE__,__LINE__);
#else
            {}
#endif
        }
    }

    pthread_mutex_unlock(obj->mutex);
}


//! Type to map a symbol name to the associated proxy class
typedef std::map<std::string,bind_proxy *> ProxyMapper;


//! Our XMLRPC server 
class RPCServer:
    public XmlRpc::XmlRpcServer
{
public:
    RPCServer(int port);
    ~RPCServer();

    void Work() { work(-1.0); }

    class Message
    {
    public:
        enum Msg { load,close,send,bind,unbind,query };

        Message(Msg m,XmlRpc::XmlRpcValue &val,XmlRpc::XmlRpcValue &res,pthread_mutex_t &mtx): 
            msg(m),value(val),result(res),mutex(mtx)
        {
#ifdef USECLOCK
            pthread_cond_init(&cond,NULL);
#endif
        }

        ~Message()
        {
#ifdef USECLOCK
            pthread_cond_destroy(&cond);
#endif
        }

#ifdef USECLOCK
        void Signal()
        {
            pthread_cond_signal(&cond);
        }

        void Wait()
        {
            pthread_mutex_lock(&mutex);
            pthread_cond_wait(&cond,&mutex);
            pthread_mutex_unlock(&mutex);
        }

        pthread_cond_t cond;
#endif
        pthread_mutex_t &mutex;

        std::string err;
        Msg msg;
        XmlRpc::XmlRpcValue &value,&result;
    };

    std::list<Message *> msglist;
    pthread_mutex_t qu_mutex,msg_mutex,px_mutex;

    //! Add message to queue (thread-safe)
    void MsgPush(Message *m);
    //! Fetch message from queue (thread-safe)
    Message *MsgPop();

    //! loaded PD patch
    t_pd *canvas;

    //! bound symbols and their proxies
    ProxyMapper objmap;

#ifdef USECLOCK
    /*! \brief trigger message worker
        \remark thread-safe!
    */
    void Trigger() { clock_delay(clk,0); }

    //! worker function
    static void clk_worker(RPCServer *s) { s->MsgWorker(); }

    //! worker function
    void MsgWorker();

    //! worker clock
    t_clock *clk;
#endif

	//! process message
	void MsgProcess(Message &m);

    void Methods(Message::Msg msg,XmlRpc::XmlRpcValue &params, XmlRpc::XmlRpcValue &result);

    void Method_Load(XmlRpc::XmlRpcValue &params, XmlRpc::XmlRpcValue &result);
    void Method_Close(XmlRpc::XmlRpcValue &params, XmlRpc::XmlRpcValue &result);
    void Method_Bind(XmlRpc::XmlRpcValue &params, XmlRpc::XmlRpcValue &result);
    void Method_Unbind(XmlRpc::XmlRpcValue &params, XmlRpc::XmlRpcValue &result);
    void Method_Send(XmlRpc::XmlRpcValue &params, XmlRpc::XmlRpcValue &result);
    void Method_Query(XmlRpc::XmlRpcValue &params, XmlRpc::XmlRpcValue &result);
};

RPCServer::RPCServer(int port):
    canvas(NULL)
{
    // Create the server socket on the specified port
    bindAndListen(port);

    // Enable introspection API
    enableIntrospection(true);

    // make mutex for message queue
    pthread_mutex_init(&qu_mutex,NULL);
    pthread_mutex_init(&msg_mutex,NULL);
    pthread_mutex_init(&px_mutex,NULL);

#ifdef USECLOCK
    // create worker clock
    clk = clock_new(this,(t_method)clk_worker);
#endif
}

RPCServer::~RPCServer()
{
    // delete message queue mutex
    pthread_mutex_destroy(&qu_mutex);
    pthread_mutex_destroy(&msg_mutex);
    pthread_mutex_destroy(&px_mutex);

#ifdef USECLOCK
    clock_free(clk);
#endif
}

void RPCServer::MsgPush(Message *m)
{
    pthread_mutex_lock(&qu_mutex);
    msglist.push_back(m);
    pthread_mutex_unlock(&qu_mutex);
}

RPCServer::Message *RPCServer::MsgPop()
{
    Message *m = NULL;

    pthread_mutex_lock(&qu_mutex);
    if(!msglist.empty()) {
        // fetch reference from queue
        m = msglist.front();
        msglist.pop_front();
    }
    pthread_mutex_unlock(&qu_mutex);

    return m;
}

//! process message
void RPCServer::MsgProcess(Message &m)
{
    try {
        // choose function
        switch(m.msg) {
        case Message::load: Method_Load(m.value,m.result); break;
        case Message::close: Method_Close(m.value,m.result); break;
        case Message::bind: Method_Bind(m.value,m.result); break;
        case Message::unbind: Method_Unbind(m.value,m.result); break;
        case Message::send: Method_Send(m.value,m.result); break;
        }
    }

    // catch all exceptions as they can't propagate across threads
    catch(const char *txt) {
        m.err = txt;
    }
    catch(const std::string &txt) {
        m.err = txt;
    }
    catch(const XmlRpc::XmlRpcException &exc) {
        m.err = std::string("XMLRPC++ error: ")+exc.getMessage();
    }
    catch(...) {
        m.err = "Undefined method error";
    }
}

#ifdef USECLOCK
//! worker function
void RPCServer::MsgWorker()
{
    for(;;) {
        // fetch reference from queue
        Message *m = MsgPop();
        if(!m) break;

		MsgProcess(*m);
		
        // signal message to be finished
        m->Signal();
    }
}
#endif


//! dispatcher function
void RPCServer::Methods(Message::Msg msg,XmlRpc::XmlRpcValue &params, XmlRpc::XmlRpcValue &result)
{
    switch(msg) {
    // these can execute in the XMLRPC thread
    case Message::query: 
        Method_Query(params,result); break;

    // all others need to be executed in the PD thread
    default: {
        // make new message
        Message *m = new Message(msg,params,result,msg_mutex);

#ifdef USECLOCK
        // add to message queue
        MsgPush(m);

		// trigger PD clock
        Trigger();

        // wait for function to finish in PD system thread
        m->Wait();
#else
		// lock PD
		sys_lock();

		// process message
		MsgProcess(*m); 

		// unlock PD
		sys_unlock();
#endif

        // save eventual error string
        std::string err = m->err;

        // delete message
        delete m;

        // if error string is non-null throw exception
        if(err.length()) throw err;
    }
    }
}


/*! \brief "load" method
    \param string-array XML data representing a PD patch

    This method takes a string array containing the code of a PD patch to load.
*/
void RPCServer::Method_Load(XmlRpc::XmlRpcValue &params, XmlRpc::XmlRpcValue &result)
{
    XmlRpc::XmlRpcValue *parr;
    if(params.getType() == XmlRpc::XmlRpcValue::TypeArray && params[0].getType() == XmlRpc::XmlRpcValue::TypeString)
        parr = &params;
    else if(params.size() == 1 && params[0].getType() == XmlRpc::XmlRpcValue::TypeArray)
        parr = &params[0];
    else 
        throw "Wrong syntax - should be 'load string-array'";

#ifndef _DEBUG
    // stop dsp while the numerous objects are created
   	int dspstate = canvas_suspend_dsp();
#endif

    // if there is already a canvas... trash it!
    if(canvas) pd_free(canvas);

    // a current directory must be set so that PD creates an environment for the new canvas
    // see pd/src/g_canvas.c - function canvas_new "if (canvas_newdirectory->s_name[0])"
    // if there would be no canvas environment PD would crash
    glob_setfilename(NULL,gensym("IAEM"),gensym("."));

    for(int i = 0; i < parr->size(); ++i) {
        // here, an exception can happen if (*parr)[i] is not a string!
        const std::string &str = (std::string &)(*parr)[i];

        // allocate binbuf storage
        t_binbuf *b = binbuf_new();

        // create binbuf from the string
	    binbuf_text(b, (char *)str.c_str(), str.size());
        // evaluate binbuf - create canvas, objects or whatever
	    binbuf_eval(b, 0, 0, 0);

        // free storage
   	    binbuf_free(b);

        // here, it would be interesting if the object has been sucessfully created...
        // we could use the following...
#if (PD_MINOR_VERSION >= 37) || defined(PD_DEVEL_VERSION)
        // This pd_newest is currently only defined with PD veriosn >= 0.37 or in the 
        // CVS devel_0_36 branch on http://sf.net/projects/pure-data
        // It is needed for knowing whether an object could be created or not
        // However, it doesn't work for messages or comments for which pd_newest is not reset...
        // Since the first statement is object a canvas, which (on success) sets pd_newest
        // to non-null this should not be a problem for following msgs or comments.
		t_pd *x = pd_newest();
        if(!x) throw "Could not create object";
#endif
    }

    // clear current file and path
    // this is done because it is done in g_canvas.c
    glob_setfilename(NULL,&s_,&s_);

    // now resume dsp
#ifndef _DEBUG
    canvas_resume_dsp(dspstate);
#endif

    // get the patch canvas
    t_pd *x = s__X.s_thing;
    if(x) {
        // pop newly created canvas
        pd_vmess(x, gensym("pop"), "i", 1);

        // issue load bang
        pd_vmess(x, gensym("loadbang"), "");

        // save created canvas object
        canvas = x;
    }
    else
        throw "Canvas could not be created";
}

/*! \brief "close" method

    The previously loaded PD patch (by "load method") is closed
*/
void RPCServer::Method_Close(XmlRpc::XmlRpcValue &params, XmlRpc::XmlRpcValue &result)
{
    if(params.valid() && params.size())
        throw "Wrong syntax - method 'close' takes no parameters";

    if(canvas) {
        pd_free(canvas);
        canvas = NULL;
    }
    else
        throw "No open canvas";
}

/*! \brief "bind" method
    \parameter symbol PD symbol to bind a callback to
    \return Fault if symbol has already been bound

    Registers a symbol for query or active feedback 
*/
void RPCServer::Method_Bind(XmlRpc::XmlRpcValue &params, XmlRpc::XmlRpcValue &result)
{
    if(!params.valid() || params.size() != 1)
        throw "Wrong syntax - should be 'bind symbol'";

    // check type of params[0] before evaluating the rest
    // this throws an exception if it's not a string
    std::string &name = (std::string &)params[0];

    // look for existing proxy object
    ProxyMapper::iterator iter = objmap.find(name);

    if(iter == objmap.end()) {
        // new proxy object
        bind_proxy *px = (bind_proxy *)pd_new(px_class);
        px->Init(gensym((char *)name.c_str()),&px_mutex);

        // bind symbol to object
	    pd_bind(&px->obj.ob_pd,(t_symbol *)px->sym);
        
        // add to map
        objmap[name] = px;
    }
    else 
        throw "The symbol is already bound";
}

/*! \brief "unbind" method
    \parameter symbol Previously bound PD symbol
    \return Fault if symbol is not found or has not been bound

    Free a previously bound symbol from its callback 
*/
void RPCServer::Method_Unbind(XmlRpc::XmlRpcValue &params, XmlRpc::XmlRpcValue &result)
{
    if(!params.valid() || params.size() != 1)
        throw "Wrong syntax - should be 'unbind symbol'";

    // check type of params[0] before evaluating the rest
    // this throws an exception if it's not a string
    std::string &name = (std::string &)params[0];

    // look for existing proxy object
    ProxyMapper::iterator iter = objmap.find(name);

    if(iter != objmap.end()) {
        // retrieve pointer to proxy from map
        bind_proxy *px = iter->second;

        // unbind and free proxy object
    	pd_unbind(&px->obj.ob_pd,gensym((char *)name.c_str()));  
	    pd_free((t_pd *)&px->obj);

        // remove from map
        objmap.erase(iter);
    }
    else 
        throw "The symbol is not bound";
}

/*! \brief "send" method
    \param symbol PD symbol to send data to
    \param parameters Data values (translated to PD atoms)

    Send data to a receiving symbol in PD
*/
void RPCServer::Method_Send(XmlRpc::XmlRpcValue &params, XmlRpc::XmlRpcValue &result)
{
    if(!params.valid() || params.size() < 2)
        throw "Wrong syntax - should be 'send symbol parameters ...'";

    // check type of params[0] before evaluating the rest
    // this throws an exception if it's not a string
    std::string &recv = (std::string &)params[0];

    int offset = 1;
    XmlRpc::XmlRpcValue *val = &params;

    // parameter 1 can also be an array (but no other parameters may follow!)
    if(params.size() == 2 && params[1].getType() == XmlRpc::XmlRpcValue::TypeArray) {
        offset = 0;
        val = &params[1]; 
    }

    t_symbol *sym = gensym((char *)recv.c_str());
    int argc = val->size()-offset;
    t_atom *args = new t_atom[argc];

    // iterate through argument list
    for(int i = offset; i < val->size(); ++i) {
        if(!Val2Atom((*val)[i],args[i-offset]))
            throw "Parameter type not handled";
    }
	
#ifdef WIN32
	// this is not very nice but I have no clue why sometimes this 
	// function hangs under windows. seems to be related to the 
	// pd function called and some mutex/thread issues
	// remove this windows hack if it reveals why it blocks
	Sleep(3);
#endif

    // forward parameters to receiving symbol
    if(sym && sym->s_thing)
        pd_forwardmess(sym->s_thing,argc,args);
    else
        post("XMLRPC: receiver %s not found",recv.c_str());

    delete[] args;
}

/*! \brief "query" method
    \parameter symbol PD symbol to query
    \return Value of sender symbol

    Queries the current value of a PD symbol
*/
void RPCServer::Method_Query(XmlRpc::XmlRpcValue &params, XmlRpc::XmlRpcValue &result)
{
    if(!params.valid() || params.size() != 1)
        throw "Wrong syntax - should be 'query symbol'";

    // check type of params[0] before evaluating the rest
    // this throws an exception if it's not a string
    std::string &name = (std::string &)params[0];

    // lock mutex to protect objmap
    pthread_mutex_lock(&px_mutex);

    // look for existing proxy object
    ProxyMapper::iterator iter = objmap.find(name);

    // initialize px to NULL (serves as a flag for error exception)
    bind_proxy *px = NULL;

    if(iter != objmap.end()) {
        // retrieve pointer to proxy from map
        px = iter->second;

        // return current value stored in proxy
        result = px->value;
    }

    pthread_mutex_unlock(&px_mutex);

    // throw exception if px has not been set
    if(!px) throw "Symbol not found";
}


//! Base class for all our XMLRPC methods
class RPCMethod:
    public XmlRpc::XmlRpcServerMethod
{
public:
    RPCMethod(const char *name,RPCServer *s,RPCServer::Message::Msg msg,const char *help): 
         XmlRpc::XmlRpcServerMethod(name,s),
         srvmsg(msg),helptext(help)
    {}

protected:
    //! the actual worker function where all is done 
    virtual void worker(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result)
    {
        getServer().Methods(srvmsg,params,result);
    }

    //! return help string
    virtual std::string help() { return helptext; }

    //! execute RPC method and handle exceptions
    void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result)
    {
        try { worker(params,result); }
        
        // the exceptions are mostly typed as strings
        catch(const char *str) {
            throw XmlRpc::XmlRpcException(str);
        }
        catch(const std::string &str) {
            throw XmlRpc::XmlRpcException(str);
        }
        catch(const XmlRpc::XmlRpcException &exc) {
            throw exc;
        }

        // treat others here generically and protect PD from crashing
        catch(...) {
            throw XmlRpc::XmlRpcException("Undefined error, please report");
        }
    }

    //! get reference to RPC server 
    RPCServer &getServer() { return *(RPCServer *)_server; }

    std::string helptext;
    const RPCServer::Message::Msg srvmsg;
};



//! our XMLRPC server
static RPCServer *xmlrpc_server = NULL;


//! Thread function where our XMLRPC server runs
static void *xmlrpc_worker(void *)
{
#ifdef _DEBUG
    // In debug mode display some messages
    XmlRpc::setVerbosity(1);

    if(!xmlrpc_server)
        error("XMLRPC: Internal error - version %s, file %s, line %i",__IEMXMLRPC_VERSION,__FILE__,__LINE__);
    else 
#endif
    {
        // Register all methods
        RPCMethod meth_load("load",xmlrpc_server,RPCServer::Message::load,"Load a patch - Syntax: 'load string-array'");
        RPCMethod meth_close("close",xmlrpc_server,RPCServer::Message::close,"Close the open patch");
        RPCMethod meth_send("send",xmlrpc_server,RPCServer::Message::send,"Send a value to a symbol - Syntax:'send symbol parameters ...'");
        RPCMethod meth_bind("bind",xmlrpc_server,RPCServer::Message::bind,"Bind a callback to a PD symbol - Syntax:'bind symbol'");
        RPCMethod meth_unbind("unbind",xmlrpc_server,RPCServer::Message::unbind,"Unbind a callback from a PD symbol - Syntax:'unbind symbol'");
        RPCMethod meth_query("query",xmlrpc_server,RPCServer::Message::query,"Query the value of PD symbol - Syntax:'query symbol'");

        // Wait for requests indefinitely
        xmlrpc_server->Work();
    }

    // This point is never reached
    delete xmlrpc_server; xmlrpc_server = NULL;

    return NULL;
}


//! xmlrpc class holder
static t_class *xmlrpc_class = NULL;


/*! \brief xmlrpc object creation function
    
    This function launches the xmlrpc thread
*/
static t_object *xmlrpc_new(t_floatarg p)
{
    t_object *ret = NULL;
    
    if(xmlrpc_server) 
        post("XMLRPC: server has already been launched");
    else {
        if(p <= 0)
            post("XMLRPC: port must be > 0");
        else {
            int port = (int)p;

            // Initialize server
            xmlrpc_server = new RPCServer(port);

            // launch worker function
            pthread_t id;
            if(pthread_create(&id,NULL,xmlrpc_worker,xmlrpc_server)) {
                error("XMLRPC thread could not be launched");

                delete xmlrpc_server; xmlrpc_server = NULL;
            }
            else {
                // success -> create object
                ret = (t_object *)pd_new(xmlrpc_class);

                post("XMLRPC: server is listening at port %i",port);
            }
        }
    }
    return ret;
}


/*! \brief Library setup routine
    \note Must be exported from shared library
*/
extern "C" { 
#ifdef NT
__declspec(dllexport)
#endif
	
void xmlrpc_setup()
{
    // post some message to the console
	post("IEMXMLRPC, version " __IEMXMLRPC_VERSION ", (C)2003 IEM Graz");
	post("");

    // register the proxy class for binding symbols
	px_class = class_new(gensym("xmlrpc proxy"),NULL,NULL,sizeof(bind_proxy),CLASS_PD|CLASS_NOINLET, A_NULL);
    // one method for all messages to proxy
	class_addanything(px_class,bind_proxy::px_method); 
 
    // register the xmlrpc class
	xmlrpc_class = class_new(gensym("xmlrpc"),(t_newmethod)xmlrpc_new,NULL,sizeof(t_object),CLASS_NOINLET, A_FLOAT,A_NULL);
}

void iemxmlrpc_setup() { xmlrpc_setup(); }

}
