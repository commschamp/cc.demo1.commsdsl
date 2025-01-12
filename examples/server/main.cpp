//
// Copyright 2018 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <iostream>
#include <stdexcept>
#include <csignal>

#include "ProgramOptions.h"
#include "Server.h"

int main(int argc, const char* argv[])
{
    try {
        cc_demo1::server::ProgramOptions options;
        options.parse(argc, argv);
        if (options.helpRequested()) {
            std::cout << "Usage:\n\t" << argv[0] << " [OPTIONS]\n";
            options.printHelp(std::cout);
            return 0;
        }

        cc_demo1::common::boost_wrap::io io;

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

        cc_demo1::server::Server server(io, options.port());
        if (!server.start()) {
            return -1;
        }

        io.run();
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: Unexpected exception: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}