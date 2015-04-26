#include "../communication.hh"

#include <chrono>
#include <thread>

typedef net::communication<net::role::HOST, net::protocol::PIPE> udp_host_communication;
typedef net::communication<net::role::CLIENT, net::protocol::PIPE> udp_client_communication;

void serv_callback(int, const std::string& s, udp_host_communication &c)
{
  std::cout<<s<<std::endl;
}

void clie_callback(int, const std::string& s, udp_client_communication &c)
{
  std::cout<<s<<std::endl;
}

int main(int argc, const char **argv)
{
  udp_host_communication ch(1026, serv_callback);
  //udp_client_communication cc("localhost", 1026, clie_callback);
  udp_client_communication cc(ch.get_file_path(), 1026, clie_callback);

  std::cout<<ch.get_file_path()<<std::endl;
  cc.send("12coucou");

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  return 0;
}
