#include "Session.h"

#include <iostream>
#include <iomanip>
#include <iterator>

#include "demo1/message/Ack.h"
#include "comms/units.h"

namespace demo1
{

namespace server
{

namespace
{

void printRawData(const std::vector<std::uint8_t>& data)
{
    std::cout << std::hex;
    std::copy(data.begin(), data.end(), std::ostream_iterator<unsigned>(std::cout, " "));
    std::cout << std::dec << '\n';
}

} // namespace

void Session::start()
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
                terminateSession();
                return;
            }

            std::cout << "<-- " << std::hex;
            std::copy_n(m_readBuf.begin(), bytesCount, std::ostream_iterator<unsigned>(std::cout, " "));
            std::cout << std::dec << std::endl;

            m_inputBuf.insert(m_inputBuf.end(), m_readBuf.begin(), m_readBuf.begin() + bytesCount);
            processInput();
            start();            
        });
}

void Session::handle (InSimpleInts& msg)
{
    std::cout << 
        "\tF1 = " << (int)msg.field_f1().value() << '\n' <<
        "\tF2 = " << (unsigned)msg.field_f2().value() << '\n' <<
        "\tF3 = " << msg.field_f3().value() << '\n' <<
        "\tF4 = " << msg.field_f4().value() << '\n' <<
        "\tF5 = " << msg.field_f5().value() << '\n' <<
        "\tF6 = " << msg.field_f6().value() << '\n' <<
        "\tF7 = " << msg.field_f7().value() << '\n' <<
        "\tF8 = " << msg.field_f8().value() << '\n' <<
        "\tF9 = " << msg.field_f9().value() << '\n' <<
        "\tF10 = " << msg.field_f10().value() << '\n' <<
        std::endl;
    sendAck(msg.doGetId());
}

void Session::handle (InScaledInts& msg)
{
    std::cout << 
        '\t' << msg.field_lat().name() << ": "
            "raw = " << msg.field_lat().value() << 
            "; deg = " << comms::units::getDegrees<float>(msg.field_lat()) <<
            "; rad = " << comms::units::getRadians<float>(msg.field_lat()) << '\n' <<
        '\t' << msg.field_lon().name() << ": " 
            "raw = " << msg.field_lon().value() << 
            "; deg = " << comms::units::getDegrees<float>(msg.field_lon()) <<
            "; rad = " << comms::units::getRadians<float>(msg.field_lon()) << '\n' <<
        '\t' << msg.field_height().name() << ": " 
            "raw = " << msg.field_height().value() << 
            "; mm = " << comms::units::getMillimeters<float>(msg.field_height()) <<
            "; cm = " << comms::units::getCentimeters<float>(msg.field_height()) << 
            "; m = " << comms::units::getMeters<float>(msg.field_height()) << '\n' <<
        '\t' << msg.field_someScaledVal().name() << ": " 
            "raw = " << msg.field_someScaledVal().value() << 
            "; scaled = " << msg.field_someScaledVal().getScaled<float>() << '\n' <<            
        std::endl;
    sendAck(msg.doGetId());
}

void Session::handle(InFloats& msg)
{
    std::cout << 
        '\t' << msg.field_timeout().name() << ": " << msg.field_timeout().value() << '\n' <<
        '\t' << msg.field_distance().name() << ": " 
            "raw = " << msg.field_distance().value() << 
            "; mm = " << comms::units::getMillimeters<float>(msg.field_distance()) <<
            "; cm = " << comms::units::getCentimeters<float>(msg.field_distance()) << 
            "; m = " << comms::units::getMeters<float>(msg.field_distance()) << '\n' <<
        std::endl;
    sendAck(msg.doGetId());
}

void Session::handle(InEnums& msg)
{
    std::cout << 
        '\t' << msg.field_f1().name() << " = " << (unsigned)msg.field_f1().value() << '\n' <<
        '\t' << msg.field_f2().name() << " = " << (int)msg.field_f2().value() << '\n' <<
        '\t' << msg.field_f3().name() << " = 0x" << std::hex  << (unsigned)msg.field_f3().value() << std::dec << '\n' <<
        '\t' << msg.field_f4().name() << " = " << (unsigned)msg.field_f4().value() << '\n' <<
        std::endl;
    sendAck(msg.doGetId());
}

void Session::handle(InSets& msg)
{
    std::cout << std::hex <<
        '\t' << msg.field_f1().name() << " = 0x" << (unsigned)msg.field_f1().value() << 
            " (valid = " << std::boolalpha << msg.field_f1().valid() << ")\n" <<
        '\t' << msg.field_f2().name() << " = 0x" << msg.field_f2().value() << 
            " (valid = " << std::boolalpha << msg.field_f2().valid() << ")\n" <<
        '\t' << msg.field_f3().name() << " = 0x" << msg.field_f3().value() << 
            " (valid = " << std::boolalpha << msg.field_f3().valid() << ")\n" <<
        std::endl;
    sendAck(msg.doGetId());
}

