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

  template<type t, protocol p, unsigned connect_num=1>
  struct connection;

  typedef struct sockaddr_in inet_sockaddr_t;
  typedef struct sockaddr sockaddr_t;
  typedef std::function<void(int,const std::string&)> callback;

  template<protocol p,unsigned connect_num>
  class connection<type::HOST, p, connect_num>
  {

    connection(unsigned port, const callback& c)
    : listening(true)
    {
      if(p == protocol::TCP)
      {
        socket_fd = socket(AF_INET,
                           SOCK_STREAM,
                           0);

        bzero((char*) &server_socket, sizeof(server_socket));
        server_socket.sin_family = AF_INET;
        server_socket.sin_addr.s_addr = INADDR_ANY;
        server_socket.sin_port = htons(port);


        bind(socket_fd,
             (sockaddr_t*)server_socket,
             sizeof(sockaddr_t));

        listen(socket_fd, 5);

        int cli_num;

        int newfd = accept(socket_fd,
                           (sockaddr_t*) &server_socket,
                           &cli_num);

        std::thread([&] 
                    {
                      char buf[1024];
                      while(read(newfd, 1024, buf))
                      {
                        c(newfd, buf);
                      }
                    });
      }
      else if(p == protocol::UDP)
      {
        socket_fd = socket(AF_INET,
                           SOCK_DGRAM,
                           0);
        bzero((char*) &server_socket, sizeof(server_socket));
        server_socket.sin_family = AF_INET;
        server_socket.sin_addr.s_addr = INADDR_ANY;
        server_socket.sin_port = htons(port);

        bind(socket_fd,
             (sockaddr_t*)server_socket,
             sizeof(inet_sockaddr_t));

        std::thread([&] 
                    {
                    char buf[1024];
                    inet_sockaddr_t from;
                    int n = recvfrom(socket_fd,
                        buf,
                        1024,
                        0,
                        (sockaddr_t*) &from,
                        sizeof(inet_sockaddr_t));
                    });
      }

    }

    void operator()(const callback &c)
    {
      char buf[1024];
      inet_sockaddr_t from;
      if(p == protocol::UDP)
      {
        int n = recvfrom(socket_fd,
                        buf,
                        1024,
                        0,
                        (sockaddr_t*) &from,
                        sizeof(inet_sockaddr_t));
        //sendto(socket_fd)
      }
      else if(p == protocol::TCP)
      {
        ;
      }
    }


    ~connection();
  
    private:
    inet_sockaddr_t server_socket;
    inet_sockaddr_t client_socket[connect_num];

    int socket_fd;
        //port;

    bool listening;
  };

  template<protocol p, unsigned connect_num>
  class connection<type::CLIENT, p, connect_num>
  {
    connection(const std::string& addr, unsigned port, const callback& c)
    {
      if(p == protocol::UDP)
      {
      socket_fd = socket(AF_INET,
                       SOCK_DGRAM,
                       0);
        server.sin_family = AF_INET;
        hp = gethostbyname(addr.c_str());
        bcopy((char*)hp->h_addr,
              (char*)&server.sin_addr,
              hp->h_length);
        server.sin_port = htons(port);
        //sento/recvfrom
      }
    }

    void send(const std::string& s)
    {
      if(p == protocol::UDP)
        sendto(socket_fd, s.c_str(), s.length(), 0, (const struct sockaddr*)&server, sizeof(struct sockaddr));
    }

    ~connection();
  
    private:
    inet_sockaddr_t server;
    int socket_fd;
    struct hostent *hp;
    bool listening;
  };

} //!net
  
#endif
