#include "../connection.hh"

#include <chrono>
#include <thread>

typedef net::connection<net::type::HOST, net::protocol::TCP> udp_host_connection;
typedef net::connection<net::type::CLIENT, net::protocol::TCP> udp_client_connection;

void serv_callback(int, const std::string& s, udp_host_connection &c)
{
  std::cout<<s<<std::endl;
}

void clie_callback(int, const std::string& s, udp_client_connection &c)
{
  std::cout<<s<<std::endl;
}

int main(int argc, const char **argv)
{
  udp_host_connection ch(1026, serv_callback);
  udp_client_connection cc("localhost", 1026, clie_callback);

  cc.send("12coucou");

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  return 0;
}
