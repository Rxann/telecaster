#include "telecaster.hpp"

namespace telecaster
{
	// todo: port to 256 bit vectors
	bool is_ip_in_range(const uint32_t* ip, const uint32_t* cidr, const uint32_t* mask)
	{
		__m128i xmm_ip   = _mm_load_si128((__m128i*)ip);
		__m128i xmm_cidr = _mm_load_si128((__m128i*)cidr);
		__m128i xmm_mask = _mm_load_si128((__m128i*)mask);

		__m128i xmm_network = _mm_and_si128(xmm_ip, xmm_mask);

		__m128i xmm_result = _mm_cmpeq_epi32(xmm_network, xmm_cidr);

		int result = _mm_movemask_epi8(xmm_result);

		return result == 0xFFFF;
	}

	// maybe: port this to 256 bit vectors
	__m128i calculate_wildcard_mask(const uint32_t* subnet_mask)
	{
		// load into simd register
		__m128i xmm_subnet_mask = _mm_load_si128((__m128i*)subnet_mask);

		__m128i xmm_wildcard_mask = _mm_xor_si128(xmm_subnet_mask, _mm_set1_epi32(-1));

		// return raw value
		return xmm_wildcard_mask;
	}

	// TODO: port this to 256 bit vectors
	void generate_ip_addresses_from_range(const uint32_t* base_ip, uint32_t prefix_length, uint32_t* ip_addresses, size_t num_addresses)
	{
		// calculate block size in simd register
		constexpr size_t SIMD_BLOCK_SIZE = sizeof(__m128i) / sizeof(uint32_t);

		//translte prefix length to bitmask
		uint32_t bitmask = (~0u) << (32 - prefix_length);

		// load base ip into simd register
		__m128i xmm_base_ip = _mm_set_epi32(base_ip[3], base_ip[2], base_ip[1], base_ip[0]);

		//broadcast bitmask to all of the simd reg. elements
		__m128i xmm_bitmask = _mm_set1_epi32(bitmask);

		//calculate number of iterations we need to compute all ips
		size_t num_iterations = (num_addresses + SIMD_BLOCK_SIZE - 1) / SIMD_BLOCK_SIZE;

		for (size_t i = 0; i < num_iterations; i++)
		{
			// calc starting idx for current iter
			size_t start_index = i * SIMD_BLOCK_SIZE;

			// calculate number of ips to process this iter
			size_t num_addresses_this_iteration = std::min(num_addresses - start_index, SIMD_BLOCK_SIZE);

			//create simd reg. containing indices
			__m128i xmm_indices = _mm_set_epi32(3, 2, 1, 0);

			//broadcast indices to all of the elements in the simd reg.
			__m128i xmm_indices_broadcasted = _mm_shuffle_epi32(xmm_indices, _MM_SHUFFLE(0, 0, 0, 0));

			// calc increment value
			__m128i xmm_increment = _mm_set1_epi32(num_addresses_this_iteration);

			// multiply indices by increment value
			// watch this, i think 32 was needed here but 16 will need to fucking do for now
			__m128i xmm_indices_multiplied = _mm_mullo_epi16(xmm_indices_broadcasted, xmm_increment);

			// add indices multiplied by increment to the starting index
			__m128i xmm_indices_adjusted = _mm_add_epi32(xmm_indices_multiplied, _mm_set1_epi32(start_index));

			// add base ip to manipulated indices
			__m128i xmm_ip_addresses = _mm_add_epi32(xmm_indices_adjusted, xmm_base_ip);

			// apply bitmask
			__m128i xmm_result = _mm_and_si128(xmm_ip_addresses, xmm_bitmask);

			// store final result in output array
			_mm_storeu_si128((__m128i*)(ip_addresses + start_index), xmm_result);
		}
	}

	void calculate_network_broadcast_addresses(const uint32_t* base_ip, uint32_t prefix_length, uint32_t* network_address, uint32_t* broadcast_address)
	{
		uint32_t subnet_mask_value  = (~0u) << (32 - prefix_length);
		uint32_t inverse_mask_value = ~subnet_mask_value;

		__m256i xmm_base_ip = _mm256_set1_epi32(*reinterpret_cast<const int32_t*>(base_ip));

		__m256i xmm_subnet_mask  = _mm256_set1_epi32(subnet_mask_value);
		__m256i xmm_inverse_mask = _mm256_set1_epi32(inverse_mask_value);

		__m256i xmm_network_address = _mm256_and_si256(xmm_base_ip, xmm_subnet_mask);

		__m256i xmm_broadcast_address = _mm256_and_si256(xmm_base_ip, xmm_inverse_mask);

		_mm256_storeu_si256((__m256i*)network_address, xmm_network_address);
		_mm256_storeu_si256((__m256i*)broadcast_address, xmm_broadcast_address);
	}

