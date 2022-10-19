#pragma once

#include <cstdint>
#include <string>
#include <iterator>
#include <vector>

#include <boost/array.hpp>

#include "common/boost_wrap.h"

#include "cc_demo1/Message.h"
#include "cc_demo1/input/ClientInputMessages.h"
#include "cc_demo1/frame/Frame.h"

namespace cc_demo1
{

namespace client    
{

class Client
{
public:
    Client(common::boost_wrap::io& io, const std::string& server, std::uint16_t port);

    bool start();

    using InputMsg = 
        cc_demo1::Message<
            comms::option::ReadIterator<const std::uint8_t*>,
            comms::option::Handler<Client> 
        >;

    using InAckMsg = cc_demo1::message::Ack<InputMsg>;
    
    void handle(InAckMsg& msg);
    void handle(InputMsg&);

private:
    using Socket = boost::asio::ip::tcp::socket;

    using OutputMsg = 
        cc_demo1::Message<
            comms::option::WriteIterator<std::back_insert_iterator<std::vector<std::uint8_t> > >,
            comms::option::LengthInfoInterface,
            comms::option::IdInfoInterface,
            comms::option::NameInterface
        >;

    using AllInputMessages = cc_demo1::input::ClientInputMessages<InputMsg>;

    using Frame = cc_demo1::frame::Frame<InputMsg, AllInputMessages>;


    void readDataFromServer();
    void readDataFromStdin();
    void sendSimpleInts();
    void sendScaledInts();
    void sendFloats();
    void sendEnums();
    void sendSets();
    void sendBitfields();
    void sendStrings();
    void sendDatas();
    void sendLists();
    void sendOptionals();
    void sendVariants();
    void sendMessage(const OutputMsg& msg);
    void waitForAck();
    void processInput();

    common::boost_wrap::io& m_io;
    Socket m_socket;
    boost::asio::deadline_timer m_timer;
    std::string m_server;
    std::uint16_t m_port = 0U;
    Frame m_frame;
    cc_demo1::MsgId m_sentId = cc_demo1::MsgId_Ack;
    boost::array<std::uint8_t, 32> m_readBuf;
    std::vector<std::uint8_t> m_inputBuf;
};

} // namespace client

} // namespace cc_demo1
