#include <compat/vsomeip/vsomeip.hpp>
#include "example_struct.h" // for type definition, serialization and deserialization of example_struct
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

    // parse received payload as example_struct
    example_struct client_example_struct;
    deserialize_example_struct(client_example_struct, req_payload);
    std::cout << " Received from client \n id : " << "0x" << std::setfill('0') << std::setw(2) << std::right << std::hex << client_example_struct.id << "\n msg: " << client_example_struct.msg << "\n";

    // send response to client as example_struct on wire (increment id by 1 and send own msg)
    std::shared_ptr<vsomeip::message> res_ = vsomeip::runtime::get()->create_response(req_);
    std::shared_ptr<vsomeip::payload> res_payload = vsomeip::runtime::get()->create_payload();
    std::vector<vsomeip::byte_t> server_struct;
    example_struct server_example_struct;
    server_example_struct.id = client_example_struct.id + 1;
    std::strcpy(server_example_struct.msg,"If you see this then the ping pong works!");
    serialize_example_struct(server_example_struct, server_struct);
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