	void ip_to_binary(const uint32_t* ip, char* binary_representation)
	{
		// load into register
		__m256i xmm_ip = _mm256_set1_epi32(*reinterpret_cast<const int32_t*>(ip));

		// mask to extract each bte
		__m256i mask = _mm256_set1_epi32(0xFF);

		//shift right by multiples of 8 to isolate each byte
		__m256i byte0 = _mm256_and_si256(xmm_ip, mask);
		__m256i byte1 = _mm256_and_si256(_mm256_srli_epi32(xmm_ip, 8), mask);
		__m256i byte2 = _mm256_and_si256(_mm256_srli_epi32(xmm_ip, 16), mask);
		__m256i byte3 = _mm256_and_si256(_mm256_srli_epi32(xmm_ip, 24), mask);

		//store bytes in an array
		uint8_t bytes[4];
		bytes[0] = static_cast<uint8_t>(_mm256_cvtsi256_si32(byte0));
		bytes[1] = static_cast<uint8_t>(_mm256_cvtsi256_si32(byte1));
		bytes[2] = static_cast<uint8_t>(_mm256_cvtsi256_si32(byte2));
		bytes[3] = static_cast<uint8_t>(_mm256_cvtsi256_si32(byte3));

		// convert byte to binary form
		for (int i = 0; i < 4; i++)
		{
			// find a way to do this with simd instead of c++ functions
			std::bitset<8> byte_bits(bytes[i]);
			std::string byte_str = byte_bits.to_string();
			std::memcpy(binary_representation + (i * 8), byte_str.c_str(), 8);
		}

		// null terminate the string by casting a char null term character
		binary_representation[32] = (char)"\0";
	}

	std::vector<std::pair<uint32_t, uint32_t>> merge_cidr_blocks(const std::vector<std::pair<uint32_t, uint32_t>>& cidr_blocks)
	{
		// sanity check
		if (cidr_blocks.empty()) [[unlikely]]
			return {};

		std::vector<std::pair<uint32_t, uint32_t>> merged_blocks;
		merged_blocks.push_back(cidr_blocks[0]);

		for (size_t i = 1; i < cidr_blocks.size(); i++)
		{
			auto& last = merged_blocks.back();
			if (cidr_blocks[i].first <= last.second + 1)
			{
				last.second = std::max(last.second, cidr_blocks[i].second);
			}
			else
			{
				merged_blocks.push_back(cidr_blocks[i]);
			}
		}

		return merged_blocks;
	}

	std::vector<std::pair<uint32_t, uint32_t>> sort_and_merge_cidr(const std::vector<std::pair<uint32_t, uint32_t>>& cidr_blocks)
	{
		std::vector<std::pair<uint32_t, uint32_t>> sorted_blocks = cidr_blocks;
		std::sort(sorted_blocks.begin(), sorted_blocks.end(), helpers::compare_cidr);
		return merge_cidr_blocks(sorted_blocks);
	}

	// i dont know who would actually want to use this but here it is ig. NOTE: see if this is rly needed
	void convert_ranges_to_cidr(const std::vector<std::pair<uint32_t, uint32_t>>& ranges, std::vector<std::pair<uint32_t, uint32_t>>& cidr_blocks)
	{
		for (const auto& range : ranges)
		{
			uint32_t start_ip = range.first;
			uint32_t end_ip   = range.second;

			while (start_ip <= end_ip)
			{
				uint32_t prefix_length = 32;
				while (prefix_length > 0)
				{
					uint32_t mask      = (prefix_length == 0) ? 0 : (~0u << (32 - prefix_length));
					uint32_t masked_ip = start_ip & mask;
					if (masked_ip != start_ip || masked_ip + ~mask > end_ip)
						break;
					--prefix_length;
				}
				cidr_blocks.emplace_back(start_ip, 32 - prefix_length);
				start_ip += (1 << (32 - prefix_length));
			}
		}
	}

