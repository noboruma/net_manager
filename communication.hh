#ifndef GLOBAL_CONNECTION_HH
#define GLOBAL_CONNECTION_HH

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>
#include <functional>
#include <thread>
#include <atomic>
#include <sys/un.h>
#include <vector>

#include <iostream>
#include <stdexcept>
#include <stdexcept>
namespace net 
{
  enum class role
  {
    HOST,
    CLIENT
  };

  enum class protocol
  {
    UDP,
    TCP,
    PIPE //UNIX domain
  };

  typedef struct sockaddr_in sockaddr_in; // Internet
  typedef struct sockaddr sockaddr;
  typedef struct sockaddr_un sockaddr_un; // Unix

  template<role t, protocol p>
  struct communication;

  namespace abstract
  {
    struct communication
    {
      communication() : listening(true)
      {}
      ~communication()
      {
        close(socket_fd);
        listening = false;
      }
      protected:
        int socket_fd;
        std::atomic_bool listening;
    };

    template<role t, protocol p>
    using callback = std::function<void(int,const std::string&, net::communication<t,p>& )>;
  } //!abstract


  template<protocol p>
  struct communication<role::HOST, p> : public abstract::communication
  {
    typedef abstract::callback<role::HOST,p> callback;

    communication(unsigned port, const callback& c);

    private:
    sockaddr_in server_socket;
    std::vector<sockaddr_in> client_socket;
  };

  template<protocol p>
  struct communication<role::CLIENT, p> : public abstract::communication
  {

    typedef abstract::callback<role::CLIENT,p> callback;

    communication(const std::string& addr, unsigned port, const callback& c);

    void send(const std::string& s);

    private:
    sockaddr_in server;
    struct hostent *hp;
  };

  template<>
  struct communication<role::HOST, protocol::PIPE> : public abstract::communication
  {
    typedef abstract::callback<role::HOST, protocol::PIPE> callback;

    communication(unsigned port, const callback& c, std::string name = "");

    std::string get_file_path()
    {
      return file_path;
    }

    ~communication()
    {
      unlink(file_path);
      delete file_path;
    }

    private:
    sockaddr_un server_socket;
    std::vector<sockaddr_un> client_socket;
    char * file_path;
  };

  template<>
  struct communication<role::CLIENT, protocol::PIPE> : public abstract::communication
  {

    typedef abstract::callback<role::CLIENT, protocol::PIPE> callback;

    communication(const std::string& addr, unsigned port, const callback& c);

    void send(const std::string& s);

    private:
    sockaddr_un server;
    struct hostent *hp;
  };

} //!net

#include "communication.hxx"
  
#endif
