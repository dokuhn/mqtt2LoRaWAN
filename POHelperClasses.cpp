
#include <cstring>

#include "POHelperClasses.hpp"



appeui::appeui(std::string appeui_hexstring){
        std::copy(appeui_hexstring.begin(), appeui_hexstring.end(), application_eui64.e8);
	// std::memcpy(&application_eui64, appeui_hexstring, 8);
}


deveui::deveui(std::string deveui_hexstring){
        std::copy(deveui_hexstring.begin(), deveui_hexstring.end(), device_eui64.e8);
	// std::memcpy(&application_eui64, appeui_hexstring, 8);
}




devkey::devkey(std::string devkey_hexstring){
      std::copy(devkey_hexstring.begin(), devkey_hexstring.end(), device_key);
}
