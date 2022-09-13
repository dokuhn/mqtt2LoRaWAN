#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

#include "POHelperClasses.hpp"
#include "MQTTDataStreamer.hpp"

#include <boost/program_options.hpp>
#include <boost/regex.hpp>
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
static const u1_t APPEUI[8] = {0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x23, 0x45};

// unique device ID (LSBF)
static const u1_t DEVEUI[8] = {0x8D, 0x53, 0x05, 0xD0, 0x7E, 0xD5, 0xB3, 0x70};

// device-specific AES key (derived from device EUI)
static const u1_t DEVKEY[16] = {0x44, 0x99, 0xB4, 0xD3, 0xDB, 0x02, 0x07, 0xE3, 0xD6, 0x1A, 0x06, 0x6E, 0xCB, 0x26, 0xFC, 0xA2};

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
        std::cout << "regex funktioniert .... " <<  boost::regex_replace(unfilterd_string, re, "" ) << std::endl;
        // v = boost::any(appeui( boost::lexical_cast<eui.e32>(match[1]) ));
    } else {
        throw validation_error(validation_error::invalid_option_value);
    }        
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

    // set options allowed by the command line
    po::options_description command_line_options("Allowed options");
    command_line_options.add_options()  ("help", "produce help message")
                                        ("version,v", "print the version number")
                                        ("hostname,h", po::value<string>()->default_value("localhost"), "Hostname")
                                        ("port,p", po::value<int>()->default_value(1883), "Port")
                                        ("config,c", po::value<std::string>()->default_value("default.conf"), "configuration file")
                                        ("magic,m", po::value<magic_number>(), "magic value (in NNN-NNN format)")
                                        ("eui", po::value<appeui>(), "APPEU");

    // set options allowed in config file
    po::options_description config_file_options;
    config_file_options.add_options()   ("APPEUI", po::value<string>(), "APPEUI");


    po::variables_map variable_map;
    po::store(po::parse_command_line(argc, argv, command_line_options), variable_map);
    po::notify(variable_map);
    
    auto config_file = variable_map.at("config").as<std::string>();

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


    if (variable_map.count("eui"))
    {
        // std::cout << "APPEUI: " << variable_map["eui"].as<appeui>().application_eui64.e32 << std::endl;
        return 0;
    }


    if (variable_map.count("APPEUI"))
    {
        std::cout << "APPEUI: " << variable_map["APPEUI"].as<string>() << std::endl;
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
