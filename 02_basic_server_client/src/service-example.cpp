#include <compat/vsomeip/vsomeip.hpp>

std::shared_ptr<vsomeip::application> app;

#define SERVICE_ID 0x4321
#define INSTANCE_ID 0x8765

int main()
{
	app = vsomeip::runtime::get()->create_application("World");
	app->init();
    	app->offer_service(SERVICE_ID, INSTANCE_ID);
	app->start();
}
