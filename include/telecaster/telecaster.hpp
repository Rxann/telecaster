#pragma once
#include <algorithm>
#include <bitset>
#include <cinttypes>
#include <cstdint>
#include <immintrin.h>
#include <vector>


namespace telecaster
{
	bool is_ip_in_range(const uint32_t* ip, const uint32_t* cidr, const uint32_t* mask);
	__m128i calculate_wildcard_mask(const uint32_t* subnet_mask);
	void generate_ip_addresses_from_range(const uint32_t* base_ip, uint32_t prefix_length, uint32_t* ip_addresses, size_t num_addresses);
	void calculate_network_broadcast_addresses(const uint32_t* base_ip, uint32_t prefix_length, uint32_t* network_address, uint32_t* broadcast_address);
	void ip_to_binary(const uint32_t* ip, char* binary_representation);
	std::vector<std::pair<uint32_t, uint32_t>> merge_cidr_blocks(const std::vector<std::pair<uint32_t, uint32_t>>& cidr_blocks);
	std::vector<std::pair<uint32_t, uint32_t>> sort_and_merge_cidr(const std::vector<std::pair<uint32_t, uint32_t>>& cidr_blocks);
	void convert_ranges_to_cidr(const std::vector<std::pair<uint32_t, uint32_t>>& ranges, std::vector<std::pair<uint32_t, uint32_t>>& cidr_blocks);
	bool is_block_overlapping(const std::vector<std::pair<uint32_t, uint32_t>>& cidr_blocks);
	std::pair<uint32_t, uint32_t> longest_prefix_match(uint32_t ip, const std::vector<std::pair<uint32_t, uint32_t>>& cidr_blocks);
}

namespace telecaster::helpers
{
	void convert_m128i_to_uint32(__m128i xmm_value, uint32_t* result);
	void cidr_to_range(uint32_t base_ip, uint32_t prefix_length, uint32_t& start_ip, uint32_t& end_ip);
	bool compare_cidr(const std::pair<uint32_t, uint32_t>& a, const std::pair<uint32_t, uint32_t>& b);
}