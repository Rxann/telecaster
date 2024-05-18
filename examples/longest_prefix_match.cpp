#include <iostream>
#include <telecaster/telecaster.hpp>


int main()
{
	std::vector<std::pair<uint32_t, uint32_t>> cidr_blocks = {
	    {0xC0A80000, 24}, // 192.168.0.0/24
	    {0xC0A80100, 24}, // 192.168.1.0/24
	    {0xC0A80200, 23}, // 192.168.2.0/23
	    {0xC0A80200, 22}  // 192.168.2.0/22 (longer prefix)
	};

	uint32_t ip = 0xC0A80201; // 192.168.2.1

	auto match = telecaster::longest_prefix_match(ip, cidr_blocks);
	std::cout << "Longest Prefix Match: " << ((match.first >> 24) & 0xFF) << "." << ((match.first >> 16) & 0xFF) << "." << ((match.first >> 8) & 0xFF) << "."
	          << (match.first & 0xFF) << "/" << match.second << std::endl;

	return 0;
}