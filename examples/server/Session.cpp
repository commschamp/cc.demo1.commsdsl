#include "Session.h"

#include <iostream>

namespace demo1
{

namespace server
{

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

            m_inputBuf.insert(m_inputBuf.end(), m_readBuf.begin(), m_readBuf.begin() + bytesCount);
            processInput();
            start();            
        });
}

void Session::handle(InputMsg&)
{
    std::cerr << "WARNING: Unexpected message received" << std::endl;
}

void Session::terminateSession()
{
    std::cout << "Terminating session to " << m_socket.remote_endpoint() << std::endl;
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

            msgPtr->dispatch(*this); // Call appropriate handle() function
        }

        // The iterator for reading has been advanced, update the difference
        consumed += std::distance(begIter, iter);
    }

    m_inputBuf.erase(m_inputBuf.begin(), m_inputBuf.begin() + consumed);
}

} // namespace server

} // namespace demo1
