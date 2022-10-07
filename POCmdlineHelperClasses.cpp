
#include <string>

#include "POHelperClasses.hpp"
#include "POCmdlineHelperClasses.hpp"


namespace po = boost::program_options;


void po_cmdline_helper::init(boost::program_options::options_description cmdline_desc){

        cmdline_desc.add_options()  ("help", "produce help message")
                                    ("version,v", "print the version number")
                                    ("hostname,h", po::value<std::string>()->default_value("localhost"), "Hostname")
                                    ("port,p", po::value<int>()->default_value(1883), "Port")
                                    ("config,c", po::value<std::string>(), "configuration file")
                                    ("magic,m", po::value<magic_number>(), "magic value (in NNN-NNN format)")
                                    ("appeui", po::value<appeui>(), "APPEUI")
                                    ("deveui", po::value<deveui>(), "DEVEUI")
                                    ("devkey", po::value<devkey>(), "DEVKEY");

}
