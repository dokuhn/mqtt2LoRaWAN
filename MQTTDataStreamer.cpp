/**
 * @file MQTTDataStreamer.cpp
 * @author Dominik Kuhn (dominik.kuhn90@googlemail.com)
 * @brief 
 * @version 0.1
 * @date 2021-08-23
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "MQTTDataStreamer.hpp"
#include "HelperClasses.hpp"

MQTTDataStreamer::MQTTDataStreamer(
        MqttAsyncTuple mqtt_async_tuple_,
        std::chrono::milliseconds timeout_) : 
    mqtt_async_tuple(mqtt_async_tuple_), timeout(timeout_) {

    auto [mqtt_async_client, callback] = mqtt_async_tuple;    

    mqtt_async_client->set_callback(*callback);    
    mqtt::connect_options connection_options = buildConnectOptions();

    try {
        std::cout << "Connecting ... ";
        mqtt_async_client->connect(connection_options)->wait();
        std::cout << "OK!\n";
    }
    catch(const mqtt::exception& exc) {
        std::cerr << exc.what() << "\n";
    }
}

MQTTDataStreamer::~MQTTDataStreamer() {
    try {
        std::cout << "Disconnecting ... ";
        std::get<0>(mqtt_async_tuple)->disconnect()->wait();
        std::cout << "OK!\n";
    }
    catch(const mqtt::exception& exc) {
        std::cerr << exc.what() << "\n";
    }
}

mqtt::connect_options MQTTDataStreamer::buildConnectOptions() {
    mqtt::connect_options opt;
    opt.set_clean_session(true);
    opt.set_keep_alive_interval(std::chrono::seconds(10));
    std::string lwt_payload = "SOMETHING_WENT_WRONG";
    opt.set_will(mqtt::message("/demonstrator_04/distress", 
                        lwt_payload, 1, false));
    return(opt);
}

mqtt::message_ptr MQTTDataStreamer::createMessage(
        const void* payload,
        std::size_t len,
        const std::string& topic,
        uint8_t QoS,
        bool retain_msg) {
    std::cout << "Creating MQTT message ... ";
    mqtt::message_ptr msg = mqtt::make_message(topic, 
            payload, len, QoS, retain_msg);
    std::cout << "Finished!\n";
    return(msg);
}

void MQTTDataStreamer::publishMessage(
        const void* payload,
        std::size_t len,
        std::string topic,
        uint8_t QoS,
        std::mutex* mut,
        const std::string& debug_info) {
    const std::lock_guard<std::mutex> lock(*mut);

    mqtt::message_ptr msg = createMessage(payload, len, topic, QoS);
    ActionListener listener("publish");
    
    // I am not using PERSISTANCE property of MQTT. This can be added 
    // if there need be.
    try {
        std::cout << "\t" << debug_info << "\n";
        mqtt::delivery_token_ptr publish_tok = std::get<0>(mqtt_async_tuple)->publish(
                msg, nullptr, listener);
        while(not listener.isDone())
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        std::cout << "\tOK!\n";
    }
    catch(const mqtt::exception& exc) {
        std::cerr << exc.what() << "\n";
    }
}
