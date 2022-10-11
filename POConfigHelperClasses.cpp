#include <string>

#include "POHelperClasses.hpp"
#include "POConfigHelperClasses.hpp"


namespace po = boost::program_options;


void po_config_helper::init(boost::program_options::options_description *config_desc){

        config_desc->add_options()	("APPEUI", po::value<appeui>(), "APPEUI")
 				                    ("DEVEUI", po::value<deveui>(), "DEVEUI")
				                    ("DEVKEY", po::value<devkey>(), "DEVKEY");


}
