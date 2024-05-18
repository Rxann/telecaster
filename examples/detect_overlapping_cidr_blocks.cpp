#include <iostream>
#include <telecaster/telecaster.hpp>


int main()
{
	std::vector<std::pair<uint32_t, uint32_t>> cidr_blocks = {
	    {0xC0A80000, 24}, // 192.168.0.0/24
	    {0xC0A80100, 24}, // 192.168.1.0/24
	    {0xC0A80200, 23}  // 192.168.2.0/23
	};

	bool has_overlap = telecaster::is_block_overlapping(cidr_blocks);

	std::cout << "Overlap detected: " << (has_overlap ? "Yes" : "No") << std::endl;

	return 0;
}