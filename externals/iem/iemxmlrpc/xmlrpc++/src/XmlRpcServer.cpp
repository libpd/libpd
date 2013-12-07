
#include "XmlRpcServer.h"
#include "XmlRpcServerConnection.h"
#include "XmlRpcServerMethod.h"
#include "XmlRpcSocket.h"
#include "XmlRpcUtil.h"
#include "XmlRpcException.h"


using namespace XmlRpc;


XmlRpcServer::XmlRpcServer()
{
  _introspectionEnabled = false;
  _listMethods = 0;
  _methodHelp = 0;
}


XmlRpcServer::~XmlRpcServer()
{
  this->shutdown();
  _methods.clear();
  delete _listMethods;
  delete _methodHelp;
}


// Add a command to the RPC server
void 
XmlRpcServer::addMethod(XmlRpcServerMethod* method)
{
  _methods[method->name()] = method;
}

// Remove a command from the RPC server
void 
XmlRpcServer::removeMethod(XmlRpcServerMethod* method)
{
  MethodMap::iterator i = _methods.find(method->name());
  if (i != _methods.end())
    _methods.erase(i);
}

// Remove a command from the RPC server by name
void 
XmlRpcServer::removeMethod(const std::string& methodName)
{
  MethodMap::iterator i = _methods.find(methodName);
  if (i != _methods.end())
    _methods.erase(i);
}


// Look up a method by name
XmlRpcServerMethod* 
XmlRpcServer::findMethod(const std::string& name) const
{
  MethodMap::const_iterator i = _methods.find(name);
  if (i == _methods.end())
    return 0;
  return i->second;
}


// Create a socket, bind to the specified port, and
// set it in listen mode to make it available for clients.
bool 
XmlRpcServer::bindAndListen(int port, int backlog /*= 5*/)
{
  int fd = XmlRpcSocket::socket();
  if (fd < 0)
  {
    XmlRpcUtil::error("XmlRpcServer::bindAndListen: Could not create socket (%s).", XmlRpcSocket::getErrorMsg().c_str());
    return false;
  }

  this->setfd(fd);

  // Don't block on reads/writes
  if ( ! XmlRpcSocket::setNonBlocking(fd))
  {
    this->close();
    XmlRpcUtil::error("XmlRpcServer::bindAndListen: Could not set socket to non-blocking input mode (%s).", XmlRpcSocket::getErrorMsg().c_str());
    return false;
  }

  // Allow this port to be re-bound immediately so server re-starts are not delayed
  if ( ! XmlRpcSocket::setReuseAddr(fd))
  {
    this->close();
    XmlRpcUtil::error("XmlRpcServer::bindAndListen: Could not set SO_REUSEADDR socket option (%s).", XmlRpcSocket::getErrorMsg().c_str());
    return false;
  }

  // Bind to the specified port on the default interface
  if ( ! XmlRpcSocket::bind(fd, port))
  {
    this->close();
    XmlRpcUtil::error("XmlRpcServer::bindAndListen: Could not bind to specified port (%s).", XmlRpcSocket::getErrorMsg().c_str());
    return false;
  }

  // Set in listening mode
  if ( ! XmlRpcSocket::listen(fd, backlog))
  {
    this->close();
    XmlRpcUtil::error("XmlRpcServer::bindAndListen: Could not set socket in listening mode (%s).", XmlRpcSocket::getErrorMsg().c_str());
    return false;
  }

  XmlRpcUtil::log(2, "XmlRpcServer::bindAndListen: server listening on port %d fd %d", port, fd);

  // Notify the dispatcher to listen on this source when we are in work()
  _disp.addSource(this, XmlRpcDispatch::ReadableEvent);

  return true;
}


// Process client requests for the specified time
void 
XmlRpcServer::work(double msTime)
{
  XmlRpcUtil::log(2, "XmlRpcServer::work: waiting for a connection");
  _disp.work(msTime);
}



// Handle input on the server socket by accepting the connection
// and reading the rpc request.
unsigned
XmlRpcServer::handleEvent(unsigned mask)
{
  acceptConnection();
  return XmlRpcDispatch::ReadableEvent;		// Continue to monitor this fd
}


