#ifndef NET_MESSAGE_HH
#define NET_MESSAGE_HH

#include <string>

namespace net
{
  struct message
  {
    typedef unsigned char byte;
    message(byte i,byte f, byte c, const std::string &s)
    : id(i)
    , flag(f)
    , count(c)
    , data(s)
    {}

    message(const std::string& s)
    : id(s[0]-'0')
    , flag(s[1])
    , count(s[2])
    , data(s.c_str()+3)
    { }

    operator std::string()
    {
      return std::to_string(id) + std::to_string(flag) + std::to_string(count) + data;
    }
  
    //private:
      byte id;
      byte flag;
      byte count;
      std::string data;
  };
} //!net

#endif
