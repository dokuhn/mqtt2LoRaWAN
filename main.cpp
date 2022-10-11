#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

#include "POHelperClasses.hpp"
#include "POCmdlineHelperClasses.hpp"
#include "POConfigHelperClasses.hpp"
#include "MQTTDataStreamer.hpp"

#include <boost/program_options.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/hex.hpp>
namespace po = boost::program_options;

extern "C"
{

#include "lmic.h"
#include "debug.h"
}

using namespace std;

const std::string DFLT_SERVER_ADDRESS{"tcp://localhost:1883"};
const std::string CLIENT_ID{"paho_cpp_async_publish"};
const std::string PERSIST_DIR{"./persist"};

const char *LWT_PAYLOAD = "Last will and testament.";

uint8_t QOS = 1;

const auto TIMEOUT = std::chrono::seconds(1);

// po::variables_map variable_map;

//////////////////////////////////////////////////
// CONFIGURATION (FOR APPLICATION CALLBACKS BELOW)
//////////////////////////////////////////////////

// application router ID (LSBF)
static u1_t APPEUI[8];

// unique device ID (LSBF)
static u1_t DEVEUI[8];

// device-specific AES key (derived from device EUI)
static u1_t DEVKEY[16];

//////////////////////////////////////////////////
// LMIC APPLICATION CALLBACKS
//////////////////////////////////////////////////

// provide application router ID (8 bytes, LSBF)
void os_getArtEui(u1_t *buf)
{
    memcpy(buf, APPEUI, 8);
}

// provide device ID (8 bytes, LSBF)
void os_getDevEui(u1_t *buf)
{
    memcpy(buf, DEVEUI, 8);
}

// provide device key (16 bytes)
void os_getDevKey(u1_t *buf)
{
    memcpy(buf, DEVKEY, 16);
}

//////////////////////////////////////////////////
// MQTT APPLICATION CALLBACKS
/////////////////////////////////////////////////

void handleTopics(std::shared_ptr<MQTTDataStreamer> streamer_obj,
                  const std::vector<std::shared_ptr<TopicsToHandle>> &topics_to_handle,
                  std::mutex *mut)
{
    /*
     * This function runs in a separate thread. Here you can
     * write down the logic to be executed when a message is sent over a
     * subscribed topic. Subscription happens via Callback described in
     * HelperClasses.hh file in MQTTDataStreamer library.
     * Messages over subscribed topics are mainly supposed
     * to act as a trigger. If you want to perform
     * actions on the sent msg, it is currently only possible in the HelperClasses.hh
     * This boundary is rather limiting and it require a rewrite of MQTTDataStreamer
     * library(which is highly needed in my(Nirmal) opinion)
     */
    while (true)
    {
        for (const auto &topic : topics_to_handle)
        {
            if (topic->name == "LoRa_test/transmitPacket/")
            {
                if (topic->message_received)
                {

                    topic->message_received = false;
                }
            }
            else
            {
                std::cout << "\tTopic '" << topic->name << "' not handled\n";
                exit(1);
            }
        }
    }
}

class DataTransmitTopic : public virtual TopicsToHandle
{
    /*
     * This class is used provided as a means to do something
     * with the messages sent over subscribed messages. It requires
     * changing processMessage() to processMessage(const_message_ptr).
     * This can be theoretically done and boundary between MQTTDataStreamer
     * and main() can be broken by declaring static variables in this
     * Translation Unit.
     */
public:
    DataTransmitTopic(const std::string &name,
                      uint8_t QoS = 1) : TopicsToHandle(name, QoS) {}
    void processMessage(mqtt::const_message_ptr msg_) override
    {
        message_received = true;

        std::size_t msg_len = msg_->get_payload().size();
        char *msg = new char[msg_len + 1];
        std::memcpy(msg, msg_->get_payload().data(), msg_len + 1);

        std::cout << "sending packet ..." << std::endl;

        for (int i = 0; i < msg_len; i++)
        {

            std::printf("%02hhX", msg[i]);
        }
        std::cout << std::endl;

        for (int i = 0; i < msg_len; i++)
        {

            LMIC.frame[i] = msg[i];
        }

        LMIC_setTxData2(1, LMIC.frame, msg_len, 0); // (port 1, 2 bytes, unconfirmed)
    }
};




std::string mapper(std::string env_var)
{
   // ensure the env_var is all caps
   std::transform(env_var.begin(), env_var.end(), env_var.begin(), ::toupper);

   if (env_var == "PATH") return "path";
   if (env_var == "EXAMPLE_VERBOSE") return "verbosity";
   if (env_var == "HOSTNAME") return "hostname";
   if (env_var == "HOME") return "home";
   return "";
}



