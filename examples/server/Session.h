//
// Copyright 2018 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <memory>
#include <vector>

#include <boost/array.hpp>

#include "common/boost_wrap.h"

#include "cc_demo1/Message.h"
#include "cc_demo1/input/ServerInputMessages.h"
#include "cc_demo1/frame/Frame.h"

namespace cc_demo1
{

namespace server
{

class Session
{
public:
    using Socket = boost::asio::ip::tcp::socket;
    using TermCallback = std::function<void ()>;

    explicit Session(common::boost_wrap::io& io, Socket&& sock) 
      : m_io(io),
        m_socket(std::move(sock)),
        m_remote(m_socket.remote_endpoint()) 
    {
    };

    template <typename TFunc>
    void setTerminateCallback(TFunc&& func)
    {
        m_termCb = std::forward<TFunc>(func);
    }

    void start();

    using InputMsg = 
        cc_demo1::Message<
            comms::option::ReadIterator<const std::uint8_t*>,
            comms::option::Handler<Session>,
            comms::option::NameInterface
        >;

    CC_DEMO1_ALIASES_FOR_SERVER_INPUT_MESSAGES_DEFAULT_OPTIONS(In,,InputMsg);

    void handle(InSimpleInts& msg);
    void handle(InScaledInts& msg);
    void handle(InFloats& msg);
    void handle(InEnums& msg);
    void handle(InSets& msg);
    void handle(InBitfields& msg);
    void handle(InStrings& msg);
    void handle(InDatas& msg);
    void handle(InLists& msg);
    void handle(InOptionals& msg);
    void handle(InVariants& msg);
    void handle(InputMsg&);

private:

    using OutputMsg = 
        cc_demo1::Message<
            comms::option::WriteIterator<std::back_insert_iterator<std::vector<std::uint8_t> > >,
            comms::option::LengthInfoInterface,
            comms::option::IdInfoInterface
        >;

    using AllInputMessages = cc_demo1::input::ServerInputMessages<InputMsg>;

    using Frame = cc_demo1::frame::Frame<InputMsg, AllInputMessages>;

    void terminateSession();
    void processInput();
    void sendAck(cc_demo1::MsgId id);

    common::boost_wrap::io& m_io;
    Socket m_socket;
    TermCallback m_termCb;    
    boost::array<std::uint8_t, 1024> m_readBuf;
    std::vector<std::uint8_t> m_inputBuf;
    Frame m_frame;
    Socket::endpoint_type m_remote;
}; 

using SessionPtr = std::unique_ptr<Session>;

} // namespace server

} // namespace cc_demo1
