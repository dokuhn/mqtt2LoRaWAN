
#include <iostream>
#include <cstring>

#include "POHelperClasses.hpp"


#include <boost/regex.hpp>
#include <boost/algorithm/hex.hpp>


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


/* Overload the 'validate' function for the user-defined class.
   It makes sure that value is of form XXX-XXX 
   where X are digits and converts the second group to an integer.
   This has no practical meaning, meant only to show how
   regex can be used to validate values.
*/
void validate(boost::any& v, 
              const std::vector<std::string>& values,
              magic_number*, int)
{
    static boost::regex r("\\d\\d\\d-(\\d\\d\\d)");

    using namespace boost::program_options;

    // Make sure no previous assignment to 'a' was made.
    validators::check_first_occurrence(v);
    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const std::string& s = validators::get_single_string(values);

    // Do regex match and convert the interesting part to 
    // int.
    boost::smatch match;
    if (regex_match(s, match, r)) {
        v = boost::any(magic_number(boost::lexical_cast<int>(match[1])));
    } else {
        throw validation_error(validation_error::invalid_option_value);
    }
}


void validate(boost::any& v, 
              const std::vector<std::string>& values,
              appeui*, int)
{
    static boost::regex r("^([[:xdigit:]]{2}[:.-]?){7}[[:xdigit:]]{2}$");

    boost::regex re("[:.-]");

    using namespace boost::program_options;

    // Make sure no previous assignment to 'a' was made.
    validators::check_first_occurrence(v);

    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const std::string& s = validators::get_single_string(values);

    // Do regex match and convert the interesting part to 
    // int.
    boost::smatch match;
    if (regex_match(s, match, r)) {
	std::string unfilterd_string = match[0];
        std::string filterd_string = boost::regex_replace( unfilterd_string, re, "" );
        std::cout << "regex funktioniert .... " <<  filterd_string << std::endl;
        std::cout << "boost algorithm ... " <<  boost::algorithm::unhex(filterd_string) << std::endl;

        v = boost::any( appeui( boost::algorithm::unhex(filterd_string)   ) );
    } else {
        throw validation_error(validation_error::invalid_option_value);
    }
}


void validate(boost::any& v, 
              const std::vector<std::string>& values,
              deveui*, int)
{
    static boost::regex r("^([[:xdigit:]]{2}[:.-]?){7}[[:xdigit:]]{2}$");

    boost::regex re("[:.-]");

    using namespace boost::program_options;

    // Make sure no previous assignment to 'a' was made.
    validators::check_first_occurrence(v);

    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const std::string& s = validators::get_single_string(values);

    // Do regex match and convert the interesting part to 
    // int.
    boost::smatch match;
    if (regex_match(s, match, r)) {
	std::string unfilterd_string = match[0];
        std::string filterd_string = boost::regex_replace( unfilterd_string, re, "" );
        std::cout << "regex funktioniert .... " <<  filterd_string << std::endl;
        std::cout << "boost algorithm ... " <<  boost::algorithm::unhex(filterd_string) << std::endl;

        v = boost::any( deveui( boost::algorithm::unhex(filterd_string)   ) );
    } else {
        throw validation_error(validation_error::invalid_option_value);
    }
}

void validate(boost::any& v, 
              const std::vector<std::string>& values,
              devkey*, int)
{
    static boost::regex r("^([[:xdigit:]]{2}[:.-]?){15}[[:xdigit:]]{2}$");

    boost::regex re("[:.-]");

    using namespace boost::program_options;

    // Make sure no previous assignment to 'a' was made.
    validators::check_first_occurrence(v);

    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const std::string& s = validators::get_single_string(values);

    // Do regex match and convert the interesting part to 
    // int.
    boost::smatch match;
    if (regex_match(s, match, r)) {
	std::string unfilterd_string = match[0];
        std::string filterd_string = boost::regex_replace( unfilterd_string, re, "" );
        std::cout << "regex funktioniert .... " <<  filterd_string << std::endl;
        std::cout << "boost algorithm ... " <<  boost::algorithm::unhex(filterd_string) << std::endl;

        v = boost::any( devkey( boost::algorithm::unhex(filterd_string)   ) );
    } else {
        throw validation_error(validation_error::invalid_option_value);
    }
}
