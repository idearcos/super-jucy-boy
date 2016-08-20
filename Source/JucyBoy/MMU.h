#pragma once

#include <cstdint>
#include <array>
#include <limits>

class MMU
{
public:
	using Address = uint16_t;

	MMU();
	~MMU();

	uint8_t Read(Address address) const;
	void Write(Address address, uint8_t value);

private:
	std::array<uint8_t, std::numeric_limits<Address>::max() + 1> memory_;
};