//////////////////////////////////////////////////
// MAIN - INITIALIZATION AND STARTUP
//////////////////////////////////////////////////

// initial job
static void initfunc(osjob_t *j)
{

    // reset MAC state
    LMIC_reset();
    // start joining
    LMIC_startJoining();
    // init done - onEvent() callback will be invoked...
}

// application entry point
int main(int argc, char *argv[])
{
    osjob_t initjob;

    std::string hostname;

    po_cmdline_helper po_cmdline_inst;
    po_config_helper po_config_inst;

    boost::program_options::options_description desc_env;
    desc_env.add_options() ("path", "the execution path")
                           ("home", "the home directory of the executing user")
     			           ("verbosity", po::value<std::string>()->default_value("INFO"), "set verbosity: DEBUG, INFO, WARN, ERROR, FATAL")
            			   ("hostname", boost::program_options::value<std::string>(&hostname));

    boost::program_options::variables_map vm_env;
    boost::program_options::store(boost::program_options::parse_environment(desc_env, boost::function1<std::string, std::string>(mapper)), vm_env);
    boost::program_options::notify(vm_env);

    if (vm_env.count("home"))
    {
        std::cout << "home path: " << vm_env["home"].as<std::string>() << std::endl;
    }

    if (vm_env.count("path"))
    {
        std::cout << "First 75 chars of the system path: \n";
        std::cout << vm_env["path"].as<std::string>().substr(0, 75) << std::endl;
    }

    std::cout << "Verbosity: " << vm_env["verbosity"].as<std::string>() << std::endl;

    if ( vm_env.count("hostname"))
    {
        std::cout << "hostname: " <<  vm_env["hostname"].as<std::string>() << std::endl; // correct value of HOSTNAME environent variable
    }


    // set options allowed by the command line
    po::options_description command_line_options("Allowed options");

    po_cmdline_inst.init( &command_line_options );
    /*
    command_line_options.add_options()  ("help", "produce help message")
                                        ("version,v", "print the version number")
                                        ("hostname,h", po::value<string>()->default_value("localhost"), "Hostname")
                                        ("port,p", po::value<int>()->default_value(1883), "Port")
                                        ("config,c", po::value<std::string>(), "configuration file")
                                        ("magic,m", po::value<magic_number>(), "magic value (in NNN-NNN format)")
                                        ("appeui", po::value<appeui>(), "APPEUI")
                                        ("deveui", po::value<deveui>(), "DEVEUI")
					                    ("devkey", po::value<devkey>(), "DEVKEY");

    */

    // set options allowed in config file
    po::options_description config_file_options;
    
    po_config_inst.init( &config_file_options );
    /*
    config_file_options.add_options() ("APPEUI", po::value<appeui>(), "APPEUI")
 				                      ("DEVEUI", po::value<deveui>(), "DEVEUI")
				                      ("DEVKEY", po::value<devkey>(), "DEVKEY");
    */

    po::variables_map variable_map;
    po::store(po::parse_command_line(argc, argv, command_line_options), variable_map);
    po::notify(variable_map);

    std::string config_file;
    if( variable_map.count("config") ){
        config_file = variable_map.at("config").as<std::string>();
    }else if( vm_env.count("home") ){
        
        if(std::filesystem::exists( vm_env["home"].as<std::string>() + "/.config/mqtt2LoRaWAN/default.conf" )){
            config_file = vm_env["home"].as<std::string>() + "/.config/mqtt2LoRaWAN/default.conf";
        }else if(std::filesystem::exists("/etc/mqtt2LoRaWAN/default.conf")){
            config_file =  "/etc/mqtt2LoRaWAN/default.conf";
        }

    }else{
        if(std::filesystem::exists("/etc/mqtt2LoRaWAN/default.conf")){
            config_file =  "/etc/mqtt2LoRaWAN/default.conf";
        }
    }


    std::ifstream ifs(config_file.c_str());
    if (!ifs) {
        std::cout << "can not open configuration file: " << config_file << "\n";
    } else {
        po::store(parse_config_file(ifs, config_file_options), variable_map);
        po::notify(variable_map);
    }

    po::notify(variable_map);
    std::cout << config_file << " was the config file\n";


    if (variable_map.count("help"))
    {
        std::cout << command_line_options << std::endl;
        return 0;
    }

    if (variable_map.count("version")) {
            cout << "Version 1.\n";
            return 0;
    }

    if (variable_map.count("magic")) {
        cout << "The magic is \"" << variable_map["magic"].as<magic_number>().n << "\"\n";
        return 0;
    }


    if (variable_map.count("DEVEUI"))
    {
        std::cout << "DEVEUI:  " << std::endl;
        for( int i = 0; i < 8; i++ ){
	        std::printf("%#02x", variable_map["DEVEUI"].as<deveui>().device_eui64.e8[i] );
	    }
	    std::cout << std::endl;
	    std::memcpy(DEVEUI, variable_map["DEVEUI"].as<deveui>().device_eui64.e8, 8);

    }else{
        std::cout << "No DEVEUI found in the config file. Add a 8 bytes long EUI to config file or specify an EUI as an option via the command line." << std::endl;
        return 1;
    }


    if (variable_map.count("APPEUI"))
    {
        std::cout << "APPEUI: "  << std::endl;
        for( int i = 0; i < 8; i++ ){
	        std::printf("%#02x", variable_map["APPEUI"].as<appeui>().application_eui64.e8[i] );
	    }
	    std::cout << std::endl;
            std::memcpy(APPEUI, variable_map["APPEUI"].as<appeui>().application_eui64.e8, 8);
    }else{
        std::cout << "No APPEUI found in the config file. Add a 8 bytes long EUI to config file or specify an EUI as an option via the command line." << std::endl;
        return 1;
    }

    if (variable_map.count("DEVKEY"))
    {
        std::cout << "DEVKEY: "  << std::endl;
        for( int i = 0; i < 16; i++ ){
	        std::printf("%#02x", variable_map["DEVKEY"].as<devkey>().device_key[i] );
	    }
	    std::cout << std::endl;
            std::memcpy(DEVKEY, variable_map["DEVKEY"].as<devkey>().device_key, 16);
    }else{
        std::cout << "No DEVKEY found in the config file. Add a 16 bytes long key to config file or specify a key as an option via the command line." << std::endl;
        return 1;
    }


    std::cout << "Initializing and connecting for server '" << variable_map["hostname"].as<string>() << "'..." << std::endl;

    std::vector<std::shared_ptr<TopicsToHandle>> topics_to_handle;
    topics_to_handle.push_back(std::make_shared<DataTransmitTopic>(
        "LoRa_test/transmitPacket/"));

    auto mqtt_async_client = std::make_shared<mqtt::async_client>((string) "tcp://" + variable_map["hostname"].as<string>() +
                                                                      (string) ":" + to_string(variable_map["port"].as<int>()),
                                                                  CLIENT_ID);

    auto callback = std::make_shared<MqttCallback>(mqtt_async_client, topics_to_handle);

    auto streamer_obj = std::make_shared<MQTTDataStreamer>(
        std::make_tuple(mqtt_async_client, callback));

    std::mutex mut;
    std::cout << "  ...OK" << endl;

    // initialize runtime env
    os_init();
    // initialize debug library
    debug_init();
    // setup initial job
    os_setCallback(&initjob, initfunc);
    // execute scheduled jobs and events
    os_runloop();
    // (not reached)
    return 0;
}

