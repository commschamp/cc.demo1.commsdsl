//
// Copyright 2018 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <string>

#include <boost/program_options.hpp>

namespace cc_demo1
{

namespace client
{

class ProgramOptions
{
public:
    void parse(int argc, const char* argv[]);
    static void printHelp(std::ostream& out);

    bool helpRequested() const;
    std::string server() const;
    std::uint16_t port() const;
private:
    boost::program_options::variables_map m_vm;
};

} // namespace client

} // namespace cc_demo1