void Session::handle(InBitfields& msg)
{
    std::cout << 
        '\t' << msg.field_f1().name() << ":\n" <<
        "\t\t" << msg.field_f1().field_mem1().name() << " = "  << 
            (unsigned)msg.field_f1().field_mem1().value() << '\n' << 
        "\t\t" << msg.field_f1().field_mem2().name() << std::hex << 
            " = 0x"  << msg.field_f1().field_mem2().value() << std::dec << '\n' << 
        "\t\t" << msg.field_f1().field_mem3().name() << " = " << (unsigned)msg.field_f1().field_mem3().value() << '\n' << 
        std::endl;
    sendAck(msg.doGetId());
}

void Session::handle(InStrings& msg)
{
    std::cout << 
        '\t' << msg.field_f1().name() << " = " << msg.field_f1().value() << '\n' <<
        '\t' << msg.field_f2().name() << " = " << msg.field_f2().value() << '\n' <<
        '\t' << msg.field_f3().name() << " = " << msg.field_f3().value() << '\n' <<
        '\t' << msg.field_f4().name() << " = " << msg.field_f4().value() << '\n' <<
        '\t' << msg.field_f5().name() << " = " << msg.field_f5().value() << '\n' <<
        std::endl;
    sendAck(msg.doGetId());
}

void Session::handle(InDatas& msg)
{
    std::cout << '\t' << msg.field_f1().name() << " = ";
    printRawData(msg.field_f1().value());
    std::cout << '\t' << msg.field_f2().name() << " = ";
    printRawData(msg.field_f2().value());
    std::cout << '\t' << msg.field_f3().name() << " = ";
    printRawData(msg.field_f3().value());
    std::cout << '\t' << msg.field_f4().name() << " = ";
    printRawData(msg.field_f4().value());
    std::cout << std::endl;
    sendAck(msg.doGetId());
}


void Session::handle(InputMsg&)
{
    std::cerr << "WARNING: Unexpected message received" << std::endl;
}

void Session::terminateSession()
{
    std::cout << "Terminating session to " << m_remote << std::endl;
    if (m_termCb) {
        m_socket.get_io_service().post(
            [this]()
            {
                m_termCb();
            });
    }
}

void Session::processInput()
{
    std::size_t consumed = 0U;
    while (consumed < m_inputBuf.size()) {
        // Smart pointer to the message object.
        Frame::MsgPtr msgPtr; 

        // Get the iterator for reading
        auto begIter = comms::readIteratorFor<InputMsg>(&m_inputBuf[0] + consumed);
        auto iter = begIter;

        // Do the read
        auto es = m_frame.read(msgPtr, iter, m_inputBuf.size() - consumed);
        if (es == comms::ErrorStatus::NotEnoughData) {
            break; // Not enough data in the buffer, stop processing
        } 
    
        if (es == comms::ErrorStatus::ProtocolError) {
            // Something is not right with the data, remove one character and try again
            std::cerr << "WARNING: Corrupted buffer" << std::endl;
            ++consumed;
            continue;
        }

        if (es == comms::ErrorStatus::Success) {
            assert(msgPtr); // If read is successful, msgPtr is expected to hold a valid pointer
            std::cout << "INFO: New message: " << msgPtr->name() << std::endl;
            msgPtr->dispatch(*this); // Call appropriate handle() function
        }

        // The iterator for reading has been advanced, update the difference
        consumed += std::distance(begIter, iter);
    }

    m_inputBuf.erase(m_inputBuf.begin(), m_inputBuf.begin() + consumed);
}

void Session::sendAck(demo1::MsgId id)
{
    demo1::message::Ack<OutputMsg> msg;
    msg.field_msgId().value() = id;

    std::vector<std::uint8_t> outputBuf;
    outputBuf.reserve(m_frame.length(msg));
    auto iter = std::back_inserter(outputBuf);
    auto es = m_frame.write(msg, iter, outputBuf.max_size());
    if (es == comms::ErrorStatus::UpdateRequired) {
        auto updateIter = &outputBuf[0];
        es = m_frame.update(updateIter, outputBuf.size());
    }

    if (es != comms::ErrorStatus::Success) {
        assert(!"Unexpected error");
        return;
    }

    std::cout << "INFO: Sending Ack back\n";
    std::cout << "--> " << std::hex;
    std::copy(outputBuf.begin(), outputBuf.end(), std::ostream_iterator<unsigned>(std::cout, " "));
    std::cout << std::dec << std::endl;
    m_socket.send(boost::asio::buffer(outputBuf));
}

} // namespace server

} // namespace demo1