//////////////////////////////////////////////////
// UTILITY JOB
//////////////////////////////////////////////////

static osjob_t reportjob;

// report sensor value every minute
static void reportfunc(osjob_t *j)
{
    // read sensor
    u2_t val = 543;
    debug_val("val = ", val);
    // prepare and schedule data for transmission
    LMIC.frame[0] = val >> 8;
    LMIC.frame[1] = val;
    LMIC_setTxData2(1, LMIC.frame, 2, 0); // (port 1, 2 bytes, unconfirmed)
    // reschedule job in 60 seconds
    os_setTimedCallback(j, os_getTime() + sec2osticks(60), reportfunc);
}

//////////////////////////////////////////////////
// LMIC EVENT CALLBACK
//////////////////////////////////////////////////
extern "C"
{

    void onEvent(ev_t ev)
    {
        debug_event(ev);

        switch (ev)
        {

        // network joined, session established
        case EV_JOINED:

            break;

        // data frame received
        case EV_RXCOMPLETE:
            // log frame data
            debug_buf(LMIC.frame + LMIC.dataBeg, LMIC.dataLen);
            /*
            if(LMIC.dataLen == 1) {
                // set LED state if exactly one byte is received
                debug_led(LMIC.frame[LMIC.dataBeg] & 0x01);
            }
            */

            break;

        // scheduled data sent (optionally data received)
        case EV_TXCOMPLETE:
            if (LMIC.dataLen)
            { // data received in rx slot after tx
                debug_buf(LMIC.frame + LMIC.dataBeg, LMIC.dataLen);
            }

            break;
        }
    }
}
