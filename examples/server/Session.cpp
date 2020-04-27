#include "Session.h"

#include <iostream>
#include <iomanip>
#include <iterator>

#include "demo1/message/Ack.h"
#include "comms/units.h"
#include "comms/process.h"

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

struct VariantValueFieldPrinter
{
    template <typename TField>
    void operator()(const TField& field) const
    {
        printInternal(field, Tag<typename std::decay<decltype(field)>::type>());
    }

private:
    struct DataTag{};
    struct OneByteIntTag{};
    struct OtherTag {};

    template <typename TField>
    using Tag =
        typename std::conditional<
            comms::field::isArrayList<TField>(),
            DataTag,
            typename std::conditional<
                comms::field::isIntValue<TField>() && (sizeof(typename TField::ValueType) == 1U),
                OneByteIntTag,
                OtherTag
            >::type
        >::type;


    template <typename TField>
    static void printInternal(const TField& field, DataTag)
    {
        printRawData(field.value());
    }

    template <typename TField>
    static void printInternal(const TField& field, OneByteIntTag)
    {
        std::cout << static_cast<int>(field.value());
    }

    template <typename TField>
    static void printInternal(const TField& field, OtherTag)
    {
        std::cout << field.value();
    }
};

template <typename TField>
void printFieldValue(const TField& field)
{
    VariantValueFieldPrinter()(field);
}

struct VariantPrintHelper
{
    template <std::size_t TIdx, typename TField>
    void operator()(const TField& field) const
    {
        std::cout << "{" << (unsigned)field.field_key().value() << ", ";
        printFieldValue(field.field_val());
        std::cout << "}";
    }
};

struct Variant2PrintHelper
{
    template <std::size_t TIdx, typename TField>
    void operator()(const TField& field) const
    {
        std::cout << "{" << (unsigned)field.field_type().value() << ", " <<
            field.field_length().value() << ", ";
        printFieldValue(field.field_val());
        std::cout << "}";
    }
};

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

void Session::handle(InLists& msg)
{
    std::cout << '\t' << msg.field_f1().name() << " = {" << std::hex;
    for (auto& v : msg.field_f1().value()) {
        if (&v != &msg.field_f1().value().front()) {
            std::cout << ", ";
        }
        std::cout << "0x" << v.value();
    }
    std::cout << std::dec << "}\n";

    std::cout << '\t' << msg.field_f2().name() << " = {" << std::hex;
    for (auto& v : msg.field_f2().value()) {
        if (&v != &msg.field_f2().value().front()) {
            std::cout << ", ";
        }
        std::cout << "0x" << v.value();
    }
    std::cout << std::dec << "}\n";

    std::cout << '\t' << msg.field_f3().name() << " = {";
    for (auto& v : msg.field_f3().value()) {
        if (&v != &msg.field_f3().value().front()) {
            std::cout << ", ";
        }
        std::cout << "{" << v.field_mem1().value() << ", " << v.field_mem2().value() << "}";
    }
    std::cout << "}\n";    

    std::cout << '\t' << msg.field_f4().name() << " = {";
    for (auto& v : msg.field_f4().value()) {
        if (&v != &msg.field_f4().value().front()) {
            std::cout << ", ";
        }
        std::cout << "{" << v.field_mem1().value() << ", " << v.field_mem2().value() << "}";
    }
    std::cout << "}\n";    

    std::cout << '\t' << msg.field_f5().name() << " = {";
    for (auto& v : msg.field_f5().value()) {
        if (&v != &msg.field_f5().value().front()) {
            std::cout << ", ";
        }
        std::cout << "{" << v.field_mem1().value() << ", " << v.field_mem2().value() << "}";
    }
    std::cout << "}\n";            

    std::cout << std::endl;
    sendAck(msg.doGetId());
}

void Session::handle(InOptionals& msg)
{
    std::cout << std::hex <<
        '\t' << msg.field_flags().name() << " = 0x" << 
            (unsigned)msg.field_flags().value() << '\n' <<
        '\t' << msg.field_f2().name() << " = 0x" << msg.field_f2().field().value() << 
            " (exists = " << std::boolalpha << msg.field_f2().doesExist() << ")\n" <<
        '\t' << msg.field_f3().name() << " = 0x" << msg.field_f3().field().value() << 
            " (exists = " << std::boolalpha << msg.field_f3().doesExist() << ")\n" <<
        std::dec << std::endl;
    sendAck(msg.doGetId());
}

void Session::handle(InVariants& msg)
{
    std::cout << '\t' << msg.field_props1().name() << " = {";
    for (auto& p : msg.field_props1().value()) {
        if (&p != &msg.field_props1().value().front()) {
            std::cout << ", ";
        }

        p.currentFieldExec(VariantPrintHelper());
    }
    std::cout << "}\n" << std::endl;
    
    std::cout << '\t' << msg.field_props2().name() << " = {";
    for (auto& p : msg.field_props2().value()) {
        if (&p != &msg.field_props2().value().front()) {
            std::cout << ", ";
        }

        p.currentFieldExec(Variant2PrintHelper());
    }
    std::cout << "}\n" << std::endl;
    
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
        common::boost_wrap::post(
            m_io,        
            [this]()
            {
                m_termCb();
            });
    }
}

void Session::processInput()
{
    if (!m_inputBuf.empty()) {
        auto consumed = comms::processAllWithDispatch(&m_inputBuf[0], m_inputBuf.size(), m_frame, *this);
        m_inputBuf.erase(m_inputBuf.begin(), m_inputBuf.begin() + consumed);
    }
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
