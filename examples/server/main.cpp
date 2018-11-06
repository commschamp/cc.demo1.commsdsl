#include <iostream>
#include <stdexcept>
#include <csignal>

#include "ProgramOptions.h"
#include "Server.h"

int main(int argc, const char* argv[])
{
    try {
        demo1::server::ProgramOptions options;
        options.parse(argc, argv);
        if (options.helpRequested()) {
            std::cout << "Usage:\n\t" << argv[0] << " [OPTIONS]\n";
            options.printHelp(std::cout);
            return 0;
        }

        boost::asio::io_service io;

        boost::asio::signal_set signals(io, SIGINT, SIGTERM);
        signals.async_wait(
            [&io](const boost::system::error_code& ec, int signum)
            {
                io.stop();
                if (ec) {
                    std::cerr << "ERROR: " << ec.message() << std::endl;
                    return;
                }

                std::cerr << "Termination due to signal " << signum << std::endl;
            });

        demo1::server::Server server(io, options.port());
        if (!server.start()) {
            return -1;
        }

        std::cout << "Hello" << std::endl;
        io.run();
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: Unexpected exception: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}