//
// Copyright 2018 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <list>

#include "common/boost_wrap.h"

#include "Session.h"

namespace cc_demo1
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

    common::boost_wrap::io& m_io;
    Acceptor m_acceptor;
    Socket m_socket;
    std::uint16_t m_port = 0U;
    std::list<SessionPtr> m_sessions;
};

} // namespace server

} // namespace cc_demo1
