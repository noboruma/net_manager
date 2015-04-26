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

#include <iostream>
#include <stdexcept>
#include <stdexcept>

void error(const char *msg)
{
    perror(msg);
    exit(0);
}
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

  template<type t, protocol p, unsigned c>
  using callback_ =  std::function<void(int,const std::string&, connection<t,p,c>& )>;

  template<protocol p,unsigned connect_num>
  struct connection<type::HOST, p, connect_num>
  {
    typedef callback_<type::HOST,p,connect_num> callback;

    connection(unsigned port, const callback& c)
    : listening(true)
    {
      if(p == protocol::UDP)
        socket_fd = socket(AF_INET,
                           SOCK_DGRAM,
                           IPPROTO_UDP);
      else
        socket_fd = socket(AF_INET,
                           SOCK_STREAM,
                           0);

      if(socket_fd < 0) throw std::logic_error("Socket creation failed");

      bzero((char*) &server_socket, sizeof(server_socket));
      server_socket.sin_family = AF_INET;
      server_socket.sin_addr.s_addr = htonl(INADDR_ANY);
      server_socket.sin_port = htons(port);

      if(bind(socket_fd,
              (struct sockaddr*)&server_socket,
              sizeof(server_socket)))
        throw std::logic_error("Socket bind failed");

      if(p == protocol::TCP)
      {
        listen(socket_fd, 5);

        unsigned cli_num;

        int newfd = accept(socket_fd,
                           (sockaddr_t*) &server_socket,
                           &cli_num);

        std::thread([&] 
                    {
                      char buf[1024];
                      while(read(newfd, buf, 1024))
                      {
                        c(newfd, buf, *this);
                      }
                    });
      }
      else if(p == protocol::UDP)
      {
        std::thread listening_thread([&] 
                                     {
           char buf[1024] = {0};

           while(listening)
           {
             inet_sockaddr_t from;
             socklen_t len = sizeof(inet_sockaddr_t);

             if (recvfrom(socket_fd,
                              buf,
                              1024,
                              0,
                              (sockaddr_t*) &from,
                              &len) == -1)
               continue; // or break?
             // ACK
             if (sendto(socket_fd, buf, 1, 0, (struct sockaddr*) &from, len) == -1)
              continue;

              //handle data
              std::string tmp = buf+2;
              c(buf[2], tmp, *this);
           }
                                     });
        listening_thread.detach();
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


    ~connection()
    {
      close(socket_fd);
      listening = false;
    }
  
    private:
    struct sockaddr_in server_socket;
    inet_sockaddr_t client_socket[connect_num];

    int socket_fd;
        //port;

    std::atomic_bool listening;
  };

  template<protocol p, unsigned connect_num>
  struct connection<type::CLIENT, p, connect_num>
  {

    typedef callback_<type::CLIENT,p,connect_num> callback;

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

    ~connection()
    {}
  
    private:
    inet_sockaddr_t server;
    int socket_fd;
    struct hostent *hp;
    bool listening;
  };

} //!net
  
#endif
