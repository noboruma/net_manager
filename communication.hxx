namespace net 
{
  //==========================================================================
  template<protocol p>
  communication<role::HOST, p>::communication(unsigned port,
                                              const callback& c,
                                              bool ack)
  : abstract::communication()
  {
    if(p == protocol::UDP)
      socket_fd = socket(AF_INET,
                         SOCK_DGRAM,
                         IPPROTO_UDP);
    else if(p == protocol::TCP)
      socket_fd = socket(AF_INET,
                         SOCK_STREAM,
                         0);

    if(socket_fd < 0) throw std::logic_error("Socket creation failed");

    bzero((char*) &server_socket, sizeof(server_socket));

    server_socket.sin_family = AF_INET;
    server_socket.sin_addr.s_addr = htonl(INADDR_ANY);
    server_socket.sin_port = htons(port);

    if(bind(socket_fd,
            (sockaddr*)&server_socket,
            sizeof(server_socket)))
      throw std::logic_error("Socket bind failed");

    if(p == protocol::TCP)
    {
      listen(socket_fd, 30); // 30 communication in parallel

      std::thread listening_thread ([=] {
        unsigned addr_len = sizeof(sockaddr_in);
        sockaddr_in client;
        while(listening)
        {
          int newfd = accept(socket_fd,
                           (sockaddr*) &client,
                           &addr_len);

          client_sockets.push_back(newfd);

          std::thread discuss_thread ([=] {
            char buf[max_message_length];
            while(read(newfd, buf, max_message_length))
              c(newfd, buf, *this); 
          }); discuss_thread.detach();
        }
      }); listening_thread.detach();
    }
    else if(p == protocol::UDP)
    {
      std::thread listening_thread([=] {
      char buf[max_message_length] = {0};

      while(listening)
      {
        sockaddr_in from;
        socklen_t len = sizeof(sockaddr_in);

      if (recvfrom(socket_fd,
                   buf,
                   max_message_length,
                   0,
                   (sockaddr*) &from,
                   &len) == -1)
        continue; // or break?

      // ACK
      if(ack)
      if (sendto(socket_fd, buf, 1, 0, (sockaddr*) &from, len) == -1)
        continue;

      //handle data
      std::string tmp = buf;
      c(buf[2], tmp, *this);

      }}); listening_thread.detach();
    }
  }

  //==========================================================================
  communication<role::HOST, protocol::PIPE>::communication(unsigned port, const callback& c, std::string s)
  : abstract::communication()
  {
    socket_fd = socket(AF_UNIX,
                       SOCK_STREAM,
                       0);

    if(socket_fd < 0) throw std::logic_error("Socket creation failed");

    bzero((char*) &server_socket, sizeof(server_socket));

    server_socket.sun_family = AF_UNIX;

    if(s.empty())
      s = "/tmp/server.XXXXXX";
    else
      s += ".XXXXXX";

    file_path = new char[s.size()+1];
    for (int i = 0; i < s.size()+1; ++i)
      file_path[i] = s[i];

    close(mkstemp(file_path));
    unlink(file_path);

    strcpy(server_socket.sun_path, file_path);

    if(bind(socket_fd,
            (sockaddr*)&server_socket,
            sizeof(server_socket)))
      throw std::logic_error("Socket bind failed");

    listen(socket_fd, 30); // 30 communication in parallel

    std::thread listening_thread ([=]() {
      unsigned addr_len = sizeof(sockaddr_un);
      sockaddr_un client;
      while(listening)
      {
        int newfd = accept(socket_fd,
                           (sockaddr*) &client,
                           &addr_len);

        client_sockets.push_back(newfd);

        std::thread discuss_thread ([=]() 
                                    {
                                      char buf[max_message_length];
                                      while(read(newfd,
                                                 buf,
                                                 max_message_length))
                                          c(newfd, buf, *this);
                                    }); discuss_thread.detach();
      }
    }); listening_thread.detach();
  }

  //==========================================================================
  communication<role::HOST, protocol::PIPE>::~communication()
  {
    unlink(file_path);
    delete file_path;
  }

  //==========================================================================
  template<protocol p>
  communication<role::CLIENT,p>::communication(const std::string& addr, unsigned port, const callback& c)
  : abstract::communication()
  {
    if(p == protocol::UDP)
      socket_fd = socket(AF_INET,
                         SOCK_DGRAM,
                         0);
    else if(p == protocol::TCP)
      socket_fd = socket(AF_INET,
                         SOCK_STREAM,
                         0);


    if(socket_fd < 0) throw std::logic_error("Socket creation failed");

    bzero((char *)&server,sizeof(server));

    server.sin_family = AF_INET;
    hp = gethostbyname(addr.c_str());
    bcopy((char*)hp->h_addr,
          (char*)&server.sin_addr,
          hp->h_length);
    server.sin_port = htons(port);

    if(p == protocol::TCP)
    {
      if (connect(socket_fd,(sockaddr *) &server,sizeof(server)) < 0) 
        throw std::logic_error("Connect failed");
      std::thread([=](){
        char buf[max_message_length];
        while(listening)
            while(read(socket_fd, buf, max_message_length))
              c(socket_fd, buf, *this);
      });
    }
  }

  
  //==========================================================================
  communication<role::CLIENT, protocol::PIPE>::communication(const std::string& addr, unsigned port, const callback& c)
  : abstract::communication()
  {
    socket_fd = socket(AF_UNIX,
                       SOCK_STREAM,
                       0);

    if(socket_fd < 0) throw std::logic_error("Socket creation failed");

    bzero((char *)&server,sizeof(server));

    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, addr.c_str());

    if (connect(socket_fd,(sockaddr *) &server,sizeof(server)) < 0) 
      throw std::logic_error("Connect failed");

    std::thread([=](){
      char buf[max_message_length];
      while(listening)
          while(read(socket_fd, buf, max_message_length))
            c(socket_fd, buf, *this);
    });
  }

  template<protocol p>
  void communication<role::CLIENT,p>::send(const std::string& s)
  {
    char buffer[256];
    unsigned length=sizeof(sockaddr_in);
    if(p == protocol::UDP)
    {
      sendto(socket_fd,
             s.c_str(),
             s.length(),
             0,
             (const sockaddr*)&server,
             sizeof(sockaddr));

      sockaddr_in from;
      recvfrom(socket_fd,buffer,256,0,(sockaddr *)&from, &length);
    }
    else if(p == protocol::TCP)
    {
      if (write(socket_fd,s.c_str(),s.size()) < 0)
        throw std::logic_error("Write to socket failed"); 
    }
  }

  //==========================================================================
  //template<>
  void communication<role::CLIENT,protocol::PIPE>::send(const std::string& s)
  {
    char buffer[256];
    unsigned length=sizeof(sockaddr_in);
    if (write(socket_fd,s.c_str(),s.size()) < 0)
      throw std::logic_error("Write to socket failed"); 
  }
} //!net
