#ifndef NET_MESSAGE_HH
#define NET_MESSAGE_HH

#include <string>
#include <sstream>
#include <array>

#include "global_network.hh"

namespace net
{
  struct message
  {
    typedef unsigned char byte;

    message(byte i,
            byte f,
            byte c,
            const std::string &s)
    : data  {(char)i, (char)f, (char)c}
    , id    ((byte&)data[0])
    , flag  ((byte&)data[1])
    , count ((byte&)data[2])
    , content(data.data()+header_byte_size)
    , c_str(data.data())
    , length (s.size() + header_byte_size)
    {
      std::copy(s.begin(), s.end(), data.data()+header_byte_size);
      data[s.length()+header_byte_size] = 0;
    }

    message(const std::string &s)
    : data {}
    , id   ((byte&)data[0])
    , flag ((byte&)data[1])
    , count((byte&)data[2])
    , content(data.data()+header_byte_size)
    , c_str(data.data())
    , length (s.size() + header_byte_size)
    {
      std::copy(s.begin(), s.end(), data.data());
    }

    message(const char s[max_message_length])
    : data ()
    , id   ((byte&)data[0])
    , flag ((byte&)data[1])
    , count((byte&)data[2])
    , content(data.data()+header_byte_size)
    , c_str(data.data())
    , length (max_message_length)
    {
      for (size_t i = 0; i < max_message_length; ++i)
      {
        data[i] = s[i];
      }   
    }

    private:
      std::array<char, max_message_length> data;
    public:
      const byte &id;
      const byte &flag;
      const byte &count;
      const char *content;
      const char *c_str;
      const size_t length;

    friend std::ostream& operator<<(std::ostream&,const message&);
  };

  std::ostream& operator<<(std::ostream &o, const message &m)
  {
    o << m.data.data();
  }

} //!net

#endif
