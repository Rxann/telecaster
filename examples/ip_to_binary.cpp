#include <iostream>
#include <telecaster/telecaster.hpp>


int main()
{
	// example IP address: 192.168.1.1
	uint32_t ip = (192 << 24) | (168 << 16) | (1 << 8) | 1;

	// allocate memory for the binary representation (32 bits + 1 for null terminator)
	char binary_representation[33];

	// Convert IP address to binary format
	telecaster::ip_to_binary(&ip, binary_representation);

	// output the binary representation
	std::cout << "Binary representation: " << binary_representation << std::endl;

	return 0;
}