#include <compat/vsomeip/vsomeip.hpp>
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
	// first 4 bytes for id
	for (int i{}; i < 4; ++i)
	{
		client_struct.push_back(1 % 256); // just to keep numbers between 0 - 255
	}
	// last 50 bytes for msg
	char client_struct_msg[] = "Hello, a ping from client!";
       // last 50 bytes for msg
	for (int i{}; i < strlen(client_struct_msg); ++i)
	{
    		client_struct.push_back(client_struct_msg[i]);
    	}
    	// just for surity
    	client_struct.push_back(0);
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

	// parse received payload as struct
	// 1st 4 bytes for id
    	int server_struct_id = 0;
    	for (vsomeip::length_t i {}; i < 4; ++i)
    	{
    		server_struct_id = (server_struct_id << 8) + (int)*(res_payload->get_data()); // shift 8 bits left (1 bytes) and add next byte
    	} 
    	// last 50 bytes for msg
	char server_struct_msg[50]; 
	int ind {};
	for (vsomeip::length_t i {4}; i < len; ++i)
	{
    		server_struct_msg[ind++] = (char)*(res_payload->get_data() + i);
    	}
	std::cout << " Received from server \n id : " << "0x" << std::setfill('0') << std::setw(2) << std::right << std::hex << server_struct_id << "\n msg: " << server_struct_msg << "\n";
	
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
