#include <compat/vsomeip/vsomeip.hpp>
#include <iostream>

std::shared_ptr<vsomeip::application> app;

#define SERVICE_ID 0x4321
#define INSTANCE_ID 0x8765

// handler will be called when the service becomes available
void on_availability(vsomeip::service_t service_, vsomeip::instance_t instance_, bool is_available_)
{
    std::cout << std::hex <<"( Service : " << service_ << ", instance: " << instance_ << " ) is "
                << (is_available_ ? "available :)" : "not available :(") << std::endl;
}
int main()
{
	app = vsomeip::runtime::get()->create_application("Hello");
	app->init();
    	// need to register on_availability callback function
    	app->register_availability_handler(SERVICE_ID, INSTANCE_ID, on_availability);
    	// and request the service
    	app->request_service(SERVICE_ID, INSTANCE_ID);
	app->start();
}
