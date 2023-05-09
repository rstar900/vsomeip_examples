#include <compat/vsomeip/vsomeip.hpp>
#include <iostream>
#include <iomanip>
#include <cstring>

#define SERVICE_ID 0x4321
#define INSTANCE_ID 0x8765
#define METHOD_ID 0x4040

std::shared_ptr<vsomeip::application> app;

// Expecting to receive struct of 54 bytes as vector of bytes
// Bytes 0 - 3 -> id
// Bytes 4 - 53 -> msg

// Handler for handling client requets messages
void on_message(const std::shared_ptr<vsomeip::message> & req_)
{
    // get payload and length
    std::shared_ptr<vsomeip::payload> req_payload = req_->get_payload();
    vsomeip::length_t len = req_payload->get_length();
    
    // display the client and session
    std::cout << "Received message with client: " << req_->get_client() << ", session: " << req_->get_session() << std::endl;

    // parse received payload as struct
    // 1st 4 bytes for id
    int client_struct_id = 0;
    for (vsomeip::length_t i {}; i < 4; ++i)
    {
    client_struct_id = (client_struct_id << 8) + (int)*(req_payload->get_data()); // shift 8 bits left (1 bytes) and add next byte
    } 
    // last 50 bytes for msg
    char client_struct_msg[50]; 
    int ind {};
    for (vsomeip::length_t i {4}; i < len; ++i)
    {
    	client_struct_msg[ind++] = (char)*(req_payload->get_data() + i);
    }
    std::cout << " Received from client \n id : " << "0x" << std::setfill('0') << std::setw(2) << std::right << std::hex << client_struct_id << "\n msg: " << client_struct_msg << "\n";

    // send response to client as client_msg_t struct
    std::shared_ptr<vsomeip::message> res_ = vsomeip::runtime::get()->create_response(req_);
    std::shared_ptr<vsomeip::payload> res_payload = vsomeip::runtime::get()->create_payload();
    std::vector<vsomeip::byte_t> server_struct;
    // 1st 4 bytes for id
    for (int i{}; i < 4; ++i)
    {
    	server_struct.push_back(2 % 256); // just to keep the numbers between 0 - 255
    }
    char server_struct_msg[] = "If you see this then the ping pong works!";
    // last 50 bytes for msg
    for (int i{}; i < strlen(server_struct_msg); ++i)
    {
    	server_struct.push_back(server_struct_msg[i]);
    }
    // just for surity
    server_struct.push_back(0);
    res_payload->set_data(server_struct);
    res_->set_payload(res_payload);
    app->send(res_, true);
    std::cout << "Sent response to client!\n";
}

int main()
{
	app = vsomeip::runtime::get()->create_application("World");
	app->init();
	// Register a callback function for handling message from client
	app->register_message_handler(SERVICE_ID, INSTANCE_ID, METHOD_ID, on_message);
    	app->offer_service(SERVICE_ID, INSTANCE_ID);
	app->start();
}
