#include <compat/vsomeip/vsomeip.hpp>
#include <cstring>

// This is an example struct of 54 bytes
// Bytes 0 - 3 -> id
// Bytes 4 - 53 -> msg

struct example_struct
{
	uint32_t id;
	char msg[50];
};

// Note : Need to manually copy parts of data from and to wire. as structs can have additional info that we don't need

// serialization function to put data from an example_struct object into std::vector<vsomeip::byte_t> 
inline void serialize_example_struct(example_struct& es, std::vector<vsomeip::byte_t>& payload_data_vec)
{
	
	// allocate enough memory on std::vector first
	payload_data_vec.resize(sizeof(uint32_t) + sizeof(char[50]));
	
	// 1st 4 bytes are integers
	std::memcpy((void*)&payload_data_vec[0], (void*)&(es.id), sizeof(uint32_t));
	
	// last 50 bytes are msg
	std::memcpy((void*)&payload_data_vec[4], (void*)&(es.msg), sizeof(char[50]));
} 

// deserialization function to get data from std::shared_ptr<vsomeip::payload> and put into an example_struct object 
inline void deserialize_example_struct(example_struct& es, std::shared_ptr<vsomeip::payload> payload)
{
	// 1st 4 bytes are integers
	std::memcpy((void*)&(es.id), (void*)payload->get_data(), sizeof(uint32_t));
	
	// last 50 bytes are msg (but can be less due to msg being shorter)
	std::memcpy((void*)&(es.msg), (void*)(payload->get_data() + 4), sizeof(char[50]));
	  
} 
