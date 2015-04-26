#include "../connection.hh"

typedef net::connection<net::type::HOST, net::protocol::UDP> udp_host_connection;

void call_back(int, const std::string& s, udp_host_connection &c)
{
  std::cout<<s<<std::endl;
}

int main(int argc, const char **argv)
{
  udp_host_connection c(1026, call_back);

  while(true)
    ;
  return 0;
}
