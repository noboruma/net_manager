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
  enum class type
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

  template<type t, protocol p>
  struct connection;

  namespace abstract
  {
    struct connection
    {
      connection() : listening(true)
      {}
      ~connection()
      {
        close(socket_fd);
        listening = false;
      }
      protected:
        int socket_fd;
        std::atomic_bool listening;
    };

    template<type t, protocol p>
    using callback =  std::function<void(int,const std::string&, net::connection<t,p>& )>;
  } //!abstract

  typedef struct sockaddr_in inet_sockaddr_t;
  typedef struct sockaddr sockaddr_t;


  template<protocol p>
  struct connection<type::HOST, p> : public abstract::connection
  {
    typedef abstract::callback<type::HOST,p> callback;

    connection(unsigned port, const callback& c);

    private:
    sockaddr_in server_socket;
    std::vector<inet_sockaddr_t> client_socket;
  };

  template<protocol p>
  struct connection<type::CLIENT, p> : public abstract::connection
  {

    typedef abstract::callback<type::CLIENT,p> callback;

    connection(const std::string& addr, unsigned port, const callback& c);

    void send(const std::string& s);

    private:
    inet_sockaddr_t server;
    struct hostent *hp;
  };

  template<>
  struct connection<type::HOST, protocol::PIPE> : public abstract::connection
  {
    typedef abstract::callback<type::HOST, protocol::PIPE> callback;

    connection(unsigned port, const callback& c, std::string name = "");

    std::string get_file_path()
    {
      return file_path;
    }

    ~connection()
    {
      delete file_path;
    }

    private:
    struct sockaddr_un server_socket;
    std::vector<sockaddr_un> client_socket;
    char * file_path;
  };

  template<>
  struct connection<type::CLIENT, protocol::PIPE> : public abstract::connection
  {

    typedef abstract::callback<type::CLIENT, protocol::PIPE> callback;

    connection(const std::string& addr, unsigned port, const callback& c);

    void send(const std::string& s);

    private:
    struct sockaddr_un server;
    struct hostent *hp;
  };

} //!net

#include "connection.hxx"
  
#endif
