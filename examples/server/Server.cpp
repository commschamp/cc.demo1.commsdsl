//
// Copyright 2018 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Server.h"

#include <algorithm>
#include <iostream>

namespace cc_demo1
{

namespace server
{

Server::Server(common::boost_wrap::io& io, std::uint16_t port)    
  : m_io(io),
    m_acceptor(io),
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

    m_acceptor.listen(Socket::max_listen_connections, ec);
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
            SessionPtr newSession(new Session(m_io, std::move(m_socket)));
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
                        static constexpr bool Should_not_happen = false;
                        static_cast<void>(Should_not_happen);
                        assert(Should_not_happen);
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

} // namespace cc_demo1
