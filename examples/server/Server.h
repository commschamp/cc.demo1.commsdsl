#pragma once

#include <cstdint>
#include <list>

#include "common/boost_wrap.h"

#include "Session.h"

namespace demo1
{

namespace server    
{

class Server
{
public:
    Server(common::boost_wrap::io& io, std::uint16_t port);

    bool start();
private:
    using Acceptor = boost::asio::ip::tcp::acceptor;
    using Socket = Session::Socket;

    void acceptNewConnection();

    Acceptor m_acceptor;
    Socket m_socket;
    std::uint16_t m_port = 0U;
    std::list<SessionPtr> m_sessions;
};

} // namespace server

} // namespace demo1