// Accept a client connection request and create a connection to
// handle method calls from the client.
void
XmlRpcServer::acceptConnection()
{
  int s = XmlRpcSocket::accept(this->getfd());
  XmlRpcUtil::log(2, "XmlRpcServer::acceptConnection: socket %d", s);
  if (s < 0)
  {
    //this->close();
    XmlRpcUtil::error("XmlRpcServer::acceptConnection: Could not accept connection (%s).", XmlRpcSocket::getErrorMsg().c_str());
  }
  else if ( ! XmlRpcSocket::setNonBlocking(s))
  {
    XmlRpcSocket::close(s);
    XmlRpcUtil::error("XmlRpcServer::acceptConnection: Could not set socket to non-blocking input mode (%s).", XmlRpcSocket::getErrorMsg().c_str());
  }
  else  // Notify the dispatcher to listen for input on this source when we are in work()
  {
    XmlRpcUtil::log(2, "XmlRpcServer::acceptConnection: creating a connection");
    _disp.addSource(this->createConnection(s), XmlRpcDispatch::ReadableEvent);
  }
}


// Create a new connection object for processing requests from a specific client.
XmlRpcServerConnection*
XmlRpcServer::createConnection(int s)
{
  // Specify that the connection object be deleted when it is closed
  return new XmlRpcServerConnection(s, this, true);
}


void 
XmlRpcServer::removeConnection(XmlRpcServerConnection* sc)
{
  _disp.removeSource(sc);
}


// Stop processing client requests
void 
XmlRpcServer::exit()
{
  _disp.exit();
}


// Close the server socket file descriptor and stop monitoring connections
void 
XmlRpcServer::shutdown()
{
  // This closes and destroys all connections as well as closing this socket
  _disp.clear();
}


// Introspection support
static const std::string LIST_METHODS("system.listMethods");
static const std::string METHOD_HELP("system.methodHelp");
static const std::string MULTICALL("system.multicall");


// List all methods available on a server
class ListMethods : public XmlRpcServerMethod
{
public:
  ListMethods(XmlRpcServer* s) : XmlRpcServerMethod(LIST_METHODS, s) {}

  void execute(XmlRpcValue& params, XmlRpcValue& result)
  {
    _server->listMethods(result);
  }

  std::string help() { return std::string("List all methods available on a server as an array of strings"); }
};


// Retrieve the help string for a named method
class MethodHelp : public XmlRpcServerMethod
{
public:
  MethodHelp(XmlRpcServer* s) : XmlRpcServerMethod(METHOD_HELP, s) {}

  void execute(XmlRpcValue& params, XmlRpcValue& result)
  {
    if (params[0].getType() != XmlRpcValue::TypeString)
      throw XmlRpcException(METHOD_HELP + ": Invalid argument type");

    XmlRpcServerMethod* m = _server->findMethod(params[0]);
    if ( ! m)
      throw XmlRpcException(METHOD_HELP + ": Unknown method name");

    result = m->help();
  }

  std::string help() { return std::string("Retrieve the help string for a named method"); }
};

    
// Specify whether introspection is enabled or not. Default is enabled.
void 
XmlRpcServer::enableIntrospection(bool enabled)
{
  if (_introspectionEnabled == enabled)
    return;

  _introspectionEnabled = enabled;

  if (enabled)
  {
    if ( ! _listMethods)
    {
      _listMethods = new ListMethods(this);
      _methodHelp = new MethodHelp(this);
    } else {
      addMethod(_listMethods);
      addMethod(_methodHelp);
    }
  }
  else
  {
    removeMethod(LIST_METHODS);
    removeMethod(METHOD_HELP);
  }
}


void
XmlRpcServer::listMethods(XmlRpcValue& result)
{
  int i = 0;
  result.setSize(_methods.size()+1);
  for (MethodMap::iterator it=_methods.begin(); it != _methods.end(); ++it)
    result[i++] = it->first;

  // Multicall support is built into XmlRpcServerConnection
  result[i] = MULTICALL;
}



