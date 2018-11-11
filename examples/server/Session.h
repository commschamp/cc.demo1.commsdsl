#pragma once

#include <functional>
#include <memory>
#include <vector>

#include <boost/asio.hpp>
#include <boost/array.hpp>

#include "demo1/Message.h"
#include "demo1/ServerInputMessages.h"
#include "demo1/frame/Frame.h"

namespace demo1
{

namespace server
{

class Session
{
public:
    using Socket = boost::asio::ip::tcp::socket;
    using TermCallback = std::function<void ()>;

    explicit Session(Socket&& sock) 
      : m_socket(std::move(sock)),
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
        demo1::Message<
            comms::option::ReadIterator<const std::uint8_t*>,
            comms::option::Handler<Session>,
            comms::option::NameInterface
        >;

    using InSimpleInts = demo1::message::SimpleInts<InputMsg>;
    using InScaledInts = demo1::message::ScaledInts<InputMsg>;
    using InFloats = demo1::message::Floats<InputMsg>;
    using InEnums = demo1::message::Enums<InputMsg>;
    using InSets = demo1::message::Sets<InputMsg>;
    using InBitfields = demo1::message::Bitfields<InputMsg>;
    using InStrings = demo1::message::Strings<InputMsg>;
    using InDatas = demo1::message::Datas<InputMsg>;
    using InLists = demo1::message::Lists<InputMsg>;

    void handle(InSimpleInts& msg);
    void handle(InScaledInts& msg);
    void handle(InFloats& msg);
    void handle(InEnums& msg);
    void handle(InSets& msg);
    void handle(InBitfields& msg);
    void handle(InStrings& msg);
    void handle(InDatas& msg);
    void handle(InLists& msg);
    void handle(InputMsg&);

private:

    using OutputMsg = 
        demo1::Message<
            comms::option::WriteIterator<std::back_insert_iterator<std::vector<std::uint8_t> > >,
            comms::option::LengthInfoInterface,
            comms::option::IdInfoInterface
        >;

    using AllInputMessages = demo1::ServerInputMessages<InputMsg>;

    using Frame = demo1::frame::Frame<InputMsg, AllInputMessages>;

    void terminateSession();
    void processInput();
    void sendAck(demo1::MsgId id);

    Socket m_socket;
    TermCallback m_termCb;    
    boost::array<std::uint8_t, 1024> m_readBuf;
    std::vector<std::uint8_t> m_inputBuf;
    Frame m_frame;
    Socket::endpoint_type m_remote;
}; 

using SessionPtr = std::unique_ptr<Session>;

} // namespace server

} // namespace demo1