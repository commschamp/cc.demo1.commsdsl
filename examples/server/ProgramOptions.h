#pragma once

#include <cstdint>
#include <boost/program_options.hpp>

namespace demo1
{

namespace server
{

class ProgramOptions
{
public:
    void parse(int argc, const char* argv[]);
    static void printHelp(std::ostream& out);

    bool helpRequested() const;
    std::uint16_t port() const;
private:
    boost::program_options::variables_map m_vm;
};

} // namespace server

} // namespace demo1
