#include "Client.h"

#include <cmath>
#include <iostream>
#include <map>
#include <sstream>

#include "cc_demo1/MsgId.h"
#include "comms/units.h"
#include "comms/process.h"

namespace cc_demo1
{

namespace client
{

Client::Client(
        common::boost_wrap::io& io, 
        const std::string& server,
        std::uint16_t port)    
  : m_io(io),
    m_socket(io),
    m_timer(io),
    m_server(server),
    m_port(port)
{
    if (m_server.empty()) {
        m_server = "localhost";
    }
}

bool Client::start()
{
    boost::asio::ip::tcp::resolver resolver(m_io);
    auto query = boost::asio::ip::tcp::resolver::query(m_server, std::to_string(m_port));

    boost::system::error_code ec;
    auto iter = resolver.resolve(query, ec);
    if (ec) {
        std::cerr << "ERROR: Failed to resolve \"" << m_server << ':' << m_port << "\" " <<
            "with error: " << ec.message() << std::endl; 
        return false;
    }

    auto endpoint = iter->endpoint();
    m_socket.connect(endpoint, ec);
    if (ec) {
        std::cerr << "ERROR: Failed to connect to \"" << endpoint << "\" " <<
            "with error: " << ec.message() << std::endl; 
        return false;
    }

    std::cout << "INFO: Connected to " << m_socket.remote_endpoint() << std::endl;
    readDataFromServer();
    readDataFromStdin();
    return true;
}

void Client::handle(InAckMsg& msg)
{
    if (msg.field_msgId().value() != m_sentId) {
        std::cerr << "WARNING: Ack for the wrong ID: " << 
            static_cast<unsigned>(msg.field_msgId().value()) << std::endl;
        return;
    }

    std::cout << "INFO: Message acknowledged" << std::endl;
    m_timer.cancel();
    readDataFromStdin();
}

void Client::handle(InputMsg&)
{
    std::cerr << "WARNING: Unexpected message received" << std::endl;
}

void Client::readDataFromServer()
{
    m_socket.async_read_some(
        boost::asio::buffer(m_readBuf),
        [this](const boost::system::error_code& ec, std::size_t bytesCount)
        {
            if (ec == boost::asio::error::operation_aborted) {
                return;
            }

            if (ec) {
                std::cerr << "ERROR: Failed to read with error: " << ec.message() << std::endl;
                m_io.stop();
                return;
            }

            std::cout << "<-- " << std::hex;
            std::copy_n(m_readBuf.begin(), bytesCount, std::ostream_iterator<unsigned>(std::cout, " "));
            std::cout << std::dec << std::endl;

            m_inputBuf.insert(m_inputBuf.end(), m_readBuf.begin(), m_readBuf.begin() + bytesCount);
            processInput();
            readDataFromServer();            
        });
}

void Client::readDataFromStdin()
{
    std::cout << "\nEnter (new) message ID to send: " << std::endl;
    m_sentId = cc_demo1::MsgId_Ack;
    do {
        // Unfortunatelly Windows doesn't provide an easy way to 
        // asynchronously read from stdin with boost::asio,
        // read synchronously. DON'T COPY-PASTE TO PRODUCTION CODE!!!
        unsigned msgId = 0U;
        std::cin >> msgId;
        if (!std::cin.good()) {
            std::cerr << "ERROR: Unexpected input, use numeric value" << std::endl;
            std::cin.clear();
            std::cin.ignore();
            break;
        }

        using SendFunc = void (Client::*)();
        static const std::map<unsigned, SendFunc> Map = {
            std::make_pair(cc_demo1::MsgId_SimpleInts, &Client::sendSimpleInts),
            std::make_pair(cc_demo1::MsgId_ScaledInts, &Client::sendScaledInts),
            std::make_pair(cc_demo1::MsgId_Floats, &Client::sendFloats),
            std::make_pair(cc_demo1::MsgId_Enums, &Client::sendEnums),
            std::make_pair(cc_demo1::MsgId_Sets, &Client::sendSets),
            std::make_pair(cc_demo1::MsgId_Bitfields, &Client::sendBitfields),
            std::make_pair(cc_demo1::MsgId_Strings, &Client::sendStrings),
            std::make_pair(cc_demo1::MsgId_Datas, &Client::sendDatas),
            std::make_pair(cc_demo1::MsgId_Lists, &Client::sendLists),
            std::make_pair(cc_demo1::MsgId_Optionals, &Client::sendOptionals),
            std::make_pair(cc_demo1::MsgId_Variants, &Client::sendVariants),
        };

        auto iter = Map.find(msgId);
        if (iter == Map.end()) {
            std::cerr << "ERROR: Unknown message ID, try another one" << std::endl;
            break;
        }

        auto func = iter->second;
        (this->*func)();
        return; // Don't read STDIN right away, wait for ACK first
    } while (false);

    common::boost_wrap::post(
        m_io,
        [this]()
        {
            readDataFromStdin();
        });
}

void Client::sendSimpleInts()
{
    cc_demo1::message::SimpleInts<OutputMsg> msg;
    msg.field_f9().value() = 12345;
    msg.field_f10().value() = 678910;
    // Keep default value of other fields
    sendMessage(msg);
}

void Client::sendScaledInts()
{
    cc_demo1::message::ScaledInts<OutputMsg> msg;
    comms::units::setDegrees(msg.field_lat(), -27.470125);
    comms::units::setDegrees(msg.field_lon(), 153.021072);
    comms::units::setMeters(msg.field_height(), 80.123);
    msg.field_someScaledVal().setScaled(123.45);
    sendMessage(msg);
}

void Client::sendFloats()
{
    cc_demo1::message::Floats<OutputMsg> msg;
    msg.field_timeout().setInvalid();
    assert(std::isnan(msg.field_timeout().value()));
    comms::units::setCentimeters(msg.field_distance(), 34.56);
    sendMessage(msg);
}

void Client::sendEnums()
{
    using OutMsg = cc_demo1::message::Enums<OutputMsg>;
    OutMsg msg;
    msg.field_f1().value() = OutMsg::Field_f1::ValueType::V2;
    msg.field_f4().value() = OutMsg::Field_f4::ValueType::V2;
    // Keep default value of other fields
    sendMessage(msg);
}

void Client::sendSets()
{
    cc_demo1::message::Sets<OutputMsg> msg;
    msg.field_f1().setBitValue_Bit2(true);
    msg.field_f2().setBitValue_Bit0(true);
    msg.field_f3().value() = 0x7; // sets also reserved bit 1
    // Keep default value of other bits
    sendMessage(msg);
}

void Client::sendBitfields()
{
    using OutMsg = cc_demo1::message::Bitfields<OutputMsg>;
    OutMsg msg;
    msg.field_f1().field_mem1().value() = 5;
    msg.field_f1().field_mem2().setBitValue_Bit2(true);
    msg.field_f1().field_mem3().value() = OutMsg::Field_f1::Field_mem3::ValueType::V2;
    sendMessage(msg);
}

void Client::sendStrings()
{
    cc_demo1::message::Strings<OutputMsg> msg;
    msg.field_f1().value() = "bla";
    msg.field_f3().value() = "str";
    msg.field_f4().value() = "oooo";
    msg.field_f5().value() = "aa";
    // Keep default value of other fields
    msg.doRefresh(); // Bring message into consistent state, i.e. update F4Len
    sendMessage(msg);
}

void Client::sendDatas()
{
    cc_demo1::message::Datas<OutputMsg> msg;
    msg.field_f1().value() = {0x10, 0x20, 0x30};
    msg.field_f3().value() = {0x01, 0x02, 0x03};
    msg.field_f4().value() = {0x06, 0x07};
    // Keep default value of other fields
    msg.doRefresh(); // Bring message into consistent state, i.e. update F3Len
    sendMessage(msg);
}

void Client::sendLists()
{
    cc_demo1::message::Lists<OutputMsg> msg;
    msg.field_f1().value().resize(2);
    msg.field_f1().value()[0].value() = 0x11223344;
    msg.field_f1().value()[1].value() = 0xaabbccdd;

    msg.field_f2().value().resize(1);
    msg.field_f2().value()[0].value() = 0xffffffff;

    msg.field_f3().value().resize(1);
    msg.field_f3().value()[0].field_mem1().value() = 5;    
    msg.field_f3().value()[0].field_mem2().value() = -5;    

    msg.field_f4().value().resize(1);
    msg.field_f4().value()[0].field_mem1().value() = 7;    
    msg.field_f4().value()[0].field_mem2().value() = "hello";      

    msg.field_f5().value().resize(2);
    msg.field_f5().value()[0].field_mem1().value() = 10;    
    msg.field_f5().value()[0].field_mem2().value() = -10;     
    msg.field_f5().value()[1].field_mem1().value() = 15;    
    msg.field_f5().value()[1].field_mem2().value() = -15;     

    msg.doRefresh(); // Bring message into consistent state, i.e. update F2Len
    sendMessage(msg);
}

void Client::sendOptionals()
{
    cc_demo1::message::Optionals<OutputMsg> msg;
    // Note usage of .field() to get access to inner field of optional
    msg.field_f2().field().value() = 0xaaaa; 
    msg.field_f3().field().value() = 0xbbbb;

    msg.field_flags().setBitValue_F2Exists(true);
    msg.field_flags().setBitValue_F3Missing(true);
    msg.doRefresh(); // Bring message into consistent state, i.e. update optional modes
    sendMessage(msg);
}

void Client::sendVariants()
{
    cc_demo1::message::Variants<OutputMsg> msg;
    auto& props1Vec = msg.field_props1().value();
    props1Vec.resize(3U);
    props1Vec[0].initField_prop1().field_val().value() = 1234;
    props1Vec[1].initField_prop3().field_val().value() = "hello";
    props1Vec[2].initField_prop2().field_val().value() = 5555555;
    
    auto& props2Vec = msg.field_props2().value();
    props2Vec.resize(3U);
    props2Vec[0].initField_prop1().field_val().value() = 4321;
    props2Vec[1].initField_prop3().field_val().value() = "blabla";
    props2Vec[2].initField_prop2().field_val().value() = 88888;
    msg.doRefresh(); // Bring all the lengths into consistent state
    
    sendMessage(msg);
}

void Client::sendMessage(const OutputMsg& msg)
{
    std::vector<std::uint8_t> outputBuf;
    outputBuf.reserve(m_frame.length(msg));
    auto iter = std::back_inserter(outputBuf);
    auto es = m_frame.write(msg, iter, outputBuf.max_size());
    if (es == comms::ErrorStatus::UpdateRequired) {
        auto updateIter = &outputBuf[0];
        es = m_frame.update(updateIter, outputBuf.size());
    }

    if (es != comms::ErrorStatus::Success) {
        static constexpr bool Unexpected_error = false;
        static_cast<void>(Unexpected_error);
        assert(Unexpected_error);
        return;
    }

    std::cout << "INFO: Sending message: " << msg.name() << '\n';
    std::cout << "--> " << std::hex;
    std::copy(outputBuf.begin(), outputBuf.end(), std::ostream_iterator<unsigned>(std::cout, " "));
    std::cout << std::dec << std::endl;
    m_socket.send(boost::asio::buffer(outputBuf));

    m_sentId = msg.getId();
    waitForAck();
}

void Client::waitForAck()
{
    m_timer.expires_from_now(boost::posix_time::seconds(2));
    m_timer.async_wait(
        [this](const boost::system::error_code& ec)
        {
            if (ec == boost::asio::error::operation_aborted) {
                return;
            }

            std::cerr << "ERROR: Previous message hasn't been acknowledged" << std::endl;
            readDataFromStdin();
        });
}

void Client::processInput()
{
    if (!m_inputBuf.empty()) {
        auto consumed = comms::processAllWithDispatch(&m_inputBuf[0], m_inputBuf.size(), m_frame, *this);
        m_inputBuf.erase(m_inputBuf.begin(), m_inputBuf.begin() + consumed);
    }
}

} // namespace client

} // namespace cc_demo1
