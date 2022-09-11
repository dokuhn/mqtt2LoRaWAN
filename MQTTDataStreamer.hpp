/**
 * @file MQTTDataStreamer.hpp
 * @author Dominik Kuhn (dominik.kuhn90@googlemail.com)
 * @brief 
 * @version 0.1
 * @date 2021-08-23
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef MQTTDataStreamer_MQTTDATASTREAMER_HH
#define MQTTDataStreamer_MQTTDATASTREAMER_HH

#include <string>
#include <mutex>
#include <cstdint>
#include <chrono>
#include <tuple>

#include "mqtt/async_client.h"
#include "HelperClasses.hpp"

/*
 * This is much simpler way of combining client and 
 * its corresponding callback. Refer to HelperClasses.hh
 * to learn more about Callback class.
 */
using MqttAsyncTuple = std::tuple<mqtt::async_client_ptr,
      mqtt::callback_ptr>;

class MQTTDataStreamer {
    /*
     * This classes acts as an wrapper for the mqtt_async_client 
     * class provided by the paho.mqtt.cpp library. It is meant
     * to make usage of paho.mqtt.cpp library easier. I(Nirmal) dunno
     * if that goal has been acheived though.
     */
    MqttAsyncTuple mqtt_async_tuple;
    std::chrono::milliseconds timeout;

    /*
     * All the meaning in the following functions 
     * have a predefinied meaning that are borrowed directly
     * from the paho.mqtt.cpp library. Besides the name of the
     * variables are pretty descriptive enough
     */
    mqtt::message_ptr createMessage(const void* payload, 
            std::size_t len, const std::string& topic, 
            uint8_t QoS, bool retain_msg = false);
    mqtt::connect_options buildConnectOptions();

    public:
    MQTTDataStreamer() = delete;
    // the default value of timeout is chosen from experiments
    // DO NOT TRUST IT.
    MQTTDataStreamer(
        MqttAsyncTuple mqtt_async_tuple_,
        std::chrono::milliseconds timeout_ = std::chrono::milliseconds(500));
    ~MQTTDataStreamer();

    /*
     * The mutex here is used to make sure that 
     * this function is thread-safe. Refer to test.cc 
     * in OpenCVImageSender project to learn more about why
     * this might be needed.
     */
    void publishMessage(
            const void* payload,
            std::size_t len,
            std::string topic,
            uint8_t QoS,
            std::mutex* mut, 
            const std::string& debug_info);
};

#endif