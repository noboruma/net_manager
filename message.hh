#ifndef NET_MESSAGE_HH
#define NET_MESSAGE_HH

#include <string>

namespace net
{
  struct message
  {
    typedef unsigned char byte;
    message(byte,byte,std::string)
    {}

    operator std::string()
    {}
  
    private:
      byte flag;
      byte count;
      std::string data;
  };
  
} //!net

#endif
