#ifndef MQTTDataStreamer_HELPERCLASSES_H
#define MQTTDataStreamer_HELPERCLASSES_H

#include <atomic>
#include <string>
#include <iostream>
#include "mqtt/async_client.h"



class TopicsToHandle {
public:
    std::atomic<bool> message_received = false;
    std::string name;
    uint8_t QoS;
    
    TopicsToHandle(const std::string& name_,
            uint8_t QoS_) : name(name_), QoS(QoS_) {}
    virtual void processMessage(mqtt::const_message_ptr msg_) = 0;
};

/**
 * A base action listener.
 */
class ActionListener : public virtual mqtt::iaction_listener {
    std::string name;
    std::atomic<bool> done;

	void on_failure(const mqtt::token& tok) override {
        auto topics = tok.get_topics();
        if(topics && !topics->empty())
            std::cout << "\t" << name << " failure for " << 
                (*topics)[0] << '\n';
        done = true;
	}
	void on_success(const mqtt::token& tok) override {
        auto topics = tok.get_topics();
        if(topics && !topics->empty())
            std::cout << "\t" << name << " success for " << 
                (*topics)[0] << '\n';
        done = true;
	}

public:
    ActionListener(const std::string& name_) :
        name(name_), done(false) {}
    bool isDone() const { return done; };
};


class MqttCallback : public virtual mqtt::callback {
    std::shared_ptr<mqtt::async_client> mqtt_async_client;
    std::vector<std::shared_ptr<TopicsToHandle>> topics_to_handle;

    ActionListener listener{"subscribe"};

    void connected(const std::string& cause) override {
        std::cout << "\tConnected!\n";
        for(const auto& topic : topics_to_handle) {
            std::cout << "\t\tSubscribing to '" << 
                topic->name << "' using QoS '" << topic->QoS << "'\n";
            mqtt_async_client->subscribe(topic->name, 
                    topic->QoS, nullptr, listener);
        }
        std::cout << "\tSubscription complete!\n";
    }
	void connection_lost(const std::string& cause) override {
		std::cout << "\tConnection lost ... ";
		if (!cause.empty())
			std::cout << cause << "\n";
        else
            std::cout << "no cause found!\n";
	}
    void message_arrived(mqtt::const_message_ptr msg) override {
        std::cout << "\tMessage arrived on " << 
            msg->get_topic() << "\n";
        for(const auto& topic : topics_to_handle) {
            if(topic->name == msg->get_topic())
                topic->processMessage(msg);
        }
    }
	void delivery_complete(mqtt::delivery_token_ptr tok) override {
        auto message = tok->get_message();
        std::cout << "\tDelivery on " << message->get_topic() << " complete!\n";
	}

public:
    MqttCallback(std::shared_ptr<mqtt::async_client> mqtt_async_client_,
            const std::vector<std::shared_ptr<TopicsToHandle>>& topics_to_handle_) :
        mqtt_async_client(mqtt_async_client_), 
        topics_to_handle(topics_to_handle_) {}
};

#endif
