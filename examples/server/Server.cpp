#include "Server.h"

#include <iostream>

namespace demo1
{

namespace server
{

Server::Server(common::boost_wrap::io& io, std::uint16_t port)    
  : m_acceptor(io),
    m_socket(io),
    m_port(port)
{
}

bool Server::start()
{
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), m_port);
    boost::system::error_code ec;
    m_acceptor.open(endpoint.protocol(), ec);
    if (ec) {
        std::cerr << "Failed to open acceptor on port " << m_port << " with error: " << ec.message() << std::endl;
        return false;
    }

    m_acceptor.bind(
        boost::asio::ip::tcp::endpoint(
            boost::asio::ip::tcp::v4(),
            m_port
        ),
        ec);

    if (ec) {
        std::cerr << "Failed to bind port " << m_port << " with error: " << ec.message() << std::endl;
        return false;
    }

    m_acceptor.listen(Socket::max_connections, ec);
    if (ec) {
        std::cerr << "Failed to listen on port " << m_port << " with error: " << ec.message() << std::endl;
        return false;
    }    

    acceptNewConnection();
    return true;
}

void Server::acceptNewConnection()
{
    m_acceptor.async_accept(
        m_socket,
        [this](const boost::system::error_code& ec2)
        {
            if (ec2) {
                std::cerr << "WARNING: failed to accept new connection with error: " << 
                    ec2.message() << std::endl;
                acceptNewConnection();
                return;
            }

            std::cerr << "New connection from " << m_socket.remote_endpoint() << std::endl;
            SessionPtr newSession(new Session(std::move(m_socket)));
            auto* sessionPtr = newSession.get();
            newSession->setTerminateCallback(
                [this, sessionPtr]()
                {
                    auto iter = 
                        std::find_if(
                            m_sessions.begin(), m_sessions.end(),
                            [sessionPtr](auto& s)
                            {
                                return s.get() == sessionPtr;
                            });

                    if (iter == m_sessions.end()) {
                        assert(!"Should not happen");
                        return;
                    }

                    m_sessions.erase(iter);
                });

            newSession->start();
            m_sessions.push_back(std::move(newSession));
            acceptNewConnection();
        });

}

} // namespace server

} // namespace demo1
