#include <compat/vsomeip/vsomeip.hpp>
#include "example_struct.h" // for type deifinition, serialization and deserialization of example_struct
#include <iostream>
#include <cstring>
#include <iomanip>
#include <condition_variable>
#include <thread>

#define SERVICE_ID 0x4321
#define INSTANCE_ID 0x8765
#define METHOD_ID 0x4040

std::shared_ptr<vsomeip::application> app;
std::mutex mutex;
std::condition_variable condition;

// Expecting to send and receive struct of 54 bytes as vector of bytes
// Bytes 0 - 3 -> id
// Bytes 4 - 53 -> msg

// Need to have run() method do the rest of logic after initializing in another thread as app->start() does not return
void run()
{
	// Need to only run after on_availability()
	std::unique_lock<std::mutex> lock(mutex);
	condition.wait(lock);
	
	// create the request
	std::shared_ptr<vsomeip::message> req_ = vsomeip::runtime::get()->create_request();
	req_->set_service(SERVICE_ID);
	req_->set_instance(INSTANCE_ID);
	req_->set_method(METHOD_ID);
	
	// populate the request payload
    	std::shared_ptr< vsomeip::payload > req_payload = vsomeip::runtime::get()->create_payload();
	std::vector<vsomeip::byte_t> client_struct;
	example_struct client_example_struct;
	client_example_struct.id = 1;
	std::strcpy(client_example_struct.msg,"Hello, a ping from client!");
	
	// serialize and send
	serialize_example_struct(client_example_struct, client_struct);
    	req_payload->set_data(client_struct);
    	req_->set_payload(req_payload);
    	app->send(req_, true);
    	std::cout << "Sent request to server!\n"; 
}

// handler when client receives a message from the server
void on_message(const std::shared_ptr<vsomeip::message>& res_)
{
	// get payload and length
	std::shared_ptr<vsomeip::payload> res_payload = res_->get_payload();
	vsomeip::length_t len = res_payload->get_length();
	
	// display the client and session
	std::cout << "Received message with client: " << res_->get_client() << ", session: " << res_->get_session() << std::endl;

	// parse received payload as example_struct
    	example_struct server_example_struct;
    	deserialize_example_struct(server_example_struct, res_payload);
	std::cout << " Received from server \n id : " << "0x" << std::setfill('0') << std::setw(2) << std::right << std::hex << server_example_struct.id << "\n msg: " << server_example_struct.msg << "\n";
	
}

// handler will be called when the service becomes available
void on_availability(vsomeip::service_t service_, vsomeip::instance_t instance_, bool is_available_)
{
    std::cout << std::hex <<"( Service : " << service_ << ", instance: " << instance_ << " ) is "
                << (is_available_ ? "available :)" : "not available :(") << std::endl;
                condition.notify_one(); // important to make run() proceed further
}
int main()
{
	app = vsomeip::runtime::get()->create_application("Hello");
	app->init();
    	// need to register on_availability callback function
    	app->register_availability_handler(SERVICE_ID, INSTANCE_ID, on_availability);
    	// and request the service
    	app->request_service(SERVICE_ID, INSTANCE_ID);
    	// register message handler
    	app->register_message_handler(SERVICE_ID, INSTANCE_ID, METHOD_ID, on_message);
    	// another thread for run()
    	std::thread sender(run);
	app->start();
}
