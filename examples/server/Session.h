#pragma once

#include <functional>
#include <memory>
#include <array>
#include <vector>

#include <boost/asio.hpp>

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

    explicit Session(Socket&& sock) : m_socket(std::move(sock)) {};

    template <typename TFunc>
    void setTerminateCallback(TFunc&& func)
    {
        m_termCb = std::forward<TFunc>(func);
    }

    void start();

    using InputMsg = 
        demo1::Message<
            comms::option::ReadIterator<const std::uint8_t*>,
            comms::option::Handler<Session> 
        >;

    
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

    Socket m_socket;
    TermCallback m_termCb;    
    std::array<std::uint8_t, 1024> m_readBuf;
    std::vector<std::uint8_t> m_inputBuf;
    Frame m_frame;
}; 

using SessionPtr = std::unique_ptr<Session>;

} // namespace server

} // namespace demo1