#ifndef ProgramOptions_CmdLine_HelperClasses_HPP
#define ProgramOptions_CmdLine_HelperClasses_HPP


#include <boost/program_options.hpp>


class po_cmdline_helper {

public:
    void init(boost::program_options::options_description *cmdline_desc);

};


#endif

