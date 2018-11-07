#pragma once

#include <cstdint>
#include <string>

#include <boost/program_options.hpp>

namespace demo1
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

} // namespace demo1
