#include <iostream>
#include <telecaster/telecaster.hpp>


int main()
{
	// Example CIDR blocks: 192.168.0.0/24, 192.168.1.0/24, 192.168.2.0/23
	std::vector<std::pair<uint32_t, uint32_t>> cidr_blocks = {{0xC0A80000, 24}, {0xC0A80100, 24}, {0xC0A80200, 23}};

	// Convert CIDR blocks to IP ranges
	std::vector<std::pair<uint32_t, uint32_t>> ip_ranges;
	for (const auto& cidr : cidr_blocks)
	{
		uint32_t start_ip, end_ip;
		telecaster::helpers::cidr_to_range(cidr.first, cidr.second, start_ip, end_ip);
		ip_ranges.emplace_back(start_ip, end_ip);
	}

	// Sort and merge IP ranges
	auto merged_ranges = telecaster::sort_and_merge_cidr(ip_ranges);

	// Convert merged ranges back to CIDR blocks
	std::vector<std::pair<uint32_t, uint32_t>> merged_cidr_blocks;
	telecaster::convert_ranges_to_cidr(merged_ranges, merged_cidr_blocks);

	// Print merged CIDR blocks
	std::cout << "Merged CIDR Blocks:" << std::endl;
	for (const auto& cidr : merged_cidr_blocks)
	{
		std::cout << ((cidr.first >> 24) & 0xFF) << "." << ((cidr.first >> 16) & 0xFF) << "." << ((cidr.first >> 8) & 0xFF) << "." << (cidr.first & 0xFF) << "/" << cidr.second << std::endl;
	}

	return 0;
}