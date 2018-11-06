#include "ProgramOptions.h"
#include <iostream>
#include <cassert>
#include <vector>

namespace po = boost::program_options;
namespace demo1
{

namespace server
{    

namespace
{

const std::string HelpStr("help");
const std::string FullHelpStr(HelpStr + ",h");
const std::string PortStr("port");
const std::string FullPortStr(PortStr + ",p");
const std::uint16_t DefaultPort = 20000;

po::options_description createDescription()
{
    po::options_description desc("Options");
    desc.add_options()
        (FullHelpStr.c_str(), "This help.")
        (FullPortStr.c_str(), po::value<std::uint16_t>()->default_value(DefaultPort),
            "TCP port for incoming connections.")
    ;
    return desc;
}

const po::options_description& getDescription()
{
    static const auto Desc = createDescription();
    return Desc;
}

} // namespace

void ProgramOptions::parse(int argc, const char* argv[])
{
    po::options_description allOptions;
    allOptions.add(getDescription());
    auto parseResult =
        po::command_line_parser(argc, argv)
            .options(allOptions)
            .run();
    po::store(parseResult, m_vm);
    po::notify(m_vm);
}

void ProgramOptions::printHelp(std::ostream& out)
{
    out << getDescription() << std::endl;
}

bool ProgramOptions::helpRequested() const
{
    return 0 < m_vm.count(HelpStr);
}

std::uint16_t ProgramOptions::port() const
{
    return m_vm[PortStr].as<std::uint16_t>();
}

} // namespace server

} // namespace demo1