	bool is_block_overlapping(const std::vector<std::pair<uint32_t, uint32_t>>& cidr_blocks)
	{
		size_t n = cidr_blocks.size();
		// sanity check
		if (n < 2) [[unlikely]]
			return false;

		std::vector<std::pair<uint32_t, uint32_t>> ranges(n);
		for (size_t i = 0; i < n; i++)
		{
			uint32_t start_ip, end_ip;
			helpers::cidr_to_range(cidr_blocks[i].first, cidr_blocks[i].second, start_ip, end_ip);
			ranges[i] = {start_ip, end_ip};
		}

		std::sort(ranges.begin(), ranges.end());

		//use simd for range comparison
		for (size_t i = 0; i < n - 1; i++)
		{
			__m256i start1 = _mm256_set1_epi32(ranges[i].first);
			__m256i end1   = _mm256_set1_epi32(ranges[i].second);
			__m256i start2 = _mm256_set1_epi32(ranges[i + 1].first);
			__m256i end2   = _mm256_set1_epi32(ranges[i + 1].second);

			__m256i cmp = _mm256_cmpgt_epi32(end1, start2);

			if (_mm256_movemask_epi8(cmp) != 0xFFFF)
			{
				return true;
			}

			return false;
		}
	}

	std::pair<uint32_t, uint32_t> longest_prefix_match(uint32_t ip, const std::vector<std::pair<uint32_t, uint32_t>>& cidr_blocks)
	{
		size_t n = cidr_blocks.size();
		std::vector<uint32_t> masks(n);

		// generate masks for all prefixes
		for (size_t i = 0; i < n; ++i)
		{
			masks[i] = (cidr_blocks[i].second == 0) ? 0 : (~0u << (32 - cidr_blocks[i].second));
		}

		// initialize vars for AVX2
		__m256i ip_vec     = _mm256_set1_epi32(ip);
		int max_prefix_len = -1;
		int max_index      = -1;

		//process in chunks of 8 for better performance
		for (size_t i = 0; i < n; i += 8)
		{
			// load base ips into avx registers
			__m256i base_ips = _mm256_set_epi32(cidr_blocks[std::min(i + 7, n - 1)].first,
			    cidr_blocks[std::min(i + 6, n - 1)].first,
			    cidr_blocks[std::min(i + 5, n - 1)].first,
			    cidr_blocks[std::min(i + 4, n - 1)].first,
			    cidr_blocks[std::min(i + 3, n - 1)].first,
			    cidr_blocks[std::min(i + 2, n - 1)].first,
			    cidr_blocks[std::min(i + 1, n - 1)].first,
			    cidr_blocks[i].first);

			__m256i prefix_masks = _mm256_set_epi32(masks[std::min(i + 7, n - 1)], masks[std::min(i + 6, n - 1)], masks[std::min(i + 5, n - 1)], masks[std::min(i + 4, n - 1)], masks[std::min(i + 3, n - 1)], masks[std::min(i + 2, n - 1)], masks[std::min(i + 1, n - 1)], masks[i]);

			__m256i masked_ips    = _mm256_and_si256(base_ips, prefix_masks);
			__m256i masked_ip_vec = _mm256_and_si256(ip_vec, prefix_masks);
			__m256i cmp           = _mm256_cmpeq_epi32(masked_ips, masked_ip_vec);

			// extract results and find the longest prefix match
			int mask = _mm256_movemask_epi8(cmp);
			for (int j = 0; j < 8; ++j)
			{
				// what the actual fuck is this
				if ((mask >> (j * 4)) & 0xF)
				{
					int idx = i + j;
					if (idx < n && cidr_blocks[idx].second > max_prefix_len)
					{
						max_prefix_len = cidr_blocks[idx].second;
						max_index      = idx;
					}
				}
			}
		}

		if (max_index != -1) [[likely]]
		{
			return cidr_blocks[max_index];
		}
		// what are the odds of nothing being found... low
		else [[unlikely]]
		{
			return {0, 0}; // nothing found
		}
	}
}

namespace telecaster::helpers
{
	void convert_m128i_to_uint32(__m128i xmm_value, uint32_t* result)
	{
		_mm_storeu_si128((__m128i*)result, xmm_value);
	}

	void cidr_to_range(uint32_t base_ip, uint32_t prefix_length, uint32_t& start_ip, uint32_t& end_ip)
	{
		uint32_t mask = (prefix_length == 0) ? 0 : (~0u << (32 - prefix_length));
		start_ip      = base_ip & mask;
		end_ip        = start_ip | ~mask;
	}

	bool compare_cidr(const std::pair<uint32_t, uint32_t>& a, const std::pair<uint32_t, uint32_t>& b)
	{
		return a.first < b.first || (a.first == b.first && a.second < b.second);
	}
}