#include "../communication.hh"

#include <chrono>
#include <thread>

typedef net::communication<net::role::HOST, net::protocol::PIPE> unix_host_communication;
typedef net::communication<net::role::CLIENT, net::protocol::PIPE> unix_client_communication;

typedef net::communication<net::role::HOST, net::protocol::UDP> udp_host_communication;
typedef net::communication<net::role::CLIENT, net::protocol::UDP> udp_client_communication;

typedef net::communication<net::role::HOST, net::protocol::TCP> tcp_host_communication;
typedef net::communication<net::role::CLIENT, net::protocol::TCP> tcp_client_communication;

void tcp_serv_callback(int, const std::string& s, tcp_host_communication &c)
{
  std::cout<<s<<std::endl;
}

void tcp_clie_callback(int, const std::string& s, tcp_client_communication &c)
{
  std::cout<<s<<std::endl;
}
void udp_serv_callback(int, const std::string& s, udp_host_communication &c)
{
  std::cout<<s<<std::endl;
}

void udp_clie_callback(int, const std::string& s, udp_client_communication &c)
{
  std::cout<<s<<std::endl;
}
void unix_serv_callback(int, const std::string& s, unix_host_communication &c)
{
  std::cout<<s<<std::endl;
}

void unix_clie_callback(int, const std::string& s, unix_client_communication &c)
{
  std::cout<<s<<std::endl;
}

int main(int argc, const char **argv)
{
  // TCP
  {
    tcp_host_communication ch(1026, tcp_serv_callback);
    tcp_client_communication cc("localhost", 1026, tcp_clie_callback);

    cc.send("12coucou");

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
  // UDP
  {
    udp_host_communication ch(1026, udp_serv_callback);
    udp_client_communication cc("localhost", 1026, udp_clie_callback);

    cc.send("12coucou");

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
  // PIPE
  {
    unix_host_communication ch(1026, unix_serv_callback);
    unix_client_communication cc(ch.get_file_path(), 1026, unix_clie_callback);

    cc.send("12coucou");

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  }
  return 0;
}
